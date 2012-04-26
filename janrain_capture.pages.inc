<?php

/**
 * @file
 * User page callbacks for the janrain_capture module.
 */

/**
 * Callback for the janrain_capture/oauth menu item. This serves as the
 * redirect_uri Capture redirects the user to and performs the authentication.
 */
function janrain_capture_oauth() {
  global $user;
  $token = isset($_REQUEST['code']) ? $_REQUEST['code'] : '';
  $origin = isset($_REQUEST['origin']) ? $_REQUEST['origin'] : '';
  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());

  if ($user->uid) {
    drupal_set_message(t("You're already signed in!"), 'error');
  }
  elseif ($token) {
    // Cannot use the Drupal url query option because drupal_http_build_query
    // decodes slashes causing incompatibility.
    $redirect_uri = url('janrain_capture/oauth', array('absolute' => TRUE));
    if ($origin) {
      $redirect_uri .= (strpos($redirect_uri, '?') !== FALSE ? '&' : '?') . 'origin=' . urlencode($origin);
    }
    $api = new JanrainCaptureApi();
    $api->newAccessToken($token, $redirect_uri);
    $profile = $api->loadUserEntity();

    if (!$profile || $profile['stat'] != 'ok') {
      drupal_set_message(t('We were unable to complete your request.'), 'error');
      watchdog('janrain_capture', 'Failed to obtain a Capture record', WATCHDOG_ERROR);
    }
    else {
      // Save profile in session.
      $_SESSION['janrain_capture_profile'] = $profile;

      $user_info = array(
        'name' => janrain_capture_profile_field($profile),
        'mail' => $profile['result']['email'],
        'status' => 1,
      );

      $user_info = janrain_capture_fields_array($user_info, $profile['result']);

      $account = user_external_load($profile['result']['uuid']);
      $new_user = FALSE;

      // No user was found with our Capture uuid.
      if ($account === FALSE) {
        // Look for a local user with the same email address.
        $local_user = reset(user_load_multiple(array(), array('mail' => $profile['result']['email'])));
        if ($local_user) {

          // Are we configured to match users based on email?
          if (isset($janrain_capture_fields['capture_match_email']) && $janrain_capture_fields['capture_match_email']) {
            // Check to see if this user is already mapped to a Capture uuid.
            $result = db_query("SELECT aid FROM {authmap} WHERE module = 'janrain_capture' AND uid = :uid", array(':uid' => $local_user->uid))->fetchAllKeyed();
            if (count($result)) {
              $mapped_hook = module_invoke_all('janrain_capture_user_already_mapped');
              if (empty($mapped_hook) || !in_array(FALSE, $mapped_hook)) {
                drupal_set_message(t('A user with this email address is already mapped.'), 'error');
              }
            }
            // Bind Capture profile data and uuid to existing user record.
            else {
              user_set_authmaps($local_user, array("authname_janrain_capture" => $profile['result']['uuid']));
              $account = $local_user;
            }
          }
          else {
            $user_exists_hook = module_invoke_all('janrain_capture_user_exists');
            if (empty($user_exists_hook) || !in_array(FALSE, $user_exists_hook)) {
              drupal_set_message(t('A user with this email address already exists.'), 'error');
            }
          }
        }
        // No local users with matching email. Create a new one.
        else {
          $user_info['pass'] = md5(time() . $profile['result']['uuid']);
          $account = user_save(NULL, $user_info);
          $new_user = TRUE;
          if (!$account->uid) {
            $failed_create = module_invoke_all('janrain_capture_failed_create');
            if (empty($failed_create) || !in_array(FALSE, $failed_create)) {
              drupal_set_message(t('Failed to create new user.'), 'error');
            }
          }
          else {
            user_set_authmaps($account, array("authname_janrain_capture" => $profile['result']['uuid']));
          }
        }
      }
      // We found the user! Re-sync Capture data to make sure we're up to date.
      else {
        $account = user_save($account, $user_info);
      }

      $signin = TRUE;
      if ($account === FALSE) {
        $signin = FALSE;
      }
      if (isset($_SESSION['janrain_capture_action'])
            && ($_SESSION['janrain_capture_action'] == 'finish_third_party'
            || $_SESSION['janrain_capture_action'] == 'legacy_register')
            && isset($janrain_capture_fields['capture_enforce_verification'])
            && $janrain_capture_fields['capture_enforce_verification']
            && $profile['result']['emailVerified'] == NULL) {
        $signin = FALSE;
        if (isset($_SESSION['janrain_capture_profile']['result']['email'])) {
          drupal_set_message(t('A verification link has been sent to @email. Please check your email.', array('@email' => $_SESSION['janrain_capture_profile']['result']['email'])), 'status');
        }
        else {
          drupal_set_message(t('A verification link has been sent. Please check your email.'), 'status');
        }
      }
      elseif (isset($janrain_capture_fields['capture_enforce_verification'])
          && $janrain_capture_fields['capture_enforce_verification']
          && $profile['result']['emailVerified'] == NULL) {
        $signin = FALSE;
        $args = array(
          'action' => 'resend_verification_email',
          'access_token' => $_SESSION['janrain_capture_access_token'],
          'redirect_uri' => url('janrain_capture/resend_verification_email', array('absolute' => TRUE)),
        );
        $resend_link = janrain_capture_url($args);
        $email_unverified = module_invoke_all('janrain_capture_email_unverified', $resend_link);
        if (empty($email_unverified) || !in_array(FALSE, $email_unverified)) {
          drupal_set_message(t('Your email address has not yet been verified. Please check your email and try again. <a href="@resend-link">Click here</a> to have this email resent.', array('@resend-link' => $resend_link)), 'error');
        }
      }

      if ($signin) {
        $form_state['uid'] = $account->uid;
        user_login_submit(array(), $form_state);
        module_invoke_all('janrain_capture_user_authenticated', $profile['result'], $account, $new_user);
      }
    }
  }
  else {
    $no_oauth = module_invoke_all('janrain_capture_no_oauth');
    if (empty($no_oauth) || !in_array(FALSE, $no_oauth)) {
      drupal_set_message(t('No Oauth token found!'), 'error');
    }
  }
  print theme('janrain_capture_oauth');
  return NULL;
}

/**
 * Callback for the janrain_capture/profile menu item. Ensures the access_token
 * is valid before redirecting the user to the Capture profile screen.
 */
function janrain_capture_profile() {
  $method = isset($_GET['method']) ? $_GET['method'] : '';
  $callback = isset($_GET['callback']) ? $_GET['callback'] : 'CAPTURE.closeProfileEditor';
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  $redirect_uri = url('janrain_capture/oauth', array('absolute' => TRUE));

  if (time() >= $_SESSION['janrain_capture_expires_in']) {
    $api = new JanrainCaptureApi();
    $api->refreshAccessToken($_SESSION['janrain_capture_refresh_token']);
  }

  $args = array(
    'action' => 'profile' . $method,
    'access_token' => $_SESSION['janrain_capture_access_token'],
    'callback' => $callback,
  );

  $url = janrain_capture_url($args);

  drupal_goto($url);
}

/**
 * Callback for the janrain_capture/profile_sync menu item. Retrieves
 * the most recent data from Capture and stores values locally.
 */
function janrain_capture_profile_sync() {
  global $user;

  $origin = isset($_REQUEST['origin']) ? urldecode($_REQUEST['origin']) : '';

  $api = new JanrainCaptureApi();
  $profile = $api->loadUserEntity();

  if (!$profile) {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to obtain a Capture record', WATCHDOG_ERROR);
    return;
  }

  $user_data = array(
    'name' => janrain_capture_profile_field($profile),
    'mail' => $profile['result']['email'],
  );

  $merged_data = janrain_capture_fields_array($user_data, $profile['result']);

  if ($account = user_save($user, $merged_data)) {
    $profile_updated_hook = module_invoke_all('janrain_capture_user_profile_updated', $profile['result'], $account, $origin);
    if (empty($profile_updated_hook) || !in_array(FALSE, $profile_updated_hook)) {
      drupal_goto($origin);
    }
  }
  else {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to save Capture data to user', WATCHDOG_ERROR);
  }
}

/**
 * Function to retrive the value of a dot-delimited field name from the
 * returned Capture user profile.
 */
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

/**
 * Function to create the array of user data to save.
 */
function janrain_capture_fields_array($user_data, $profile) {
  $fields = module_invoke_all('janrain_capture_fields_array', $profile);
  if (is_array($fields)) {
    foreach ($fields as &$f) {
      if (is_array($f)) {
        $f = $f[0];
      }
    }
  }
  if (!empty($fields) && is_array($fields)) {
    $user_data = array_merge($user_data, $fields);
  }
  return $user_data;
}

/**
 * Callback for the janrain_capture/resend_verification_email menu item.
 * Displays a confirmation message that a verification email was resent.
 */
function janrain_capture_resend_verification_email() {
  $hook = module_invoke_all('janrain_capture_verification_resent');
  if (empty($hook) || !in_array(FALSE, $hook)) {
    if (isset($_SESSION['janrain_capture_profile']['result']['email'])) {
      drupal_set_message(t('A verification link has been sent to @email. Please check your email.', array('@email' => $_SESSION['janrain_capture_profile']['result']['email'])), 'status');
    }
    else {
      drupal_set_message(t('A verification link has been sent. Please check your email.'), 'status');
    }
    drupal_goto();
  }
}

/**
 * Callback the janrain_capture/token_expired menu item. Logs out a user
 * due to an expired session.
 */
function janrain_capture_token_expired() {
  global $user;
  $tmp = NULL;
  session_destroy();
  user_module_invoke('logout', $tmp, $user);
  $user = drupal_anonymous_user();

  drupal_set_message(t('Your session has expired. Please sign in again.'), 'error');
  drupal_goto();
}
