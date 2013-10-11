<?php
/**
 * @file
 * Profile-related functions
 */

/**
 * Implementation of hook_user_view to customize profile view
 */
function janrain_capture_user_view($account) {
  global $base_url;
  $ver = variable_get('janrain_capture_ver', JANRAIN_CAPTURE_VERSION_DEFAULT);
  $janrain_capture_ui = variable_get('janrain_capture_ui2', array());
  
  if($ver == JANRAIN_CAPTURE_VERSION_LEGACY
      || isset($janrain_capture_ui['profile_enabled']) == false
      || $janrain_capture_ui['profile_enabled'] != 1) {
    return $account;
  }
  $uuid = $account->field_janrain_capture_uuid['und'][0]['value'];
  $screen = _janrain_capture_get_screen('public-profile.html');

  $account->content = array();
  $account->content['summary'] = array(
      '#type' => 'user_profile_item',
      '#title' => t('Profile'),
      '#markup' => '<div id="janrain-capture-view-profile">'.$screen.'</div>
      <script>
      if(window.location.search != "?uuid='.$uuid.'") {
        window.location.search = "?uuid='.$uuid.'";
       }
      jQuery(document).ready(function(){
       if (jQuery(".profile").parent().attr("class") != "fieldset-wrapper") {
        jQuery(".profile").wrap("<div class=\"fieldset-wrapper\">");
        jQuery(".fieldset-wrapper").wrap("<fieldset class=\"form-wrapper\" id=\"view-profile\">");
       }
      });
       </script>',
      '#attributes' => array('class' => array()),
   );

  if ($js = _janrain_capture_get_screen("public-profile.js")) {
    // JS for the profile screen that should go into <head> before the widget.
    drupal_add_js($js, array('type' => 'inline', 'every_page' => TRUE, 'weight' => 2, 'preprocess' => FALSE));
  }

  return $account;
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function janrain_capture_form_user_profile_form_alter(&$form, &$form_state) {
  global $user;
  global $base_url;

  if ($form['#user_category'] != 'account') {
    // @todo - should core profile module tabs be blocked?
    return;
  }
  unset($form['#validate']);
  $account = $form['#user'];
  $enforce = variable_get('janrain_capture_enforce', FALSE);

  if ($enforce || janrain_capture_mapping_exists($account->uid)) {
    // Account information.
    $form['profile'] = array(
      '#type' => 'fieldset',
      '#weight' => -10,
    );
    $form['account'] = array(
      '#type' => 'fieldset',
      '#weight' => 1,
    );
    if ($user->uid == $account->uid) {
      // test 1.0 or 2.0
      $ver = variable_get('janrain_capture_ver', JANRAIN_CAPTURE_VERSION_DEFAULT);
      if($ver == JANRAIN_CAPTURE_VERSION_WIDGET) {
        
        $janrain_capture_ui = variable_get('janrain_capture_ui2', array());
        if(isset($janrain_capture_ui['profile_enabled']) == false
            || $janrain_capture_ui['profile_enabled'] != 1) {
          
          unset($form['profile']);
          unset($form['account']);
          unset($form['field_janrain_capture_uuid']);
          $form['account']['mail'] = array(
            '#type' => 'item',
            '#title' => t('E-mail address'),
            '#markup' => check_plain($account->mail),
          );
          
          return;
        }
        
        // grab the edit profile screen
        $result = _janrain_capture_get_screen("edit-profile.html");

        $addme ="";
        if (REQUEST_TIME >= $_SESSION['janrain_capture_expires_in']) {
          $api = new JanrainCaptureApi();
          $api->refreshAccessToken();
          $access_token = $_SESSION['janrain_capture_access_token'];
          $addme = "<script type='text/javascript'>access_token = '$access_token';</script>";
        }
        $result = $addme . $result;
        $profile = Array('#markup' => '
            '.$result.'
            <script type="text/javascript">
           jQuery(document).ready(function(){
            jQuery("#user-profile-form").after(jQuery("#janrain-edit-wrapper")).remove();
              if (jQuery("#janrain-edit-wrapper").parent().attr("class") != "fieldset-wrapper") {
                jQuery("#janrain-edit-wrapper").wrap("<div class=\"fieldset-wrapper\">");
                jQuery(".fieldset-wrapper").wrap("<fieldset class=\"form-wrapper\" id=\"edit-profile\">");
               }
           });
            </script>'
        );
        
          $form['profile'] = array(
          '#type' => 'item',
          '#title' => t('Profile'),
          $profile
        );

          if ($js = _janrain_capture_get_screen("edit-profile.js")) {
            // JS for the edit profile screen that should go into <head> before the widget.
            drupal_add_js($js, array('type' => 'inline', 'every_page' => TRUE, 'weight' => 2, 'preprocess' => FALSE));
          }

          return;
      } else {
        $profile = Array('#markup' => '<div style="height:1300px;"></div>
            <script type="text/javascript">jQuery("#edit-janrain-capture-profile").html("<iframe width=\"900\" height=\"1300\" frameborder=\"0\" src=\"'.$base_url.'/janrain_capture/profile\" />");</script>'
        );
      }
    } else {
       $profile = array('#markup' => t('Profile managed in Janrain Capture'));
    }

     $form['profile']['janrain_capture_profile'] = array(
      '#type' => 'item',
      '#title' => t('Profile'),
      $profile
     );
     $form['account']['mail'] = array(
      '#type' => 'item',
      '#title' => t('E-mail address'),
      '#markup' => check_plain($account->mail),
    );
  }
}
