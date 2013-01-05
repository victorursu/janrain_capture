<?php

/**
 * @file
 * Admin settings menu for Capture 2.0
 */

/**
 * Sets up the admin settings form for this module.
 */
function janrain_capture_settings() {
  global $base_url;
  $form['description'] = array(
    '#markup' => '<p>' . t('Connection to Janrain Capture requires a subscription. Once subscribed, use the values on the Janrain dashboard to complete the fields below to connect the service.') . '</p>',
  );
  $janrain_capture_ver = variable_get('janrain_capture_ver', array());
  $ver = !empty($janrain_capture_ver) ? $janrain_capture_ver : 'empty';
  
  $form['janrain_capture_ver'] = array(
    '#type' => 'select',
    '#title' => t('Capture Version'),
    '#options' => array(
      '2.0' => t('Capture'),
      '1.0' => t('Capture Legacy'),
    ),
    '#default_value' => !empty($janrain_capture_ver) ? $janrain_capture_ver : '1.0',
    '#description' => t('Which version of Capture are you using? Chose and save before configuring. Settings below will change accordingly and are stored separately.'),
  );
  $janrain_capture_main = variable_get('janrain_capture_main2', array());
  $form['janrain_capture_main2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['janrain_capture_main2']['capture_app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Application ID'),
    '#default_value' => !empty($janrain_capture_main['capture_app_id']) ? $janrain_capture_main['capture_app_id'] : '',
  );
  $form['janrain_capture_main2']['capture_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#default_value' => !empty($janrain_capture_main['capture_client_id']) ? $janrain_capture_main['capture_client_id'] : '',
  );
  $form['janrain_capture_main2']['capture_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Secret'),
    '#default_value' => !empty($janrain_capture_main['capture_client_secret']) ? $janrain_capture_main['capture_client_secret'] : '',
  );
  $form['janrain_capture_main2']['capture_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Capture URL'),
    '#default_value' => !empty($janrain_capture_main['capture_address']) ? $janrain_capture_main['capture_address'] : '',
    '#description' => t('For example, https://myapp.janraincapture.com.'),
  );
  $form['janrain_capture_main2']['engage_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Engage URL'),
    '#default_value' => !empty($janrain_capture_main['engage_address']) ? $janrain_capture_main['engage_address'] : '',
    '#description' => t('For example, https://myapp.rpxnow.com.'),
  );
  
  $janrain_capture_ui = variable_get('janrain_capture_ui2', array());
  $form['janrain_capture_ui2'] = array(
    '#type' => 'fieldset',
    '#title' => t('UI Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['janrain_capture_ui2']['profile_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Capture Profiles'),
    '#default_value' => isset($janrain_capture_ui['profile_enabled']) ? $janrain_capture_ui['profile_enabled'] : 1,
    '#description' => t('Enable/Disable overriding the standard Drupal Profiles'),
  );
  $form['janrain_capture_ui2']['load_js'] = array(
    '#type' => 'textfield',
    '#title' => t('Url for load.js file'),
    '#default_value' => !empty($janrain_capture_ui['load_js']) ? $janrain_capture_ui['load_js'] : 'd16s8pqtk4uodx.cloudfront.net/default/load.js',
    '#description' => t('The absolute url (minus protocol) of the Widget load.js file<br/>(example: d16s8pqtk4uodx.cloudfront.net/default/load.js)'),
  );
  $form['janrain_capture_ui2']['registration_flow'] = array(
    '#type' => 'textfield',
    '#title' => t('Registration Flow'),
    '#default_value' => !empty($janrain_capture_ui['registration_flow']) ? $janrain_capture_ui['registration_flow'] : 'socialRegistration',
    '#description' => t('Change this only when instructed to do so<br/>(default: socialRegistration)'),
  );
  $form['janrain_capture_ui2']['screens_folder'] = array(
    '#type' => 'textfield',
    '#title' => t('Screens Folder'),
    '#default_value' => !empty($janrain_capture_ui['screens_folder']) ? $janrain_capture_ui['screens_folder'] : $base_url.'/sites/all/themes/janrain-capture-screens/',
    '#description' => t('The absolute url of the Capture screens folder<br/>(example: '.$base_url.'/sites/all/themes/janrain-capture-screens/)'),
  );
  $form['janrain_capture_ui2']['recaptcha_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Recaptcha Public Key'),
    '#default_value' => !empty($janrain_capture_ui['recaptcha_key']) ? $janrain_capture_ui['recaptcha_key'] : '',
    '#description' => t('Your Recaptcha Public Key'),
  );
    
  $janrain_capture_federate = variable_get('janrain_capture_federate2', array());
  $form['janrain_capture_federate2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Federate Settings (optional)'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['janrain_capture_federate2']['capture_sso_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable SSO'),
    '#default_value' => isset($janrain_capture_federate['capture_sso_enabled']) ? $janrain_capture_federate['capture_sso_enabled'] : 0,
    '#description' => t('Enable/Disable SSO for Capture 2.0'),
  );
  $form['janrain_capture_federate2']['capture_sso_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Single sign on domain'),
    '#default_value' => !empty($janrain_capture_federate['capture_sso_address']) ? $janrain_capture_federate['capture_sso_address'] : '',
    '#description' => t('For example, myapp.janrainsso.com.'),
  );
  
  $janrain_capture_backplane = variable_get('janrain_capture_backplane2', array());
  $form['janrain_capture_backplane2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Backplane Settings (optional)'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['janrain_capture_backplane2']['backplane_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Backplane'),
    '#default_value' => isset($janrain_capture_backplane['backplane_enabled']) ? $janrain_capture_backplane['backplane_enabled'] : 0,
    '#description' => t('Enable/Disable Backplane for Capture 2.0'),
  );
  $form['janrain_capture_backplane2']['backplane_bus_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane bus name'),
    '#default_value' => !empty($janrain_capture_backplane['backplane_bus_name']) ? $janrain_capture_backplane['backplane_bus_name'] : '',
    '#description' => t('For example, '.$_SERVER['SERVER_NAME']),
  );
  
  $janrain_capture_fields = variable_get('janrain_capture_fields2', array());
  $form['janrain_capture_fields2'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['janrain_capture_fields2']['capture_enforce_verification'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require email verification'),
    '#default_value' => isset($janrain_capture_fields['capture_enforce_verification']) ? $janrain_capture_fields['capture_enforce_verification'] : 0,
    '#description' => t('Prompt users who have not verified their email address to do so before completing authentication'),
  );
  $form['janrain_capture_fields2']['capture_match_email'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match users by email address'),
    '#default_value' => isset($janrain_capture_fields['capture_match_email']) ? $janrain_capture_fields['capture_match_email'] : 0,
    '#description' => t('Associate user and Capture accounts if both share the same email address.'),
    '#states' => array(
      'enabled' => array(
        ':input[name="janrain_capture_fields2[capture_enforce_verification]"]' => array('checked' => TRUE),
      )
    ),
  );
  $form['janrain_capture_fields2']['capture_map_profile_pic'] = array(
    '#type' => 'checkbox',
    '#title' => t('Map profile pic'),
    '#default_value' => isset($janrain_capture_fields['capture_map_profile_pic']) ? $janrain_capture_fields['capture_map_profile_pic'] : 1,
    '#description' => t('Automatically map the Capture profile photo to the Drupal user profile picture.'),
  );
  $form['janrain_capture_fields2']['capture_preferred_photo_variant'] = array(
    '#type' => 'select',
    '#title' => t('Preferred photo variant'),
    '#options' => array(
      'small' => t('Small'),
      'normal' => t('Normal'),
      'large' => t('Large'),
      'square_thumbnail' => t('Square Thumbnail'),
      'original' => t('Original')
    ),
    '#default_value' => isset($janrain_capture_fields['capture_preferred_photo_variant']) ? $janrain_capture_fields['capture_preferred_photo_variant'] : 'small',
    '#description' => t('Select the photo variant to use from the Capture profile.'),
  );
  $form['janrain_capture_enforce'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enforce the use of Janrain Capture.'),
    '#default_value' => variable_get('janrain_capture_enforce', 0),
    '#description' => t('Make it impossible to login to this site outside of Janrain Capture. <br/>WARNING: DO NOT enable this setting until you have made at least one Capture user a Drupal Administrator and all of your user accounts have been migrated to Janrain Capture.'),
  );
  $form['#validate'] = array(
    'janrain_capture_settings_validate2',
  );
  $form['#submit'] = array(
    'janrain_capture_settings_submit',
  );
  
  return system_settings_form($form);
}

/**
 * Validate callback for the settings form.
 */
function janrain_capture_settings_validate2($form, &$form_state) {
  if ($form_state['values']['janrain_capture_fields2']['capture_match_email'] && !$form_state['values']['janrain_capture_fields2']['capture_enforce_verification']) {
    form_set_error('janrain_capture_fields2][capture_match_email', t('You must enable the Enforce Email Verification setting in order to enable the Match Users on Email setting.'));
  }
}

/**
 * Submit callback for the settings form.
 */
function janrain_capture_settings_submit($form, &$form_state) {
  // Rebuild the menu in case the "enforce" setting changed.
  variable_set('menu_rebuild_needed', TRUE);
}
