<?php
/**
 * @file
 * Widget-related functions
 */

/**
 * Adds widget JS settings to the page.
 */
function janrain_capture_widget_add_settings($settings = array()) {
  // Widget settings
  $janrain_capture_main = variable_get('janrain_capture_main2', array());
  $janrain_capture_main = array_merge($janrain_capture_main, variable_get('janrain_capture_ui2', array()));
  $janrain_capture_optional = variable_get('janrain_capture_federate2', array());
  $janrain_capture_optional = array_merge($janrain_capture_optional, variable_get('janrain_capture_backplane2', array()));

  if (!empty($janrain_capture_optional['capture_sso_address'])) {
    $settings['janrainCapture']['sso_address'] = $janrain_capture_optional['capture_sso_address'];
  }

  if (isset($janrain_capture_optional['backplane_enabled'])
      && !empty($janrain_capture_optional['backplane_bus_name'])) {
    $settings['janrainCapture']['backplane_enabled'] = $janrain_capture_optional['backplane_enabled'];
    $settings['janrainCapture']['backplane_bus_name'] = $janrain_capture_optional['backplane_bus_name'];
  }

  // Add settings array into a JS variable
  drupal_add_js($settings, array('type' => 'setting', 'every_page' => TRUE, 'preprocess' => FALSE, 'weight' => 0, 'scope' => 'header',));
}

/**
 * Adds widget JS scripts to the page.
 */
function janrain_capture_widget_add_scripts() {
  global $base_url;

  // File scripts
  drupal_add_js(drupal_get_path('module', 'janrain_capture') . '/janrain_capture.js', array(
    'type' => 'file',
    'every_page' => TRUE,
    'weight' => 1,
    'preprocess' => FALSE,
    'scope' => 'header',));

  $widget = array(
    '#type' => 'markup',
    '#prefix' => '<script type="text/javascript">',
    '#suffix' => '</script>',
    '#markup' => janrain_capture_widget_js(),
    '#weight' => 3,
  );
  $capture_client = array(
    '#type' => 'markup',
    '#prefix' => '<script type="text/javascript" src="https://d7v0k4dt27zlp.cloudfront.net/assets/capture_client.js">',
    '#suffix' => '</script>',
    '#markup' => '',
    '#weight' => 2,
  );
  $capture_js = array(
    '#type' => 'markup',
    '#prefix' => '<script type="text/javascript" src="'. $base_url . '/' . drupal_get_path('module', 'janrain_capture') . '/janrain_capture.js">',
    '#suffix' => '</script>',
    '#markup' => '',
    '#weight' => 1,
  );

  drupal_add_html_head($widget, 'janrain_capture_widget_js');
  drupal_add_html_head($capture_client, 'janrain_capture_client_js');
}

/**
 * Returns Capture widget js.
 */
function janrain_capture_widget_js() {
  global $base_url;
  global $base_path;
  global $language;

  $janrain_settings = variable_get('janrain_capture_fields2', array());
  $janrain_settings = array_merge($janrain_settings, variable_get('janrain_capture_main2', array()));
  $janrain_settings = array_merge($janrain_settings, variable_get('janrain_capture_ui2', array()));
  $janrain_settings = array_merge($janrain_settings, variable_get('janrain_capture_federate2', array()));
  $janrain_settings = array_merge($janrain_settings, variable_get('janrain_capture_backplane2', array()));

  // capture
  $settings["capture.redirectUri"]        = url('janrain_capture/oauth', array('absolute' => TRUE));
  $settings["capture.appId"]              = $janrain_settings['capture_app_id'];
  $settings["capture.clientId"]           = $janrain_settings['capture_client_id'];
  $settings["capture.responseType"]       = "code";
  $settings["capture.captureServer"]      = $janrain_settings['capture_address'];
  $settings["capture.loadJsUrl"]          = $janrain_settings['load_js'];

  $share_settings = variable_get('janrain_capture_share', array());
  if (isset($share_settings["enabled"]) && $share_settings["enabled"]) {
    $settings["packages"]                 = '["login","capture","share"]';
  } else {
    $settings["packages"]                 = '["login","capture"]';
  }

  // engage
  $settings["appUrl"]                     = $janrain_settings['engage_address'];

  // federate
  $settings["capture.federate"]           = $janrain_settings['capture_sso_enabled'];
  $settings["capture.federateServer"]     = 'https://' . $janrain_settings['capture_sso_address'];
  $settings["capture.federateXdReceiver"] = $base_url . base_path() . drupal_get_path('module', 'janrain_capture') . '/xdcomm.html';
  $settings["capture.federateLogoutUri"]  = url('janrain_capture/simple_logout', array('absolute' => TRUE));
  $settings["capture.federateSegment"]    = isset($janrain_settings['capture_sso_segment_name']) ? $janrain_settings['capture_sso_segment_name'] : '';

  if (isset($janrain_settings['capture_sso_supported_segment_names'])) {
    $segment_names = explode(',', $janrain_settings['capture_sso_supported_segment_names']);

    if ($segment_names) {
      $settings["capture.federateSupportedSegments"] = json_encode($segment_names);
    }
  }

  // backplane
  $settings["capture.backplane"]          = $janrain_settings['backplane_enabled'];
  $settings["capture.backplaneServerBaseUrl"] = isset($janrain_settings['backplane_server_base_url']) ? $janrain_settings['backplane_server_base_url'] : '';
  $settings["capture.backplaneBusName"]   = $janrain_settings['backplane_bus_name'];
  $settings["capture.backplaneVersion"]   = $janrain_settings['backplane_version'];

  // miscellaneous
  $settings["capture.language"]           = $language->language;

  if (module_exists('janrain_capture_screens')) {
    $settings["capture.stylesheets"] = "'" . file_create_url(_janrain_capture_get_screen_file('stylesheets/styles.css')) . "'";
    if ($mobile_stylesheet = _janrain_capture_get_screen_file('stylesheets/mobile-styles.css')) {
      $settings["capture.mobileStylesheets"] = "'" . file_create_url($mobile_stylesheet) . "'";
    }
    if ($ie_stylesheet = _janrain_capture_get_screen_file('stylesheets/ie-styles.css')) {
      $settings["capture.conditionalIEStylesheets"] = "'" . file_create_url($ie_stylesheet) . "'";
    }
  }
  else {
    $folder_url = variable_get('janrain_capture_screens_folder', 'file:///sites/all/themes/janrain-capture-screens/');

    // If path is local, search for user agent-specific stylesheets in the file system.
    if (strpos($folder_url, 'file:///', 0) === 0) {
      // Example of $folder_url: file:///sites/all/themes/janrain-capture-screens/
      $web_path = str_replace('file://', '', $folder_url);
      // Example of $web_path: /sites/all/themes/janrain-capture-screens/
      $fs_path = DRUPAL_ROOT . $web_path;
      // Example of $fs_path: /var/www/d7/sites/all/themes/janrain-capture-screens/
      //watchdog('janrain_capture', 'Searching for Capture screen stylesheets in: ' . $fs_path,  array(), WATCHDOG_DEBUG);
      $web_url = $base_url . $web_path;

      $is_nested = is_dir($fs_path.'stylesheets');

      if ($is_nested) {
        $fs_path .= "stylesheets";
      }

      try {
        $dir = new DirectoryIterator($fs_path);
      }
      catch (UnexpectedValueException $e) {
        if ($is_nested) {
          $screens_message = 'No "stylesheets" subfolder found in the screens folder (@path). Please check the Janrain Capture module settings.';

        }
        else {
          $screens_message = 'The screens folder (@path) could not be found. Please check the Janrain Capture module settings.';
        }

        watchdog('janrain_capture',
                 $screens_message,
                 array('@path' => $fs_path),
                 WATCHDOG_WARNING,
                 l(t('Janrain Capture module settings'), 'admin/config/people/janrain_capture'));

        return;
      }

      if ($is_nested) {
        $web_url .= "stylesheets/";
      }

      $css = array();
      $css_mobile = array();
      $css_ie = array();
      if ($dir) {
        foreach ($dir as $fileinfo) {
          $fn = $fileinfo->getFilename();
          if (!$fileinfo->isDot() && stripos($fn,'.css')) {
            switch (true){
              // if file begins with mobile set is as a mobile style
            case (stripos($fn, 'mobile') === 0):
              $css_mobile[] = "'{$web_url}{$fn}'";
              break;
              // if file begins with ie set is as an IE style
            case (stripos($fn, 'ie') === 0):
              $css_ie[] = "'{$web_url}{$fn}'";
              break;
              // otherwise set it as a normal style
            default:
              $css[] = "'{$web_url}{$fn}'";
              break;
            }
          }
        }
      }
      $settings["capture.stylesheets"] = implode(', ', $css);
      $settings["capture.mobileStylesheets"] = implode(', ', $css_mobile);
      $settings["capture.conditionalIEStylesheets"] = implode(', ', $css_ie);
    } else {
      // Remote stylesheets
      $settings["capture.stylesheets"] = "'{$folder_url}stylesheets/styles.css'";
    }

    // Log a warning if directories are setup properly but no stylesheets were found
    if (!count($settings["capture.stylesheets"]) && !count($settings["capture.mobileStylesheets"]) && !count($settings["capture.conditionalIEStylesheets"])) {
      watchdog('janrain_capture',
                 'No stylesheets were found in the screens folder (@path). Please check the Janrain Capture module settings.',
                 array('@path' => $fs_path ?: $folder_url),
                 WATCHDOG_WARNING,
                 l(t('Janrain Capture module settings'), 'admin/config/people/janrain_capture'));
    }
  }



  $output = <<<EOD
function janrainSignOut(){
  janrain.capture.ui.endCaptureSession();
}
(function() {
  if (typeof window.janrain !== 'object') window.janrain = {};
  window.janrain.settings = {};
  window.janrain.settings.capture = {};

  // capture settings
  janrain.settings.capture.redirectUri = '{$settings["capture.redirectUri"]}';
  janrain.settings.capture.appId= '{$settings["capture.appId"]}';
  janrain.settings.capture.clientId = '{$settings["capture.clientId"]}';
  janrain.settings.capture.responseType = '{$settings["capture.responseType"]}';
  janrain.settings.capture.captureServer = '{$settings["capture.captureServer"]}';
  janrain.settings.capture.registerFlow = 'socialRegistration';
  janrain.settings.packages = {$settings['packages']};

  janrain.settings.capture.setProfileCookie = true;
  janrain.settings.capture.keepProfileCookieAfterLogout = true;
  janrain.settings.capture.setProfileData = true;

  // styles
  janrain.settings.capture.stylesheets = [{$settings["capture.stylesheets"]}];

EOD;

  // mobile styles
  if (isset($settings["capture.mobileStylesheets"]) && $settings["capture.mobileStylesheets"] != '') {
    $output .= <<<EOD
  janrain.settings.capture.mobileStylesheets = [{$settings["capture.mobileStylesheets"]}];

EOD;
  }

  //IE styles
  if (isset($settings["capture.conditionalIEStylesheets"]) && $settings["capture.conditionalIEStylesheets"] != '') {
    $output .= <<<EOD
  janrain.settings.capture.conditionalIEStylesheets = [{$settings["capture.conditionalIEStylesheets"]}];

EOD;
  }

  // captcha
  $output .= <<<EOD
  janrain.settings.capture.recaptchaPublicKey = '6LeVKb4SAAAAAGv-hg5i6gtiOV4XrLuCDsJOnYoP';

EOD;

  $output .= <<<EOD
  // engage settings
  janrain.settings.appUrl = '{$settings["appUrl"]}';
  janrain.settings.tokenAction = 'event';

EOD;

  // Backplane
  if ($settings["capture.backplane"]) {
    $output .= <<<EOD
  // backplane settings
  janrain.settings.capture.backplane = '{$settings["capture.backplane"]}';
  janrain.settings.capture.backplaneBusName = '{$settings["capture.backplaneBusName"]}';
  janrain.settings.capture.backplaneVersion = '{$settings["capture.backplaneVersion"]}';

EOD;
    if ($settings['capture.backplaneServerBaseUrl']) {
      $output .= "janrain.settings.capture.backplaneServerBaseUrl = 'https://{$settings['capture.backplaneServerBaseUrl']}';";
    }
  }

  if ($settings["capture.federate"]) {
    $output .= <<<EOD
  // federate settings
  janrain.settings.capture.federate = '{$settings["capture.federate"]}';
  janrain.settings.capture.federateServer = '{$settings["capture.federateServer"]}';
  janrain.settings.capture.federateXdReceiver = '{$settings["capture.federateXdReceiver"]}';
  janrain.settings.capture.federateLogoutUri = '{$settings["capture.federateLogoutUri"]}';

EOD;

    if (isset($settings["capture.federateSegment"])) {
      $output .= "  janrain.settings.capture.federateSegment = '{$settings["capture.federateSegment"]}';
";
    }

    if (isset($settings["capture.federateSupportedSegments"])) {
      $output .= "  janrain.settings.capture.federateSupportedSegments = {$settings["capture.federateSupportedSegments"]};
";
    }

    $output .= "

";
  }

  if ($settings["capture.language"]) {
    $output .= "  janrain.settings.language = '{$settings["capture.language"]}';

";
  }

  if (!isset($_SESSION['janrain_capture_access_token'])) {
    $api = new JanrainCaptureApi();
    $api->refreshAccessToken();
  }
  $access_token = "var access_token = '";
  $access_token .= isset($_SESSION['janrain_capture_access_token']) ? $_SESSION['janrain_capture_access_token'] : "";
  $access_token .= "';";

  $output .= <<<EOD
  function isReady() { janrain.ready = true; };
  if (document.addEventListener) {
    document.addEventListener("DOMContentLoaded", isReady, false);
  } else {
    window.attachEvent('onload', isReady);
  }

  var e = document.createElement('script');
  e.type = 'text/javascript';
  e.id = 'janrainAuthWidget';
  var url = document.location.protocol === 'https:' ? 'https://' : 'http://';
  url += '{$settings["capture.loadJsUrl"]}';
  e.src = url;
  var s = document.getElementsByTagName('script')[0];
  s.parentNode.insertBefore(e, s);
})();
{$access_token}

EOD;

  return $output;
}
