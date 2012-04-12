<?php
/**
 * @file
 * User page callbacks for the janrain_capture module.
 */

/**
 * Callback Method: Redirects the user to the login URL. Suitable to be used
 * in the menu since this way the menu path is always static.
 */
function janrain_user_register() {
  $login_url = _janrain_capture_get_login_url();
  drupal_goto($login_url);
  exit();
}

/**
 * Page the user is returned to after completing oauth authentication.
 * This page will complete the profile request, merge all mapped data from
 * the Janrain Capture profile to their Drupal fields.
 *
 * Logs the user in by logging into their current account or creating a new
 * Drupal account if a current account can't be found.
 */
function janrain_capture_oauth() {
  module_load_include('inc', 'janrain_capture', 'janrain_capture.api');
  global $user;

  $options = array(
    'token' => isset($_REQUEST['code']) ? $_REQUEST['code'] : '',
    'from_sso' => isset($_REQUEST['from_sso']) ? $_REQUEST['from_sso'] : '',
    'origin' => isset($_REQUEST['origin']) ? $_REQUEST['origin'] : '',
  );

  if ($user->uid) {
    _janrain_capture_oauth_already_signed_in();
  }
  elseif ($options['token']) {
    $profile = _janrain_capture_oauth_get_profile($options);
    _janrain_capture_oauth_save_profile($profile);
  }
  else {
    _janrain_capture_oauth_no_token();
  }

  print theme('janrain_capture_oauth');

  return NULL;
}

/**
 *
 */
function _janrain_capture_oauth_already_signed_in() {
  $no_oauth = module_invoke_all('janrain_capture_already_signed_in');

  if (empty($no_oauth) || !in_array(false, $no_oauth))
    drupal_set_message(t(_janrain_capture_get_message('already_signed_in')), 'error');
}

/**
 *
 */
function _janrain_capture_oauth_get_profile($options) {
  $uri_options = array(
    'absolute' => TRUE,
  );

  if ($options['from_sso'] && $optionsp['origin']) {
    $uri_options['query'] = array(
      'from_sso' => 'true',
      'origin' => $origin
    );
  }

  $redirect_uri = url(JANRAIN_CAPTURE_PATH_OAUTH, $uri_options);

  $api = new Capture_API();
  $api->new_access_token($options['token'], $redirect_uri);
  $profile = $api->load_user_entity();
  return $profile;
}

/**
 *
 */
function _janrain_capture_oauth_no_token() {
  $no_oauth = module_invoke_all('janrain_capture_no_oauth');

  if (empty($no_oauth) || !in_array(false, $no_oauth))
    drupal_set_message(t(_janrain_capture_get_message('already_signed_in')), 'error');
}

/**
 *
 */
function _janrain_capture_oauth_save_profile($profile) {
  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());

  if (!$profile || $profile['stat'] != 'ok') {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to obtain a Capture record', WATCHDOG_ERROR);
  }
  else {
    // Save profile in session
    $_SESSION['janrain_capture_profile'] = $profile;

    $user_info = array(
      'name' => janrain_capture_profile_field($profile),
      'mail' => $profile['result']['email'],
      'status' => 1
    );

    $user_info = janrain_capture_fields_array($user_info, $profile['result']);

    $account = user_external_load($profile['result']['uuid']);
    $new_user = FALSE;

    // No user was found with our Capture uuid.
    if ($account === FALSE) {
      // Look for a local user with the same email address
      if ($local_user = user_load_by_mail($profile['result']['email'])) {
        // Are we configured to match users based on email?
        if (isset($janrain_capture_fields['capture_match_email']) && $janrain_capture_fields['capture_match_email']) {
          // Check to see if this user is already mapped to a Capture uuid
          $query = db_select('authmap', 'a')
            ->fields('a', array('aid'))
            ->condition('module', 'janrain_capture')
            ->condition('uid', $local_user->uid);
          $result = $query->execute();
          if ($result->FetchAll()) {
            $mapped_hook = module_invoke_all('janrain_capture_user_already_mapped');
            if (empty($mapped_hook) || !in_array(false, $mapped_hook))
              drupal_set_message(t('A user with this email address is already mapped.'), 'error');
          }
          else {
            // Bind Capture profile data and uuid to existing user record
            $user_info['auth_janrain_capture'] = $profile['result']['uuid'];
            $account = janrain_capture_user_save($local_user, $user_info);
            if (!$account->uid) {
              $mapping_failed_hook = module_invoke_all('janrain_capture_mapping_failed');
              if (empty($mapping_failed_hook) || !in_array(FALSE, $mapping_failed_hook))
                drupal_set_message(t('Failed to map to existing user.'), 'error');
            }
          }
        }
        else {
          $user_exists_hook = module_invoke_all('janrain_capture_user_exists');
          if (empty($user_exists_hook) || !in_array(FALSE, $user_exists_hook))
            drupal_set_message(t('A user with this email address already exists.'), 'error');
        }
      }
      // No local users with matching email. Create a new one.
      else {
        $user_info['pass'] = md5(REQUEST_TIME . $profile['result']['uuid']);
        $user_info['auth_janrain_capture'] = $profile['result']['uuid'];
        $account = janrain_capture_user_save(NULL, $user_info);
        $new_user = TRUE;
        if (!$account->uid) {
          $failed_create = module_invoke_all('janrain_capture_failed_create');
          if (empty($failed_create) || !in_array(FALSE, $failed_create))
            drupal_set_message(t('Failed to create new user.'), 'error');
        }
      }
    }
    else {
      // We found the user! Re-sync Capture data to make sure we're up to date.
      $account = janrain_capture_user_save($account, $user_info);
    }

    $signin = TRUE;
    if (!$account->uid)
      $signin = FALSE;

    if (isset($_SESSION['janrain_capture_action'])
          && ($_SESSION['janrain_capture_action'] == 'finish_third_party'
          || $_SESSION['janrain_capture_action'] == 'legacy_register')
          && isset($janrain_capture_fields['capture_enforce_verification'])
          && $janrain_capture_fields['capture_enforce_verification']
          && $profile['result']['emailVerified'] == NULL) {
        $signin = FALSE;
        $email_text = (isset($_SESSION['janrain_capture_profile']['result']['email'])) ? ' to ' . $_SESSION['janrain_capture_profile']['result']['email'] : '';
        drupal_set_message(t('A verification link has been sent@email-text. Please check your email.', array('@email-text' => $email_text)), 'status');
    }
    elseif (isset($janrain_capture_fields['capture_enforce_verification'])
        && $janrain_capture_fields['capture_enforce_verification']
        && $profile['result']['emailVerified'] == NULL) {
      $signin = FALSE;
      $resend_link = 'https://'
        . (!empty($janrain_capture_optional['captureui_address']) ? $janrain_capture_optional['captureui_address'] : $janrain_capture_main['capture_address'])
        . '/oauth/resend_verification_email?access_token='
        . $_SESSION['janrain_capture_access_token'] . '&redirect_uri='
        . urlencode(url('janrain_capture/resend_verification_email', array('absolute' => TRUE)));
      drupal_set_message(t('Your email address has not yet been verified. Please check your email and try again. <a href="@resend-link">Click Here</a> to have this email resent.', array('@resend-link' => $resend_link)), 'error');
    }

    if ($signin) {
      $form_state['uid'] = $account->uid;
      user_login_submit(array(), $form_state);
      module_invoke_all('janrain_capture_user_authenticated', $profile['result'], $account, $new_user);
    }
  }
}

/**
 *
 */
function janrain_capture_profile() {
  module_load_include('inc', 'janrain_capture', 'janrain_capture.api');

  $method = isset($_GET['method']) ? $_GET['method'] : '';
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());

  if (REQUEST_TIME >= $_SESSION['janrain_capture_expires_in']) {
    $api = new Capture_API();
    $api->refresh_access_token($_SESSION['janrain_capture_refresh_token']);
  }

  $url = 'https://'
      . (!empty($janrain_capture_optional['captureui_address']) ? $janrain_capture_optional['captureui_address'] : $janrain_capture_main['capture_address'])
      . '/oauth/profile' . $method . '?flags=stay_in_window&access_token='
      . $_SESSION['janrain_capture_access_token']
      . '&callback=closeProfileEditor&xd_receiver='
      . urlencode(url(NULL, array('absolute' => TRUE)) . drupal_get_path('module', 'janrain_capture') . '/xdcomm.html');

  header("Location: " . $url, TRUE, 302);
  exit;
}

/**
 *
 */
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

  $user_data = array(
    'name' => janrain_capture_profile_field($profile),
    'mail' => $profile['result']['email']
  );

  $merged_data = janrain_capture_fields_array($user_data, $profile['result']);

  if ($account = janrain_capture_user_save($user, $merged_data)) {
    $profile_updated_hook = module_invoke_all('janrain_capture_user_profile_updated', $profile['result'], $account, $origin);
    if (empty($profile_updated_hook) || !in_array(false, $profile_updated_hook))
      drupal_goto($origin);
  }
  else {
    drupal_set_message(t('We were unable to complete your request.'), 'error');
    watchdog('janrain_capture', 'Failed to save Capture data to user', WATCHDOG_ERROR);
  }
}

/**
 *
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
 *
 */
function janrain_capture_fields_array($user_data, $profile) {
  $fields = module_invoke_all('janrain_capture_fields_array', $profile);
  // TODO: This needs to be fixed to deal with merged crap.

  if (!empty($fields) && is_array($fields)) {
    $user_data = array_merge($user_data, $fields);
  }

  return $user_data;
}

/**
 * @param array
 *   Profile as returned from the profile API thingy.
 */
function janrain_capture_janrain_capture_fields_array($profile) {
  $user_info = array();

  $catalog = _janrain_capture_drupal_field_catalog();
  $map = _janrain_capture_get_field_map();
  $janrain_fields = _janrain_capture_get_mappable_capture_fields(array('path', 'parsed_path'));

  foreach ($map as $mapped_field) {
    if (isset($janrain_fields[$mapped_field['fid']])) {
      $mapped_field_data = $janrain_fields[$mapped_field['fid']];
      $mapped_field_data['parsed_path'] = unserialize($mapped_field_data['parsed_path']);
      if ($mapped_field_data['parsed_path'][0] == 'profile') {
        array_shift($mapped_field_data['parsed_path']);
        foreach ($profile['profiles'] as $key => $prof) {
          $data = $prof['profile'];
          foreach ($mapped_field_data['parsed_path'] as $k => $v) {
            if (isset($data[$v]) === TRUE) {
              $data = $data[$v];
            }
            else {
              break;
            }
          }
          if ($data != $prof && !is_array($data) && is_string($data) && drupal_strlen($data) > 0) {
            $user_info['profiles'][$mapped_field['bundle']][$mapped_field['field']][LANGUAGE_NONE][0]['value'] = $data;
          }
        }
      }
      else {
        // Not on a profile?
      }
    }
  }

  return $user_info;
}

/**
 * Saves a user object with new fields. This method has provisions for saving
 * fields to profiles that are associated with the user.
 *
 * TODO: Needs to be tested again.
 *
 * @param object
 *   The $user object to save.
 * @param array
 *   Extra fields to be saved to the $user object.
 *
 * @return object
 *   The user object after it has been saved.
 */
function janrain_capture_user_save($current_user, $user_data) {
  $profile_data = array();
  if (isset($user_data['profiles'])) {
    $profile_data = $user_data['profiles'];
    unset($user_data['profiles']);
  }

  $user = user_save($current_user, $user_data);

  foreach ($profile_data as $profile_name => $value) {
    $profile = profile2_load_by_user($user, $profile_name);
    if (!$profile) {
      $profile = profile_create(array('type' => $profile_name));
      $profile->setUser($user);
      $profile->is_new = TRUE;
    }
    foreach ($value as $k => $v) {
      $profile->$k = $v;
    }
    profile2_save($profile);
  }

  return $user;
}

/**
 * TODO: I don't think this does anything.
 */
function janrain_capture_resend_verification_email() {
  $hook = module_invoke_all('janrain_capture_verification_resent');
  if (empty($hook) || !in_array(false, $hook)) {
    $email_text = (isset($_SESSION['janrain_capture_profile']['result']['email'])) ? ' to ' . $_SESSION['janrain_capture_profile']['result']['email'] : '';
    drupal_set_message(t('A verification link has been sent@email-text. Please check your email.', array('@email-text' => $email_text)), 'status');
    drupal_goto();
  }
}

/**
 * TODO: This doesn't seem to ever have a link to it or be called anywhere.
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
