<?php

/**
 * @file janrain_capture.admin.inc
 * Admin settings menu
 */
function janrain_capture_settings() {
  
  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $form['janrain_capture_main'] = array(
    '#type' => 'fieldset',
    '#title' => t('Janrain Capture Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_main']['capture_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Client ID'),
    '#default_value' => !empty($janrain_capture_main['capture_client_id'])
      ? $janrain_capture_main['capture_client_id']
      : '',
    '#description' => t('Your Janrain Capture Client ID')
  );
  $form['janrain_capture_main']['capture_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Client Secret'),
    '#default_value' => !empty($janrain_capture_main['capture_client_secret'])
      ? $janrain_capture_main['capture_client_secret']
      : '',
    '#description' => t('Your Janrain Capture Client Secret')
  );
  $form['janrain_capture_main']['capture_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Address'),
    '#default_value' => !empty($janrain_capture_main['capture_address'])
      ? $janrain_capture_main['capture_address']
      : '',
    '#description' => t('Your Janrain Capture Address (e.g. myapp.janraincapture.com)')
  );
  
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  $form['janrain_capture_optional'] = array(
    '#type' => 'fieldset',
    '#title' => t('Optional Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['janrain_capture_optional']['capture_sso_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture SSO Address'),
    '#default_value' => !empty($janrain_capture_optional['capture_sso_address'])
      ? $janrain_capture_optional['capture_sso_address']
      : '',
    '#description' => t('Your Janrain Capture SSO Address (e.g. myapp.janrainsso.com)')
  );
  $form['janrain_capture_optional']['captureui_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain CaptureUI Address'),
    '#default_value' => !empty($janrain_capture_optional['captureui_address'])
      ? $janrain_capture_optional['captureui_address']
      : '',
    '#description' => t('Your Janrain CaptureUI Address (defaults to Janrain Capture Address)')
  );
  $form['janrain_capture_optional']['backplane_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane Server Base URL'),
    '#default_value' => !empty($janrain_capture_optional['backplane_server'])
      ? $janrain_capture_optional['backplane_server']
      : '',
    '#description' => t('Your Backplane Server Base URL')
  );
  $form['janrain_capture_optional']['backplane_bus_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane Bus Name'),
    '#default_value' => !empty($janrain_capture_optional['backplane_bus_name'])
      ? $janrain_capture_optional['backplane_bus_name']
      : '',
    '#description' => t('Your Backplane Bus Name')
  );
  $form['janrain_capture_optional']['backplane_js_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane JS Path'),
    '#default_value' => !empty($janrain_capture_optional['backplane_js_path'])
      ? $janrain_capture_optional['backplane_js_path']
      : '',
    '#description' => t('The path to backplane.js')
  );
  
  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $form['janrain_capture_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_fields']['capture_name_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile Name Field'),
    '#default_value' => !empty($janrain_capture_fields['capture_name_field'])
      ? $janrain_capture_fields['capture_name_field']
      : 'email',
    '#description' => t('The profile field to use for the local username. Must be unique.')
  );
  $form['janrain_capture_fields']['capture_match_email'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match Users on Email'),
    '#default_value' => !empty($janrain_capture_fields['capture_match_email'])
      ? $janrain_capture_fields['capture_match_email']
      : 0,
    '#description' => t('Match authenticated Capture users with a local record with the same email')
  );
  $form['janrain_capture_fields']['capture_enforce_verification'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enforce Email Verification'),
    '#default_value' => !empty($janrain_capture_fields['capture_enforce_verification'])
      ? $janrain_capture_fields['capture_enforce_verification']
      : 0,
    '#description' => t('Prompt users who have not verified their email address to do so before completing authentication')
  );
  
  $janrain_capture_dependencies = variable_get('janrain_capture_dependencies', array());
  $form['janrain_capture_dependencies'] = array(
    '#type' => 'fieldset',
    '#title' => t('Dependency Locations'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_dependencies']['capture_json2'] = array(
    '#type' => 'textfield',
    '#title' => t('JSON2.js Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_json2'])
      ? $janrain_capture_dependencies['capture_json2']
      : 'sites/all/libraries/json2/json2.js',
    '#description' => t('The path to json2.js releative to your Drupal installation root')
  );
  $form['janrain_capture_dependencies']['capture_fancy_js'] = array(
    '#type' => 'textfield',
    '#title' => t('Fancybox JS Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_fancy_js'])
      ? $janrain_capture_dependencies['capture_fancy_js']
      : 'sites/all/libraries/fancybox/jquery.fancybox-1.3.4.pack.js',
    '#description' => t('The path to the Fancybox javascript library releative to your Drupal installation root')
  );
  $form['janrain_capture_dependencies']['capture_fancy_css'] = array(
    '#type' => 'textfield',
    '#title' => t('Fancybox CSS Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_fancy_css'])
      ? $janrain_capture_dependencies['capture_fancy_css']
      : 'sites/all/libraries/fancybox/jquery.fancybox-1.3.4.css',
    '#description' => t('The path to the Fancybox css stylesheet releative to your Drupal installation root')
  );

  return system_settings_form($form);
}

