<?php

/**
 * @file
 * User page callbacks for the janrain_capture module.
 */
function janrain_capture_oauth() {
  global $user;
  $token = isset($_REQUEST['code']) ? $_REQUEST['code'] : '';
  $from_sso = isset($_REQUEST['from_sso']) ? $_REQUEST['from_sso'] : '';
  $origin = isset($_REQUEST['origin']) ? $_REQUEST['origin'] : '';
  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());

  if ($user->uid) {
    drupal_set_message(t("You're already signed in!"), 'error');
  }
  elseif ($token) {
    $uri_opts = ($from_sso && $origin) ? array('query' => array('from_sso' => 'true', 'origin' => $origin), 'absolute' => true) : array('absolute' => true);

    $redirect_uri = url('janrain_capture/oauth', $uri_opts);

    $api = new Capture_API();
    $api->new_access_token($token, $redirect_uri);
    $profile = $api->load_user_entity();

    if (!$profile || $profile['stat'] != 'ok') {
      drupal_set_message(t('We were unable to complete your request.'), 'error');
      watchdog('janrain_capture', t('Failed to obtain a Capture record'), WATCHDOG_ERROR);
    }
    else {
      // Save profile in session
      $_SESSION['janrain_capture_profile'] = $profile;
      
      $userInfo = array(
        'name' => janrain_capture_profile_field($profile),
        'mail' => $profile['result']['email'],
        'status' => 1
      );
      
      $userInfo = janrain_capture_fields_array($userInfo, $profile['result']);

      $account = user_external_load($profile['result']['uuid']);
      $newUser = false;

      // No user was found with our Capture uuid.
      if (!$account->uid) {
        // Look for a local user with the same email address
        if ($local_user = user_load(array('mail' => $profile['result']['email']))) {
          // Are we configured to match users based on email?
          if (isset($janrain_capture_fields['capture_match_email']) && $janrain_capture_fields['capture_match_email']) {
            // Check to see if this user is already mapped to a Capture uuid
            $result = db_query("SELECT aid FROM {authmap} WHERE module = 'janrain_capture' AND uid = %d", $local_user->uid);
            if (db_fetch_array($result)) {
              $mapped_hook = module_invoke_all('janrain_capture_user_already_mapped');
              if (empty($mapped_hook) || !in_array(false, $mapped_hook))
                drupal_set_message(t('A user with this email address is already mapped.'), 'error');
            }
            // Bind Capture profile data and uuid to existing user record
            else {
              $userInfo['auth_janrain_capture'] = $profile['result']['uuid'];
              $account = user_save($local_user, $userInfo);
              if (!$account->uid) {
                $mapping_failed_hook = module_invoke_all('janrain_capture_mapping_failed');
                if (empty($mapping_failed_hook) || !in_array(false, $mapping_failed_hook))
                  drupal_set_message(t('Failed to map to existing user.'), 'error');
              }
            }
          }
          else {
            $user_exists_hook = module_invoke_all('janrain_capture_user_exists');
            if (empty($user_exists_hook) || !in_array(false, $user_exists_hook))
              drupal_set_message(t('A user with this email address already exists.'), 'error');
          }
        }
        // No local users with matching email. Create a new one.
        else {
          $userInfo['pass'] = md5(time() . $profile['result']['uuid']);
          $userInfo['auth_janrain_capture'] = $profile['result']['uuid'];
          $account = user_save(null, $userInfo);
          $newUser = true;
          if (!$account->uid) {
            $failed_create = module_invoke_all('janrain_capture_failed_create');
            if (empty($failed_create) || !in_array(false, $failed_create))
              drupal_set_message(t('Failed to create new user.'), 'error');
          }
        }
      }
      // We found the user! Re-sync Capture data to make sure we're up to date.
      else {
        $account = user_save($account, $userInfo);
      }

      $signin = true;
      if (!$account->uid)
        $signin = false;

      if (isset($_SESSION['janrain_capture_action'])
            && ($_SESSION['janrain_capture_action'] == 'finish_third_party'
            || $_SESSION['janrain_capture_action'] == 'legacy_register')
            && isset($janrain_capture_fields['capture_enforce_verification'])
            && $janrain_capture_fields['capture_enforce_verification']
            && $profile['result']['emailVerified'] == null) {
          $signin = false;
          $email_text = (isset($_SESSION['janrain_capture_profile']['result']['email'])) ? ' to ' . $_SESSION['janrain_capture_profile']['result']['email'] : '';
          drupal_set_message(t('A verification link has been sent@email-text. Please check your email.', array('@email-text' => $email_text)), 'status');
      } elseif(isset($janrain_capture_fields['capture_enforce_verification'])
          && $janrain_capture_fields['capture_enforce_verification']
          && $profile['result']['emailVerified'] == null) {
        $signin = false;
        $resend_link = 'https://'
          . (!empty($janrain_capture_optional['captureui_address']) ? $janrain_capture_optional['captureui_address'] : $janrain_capture_main['capture_address'])
          . '/oauth/resend_verification_email?access_token='
          . $_SESSION['janrain_capture_access_token'] . '&redirect_uri='
          . urlencode(url('janrain_capture/resend_verification_email', array('absolute' => true)))
          . (!empty($janrain_capture_optional['capture_application_id']) ? '&application_id=' . $janrain_capture_optional['capture_application_id'] : '');
        drupal_set_message(t('Your email address has not yet been verified. Please check your email and try again. <a href="@resend-link">Click Here</a> to have this email resent.', array('@resend-link' => $resend_link)), 'error');
      }

      if ($signin) {
        user_external_login($account);
        module_invoke_all('janrain_capture_user_authenticated', $profile['result'], $account, $newUser);
      }
    }
  }
  else {
    $no_oauth = module_invoke_all('janrain_capture_no_oauth');
    if (empty($no_oauth) || !in_array(false, $no_oauth))
      drupal_set_message(t('No Oauth token found!'), 'error');
  }
  print theme('janrain_capture_oauth');
  return null;
}

function janrain_capture_profile() {
  $method = isset($_GET['method']) ? $_GET['method'] : '';
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());

  if (time() >= $_SESSION['janrain_capture_expires_in']) {
    $api = new Capture_API();
    $api->refresh_access_token($_SESSION['janrain_capture_refresh_token']);
  }

  $url = 'https://'
      . (!empty($janrain_capture_optional['captureui_address']) ? $janrain_capture_optional['captureui_address'] : $janrain_capture_main['capture_address'])
      . '/oauth/profile' . $method . '?flags=stay_in_window&access_token='
      . $_SESSION['janrain_capture_access_token']
      . '&callback=closeProfileEditor&xd_receiver='
      . urlencode(url(null, array('absolute' => true)) . drupal_get_path('module', 'janrain_capture') . '/xdcomm.html')
      . (!empty($janrain_capture_optional['capture_application_id']) ? '&application_id=' . $janrain_capture_optional['capture_application_id'] : '');
  
  header("Location: " . $url, true, 302);
  exit;
}

function janrain_capture_profile_sync() {
  global $user;

  $origin = isset($_REQUEST['origin']) ? urldecode($_REQUEST['origin']) : '';

  $api = new Capture_API();
  $profile = $api->load_user_entity();

  if (!$profile) {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to obtain a Capture record', WATCHDOG_ERROR);
    return;
  }

  $userData = array(
    'name' => janrain_capture_profile_field($profile),
    'mail' => $profile['result']['email']
  );
  
  $mergedData = janrain_capture_fields_array($userData, $profile['result']);

  if ($account = user_save($user, $mergedData)) {
    $profile_updated_hook = module_invoke_all('janrain_capture_user_profile_updated', $profile['result'], $account, $origin);
    if (empty($profile_updated_hook) || !in_array(false, $profile_updated_hook))
      drupal_goto($origin);
  }
  else {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to save Capture data to user', WATCHDOG_ERROR);
  }
}

function janrain_capture_profile_field($profile) {
  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $name = !empty($janrain_capture_fields['capture_name_field'])
    ? $janrain_capture_fields['capture_name_field']
    : 'email';
  if (strpos($name, '.')) {
    $names = explode('.', $name);
    $value = $profile['result'];
    foreach ($names as $n) {
      $value = $value[$n];
    }
    return $value;
  }
  else {
    return $profile['result'][$name];
  }
}

function janrain_capture_fields_array($userData, $profile) {
  $fields = module_invoke_all('janrain_capture_fields_array', $profile);
  if (is_array($fields)) {
    foreach ($fields as &$f) {
      if (is_array($f))
        $f = $f[0];
    }
  }
  if (!empty($fields) && is_array($fields)) {
    $userData = array_merge($userData, $fields);
  }
  return $userData;
}

function janrain_capture_resend_verification_email() {
  $hook = module_invoke_all('janrain_capture_verification_resent');
  if (empty($hook) || !in_array(false, $hook)) {
    $email_text = (isset($_SESSION['janrain_capture_profile']['result']['email'])) ? ' to ' . $_SESSION['janrain_capture_profile']['result']['email'] : '';
    drupal_set_message(t('A verification link has been sent@email-text. Please check your email.', array('@email-text' => $email_text)), 'status');
    drupal_goto();
  }
}