<?php

/**
 * @file
 *
 * API Client for making calls to the Janrain Capture web service
 *
 */
class Capture_API {

  protected $args;
  protected $capture_addr;

  function __construct() {
    $janrain_capture_main = variable_get('janrain_capture_main', array());
    $janrain_capture_optional = variable_get('janrain_capture_optional', array());
    
    $this->args = array();
    $this->args['client_id'] = isset($janrain_capture_main['capture_client_id'])
        ? $janrain_capture_main['capture_client_id']
        : '';
    $this->args['client_secret'] = isset($janrain_capture_main['capture_client_secret'])
        ? $janrain_capture_main['capture_client_secret']
        : '';
    if (isset($janrain_capture_optional['capture_application_id']) && !empty($janrain_capture_optional['capture_application_id']))
      $this->args['application_id'] = $janrain_capture_optional['capture_application_id'];
    
    $this->capture_addr = !empty($janrain_capture_main['capture_address'])
      ? $janrain_capture_main['capture_address']
      : '';
  }

  /**
   * Performs the HTTP request.
   *
   * @param string $command
   *   The Capture command to perform
   * @param array $arg_array
   *   The data set to pass via POST
   * @param string $access_token
   *   The client access token to use when performing user-specific calls
   * @return
   *   The HTTP request result data
   */
  protected function call($command, $arg_array = null, $access_token = null) {

    $url = "https://" . $this->capture_addr . "/$command";

    $headers = array(
      'Content-Type' => 'application/x-www-form-urlencoded',
      'User-Agent' => 'Drupal'
    );
    if (isset($access_token))
      $headers['Authorization'] = "OAuth $access_token";

    if (isset($arg_array)) {
      $arg_array = array_merge($arg_array, $this->args);
      $result = drupal_http_request($url, $headers, "POST", http_build_query($arg_array, '', '&'));
    }
    else {
      $result = drupal_http_request($url, $headers);
    }

    if (!isset($result->data)) {
      return false;
    }

    $json_data = json_decode($result->data, true);

    if ($json_data['stat'] == 'error') {
      $error = new stdClass();
      $error->code = $json_data['code'];
      $error->error = $json_data['error'];
      $this->report_error($error);
      return false;
    }

    return $json_data;
  }

  /**
   * Updates session variables with Capture user tokens
   *
   * @param string $json_data
   *   The data received from the HTTP request containing the tokens
   */
  protected function update_capture_session($json_data) {
    $_SESSION['janrain_capture_access_token'] = $json_data['access_token'];
    $_SESSION['janrain_capture_refresh_token'] = $json_data['refresh_token'];
    $_SESSION['janrain_capture_expires_in'] = time() + $json_data['expires_in'];

    $password_recover = (isset($json_data['transaction_state']['capture']['password_recover'])
        && $json_data['transaction_state']['capture']['password_recover'] == true) ? true : false;
    $_SESSION['janrain_capture_password_recover'] = $password_recover;
    if (isset($json_data['transaction_state']['capture']['action']))
      $_SESSION['janrain_capture_action'] = $json_data['transaction_state']['capture']['action'];
  }

  /**
   * Helper function for the Engage web API wrappers.
   * 
   * @param stdClass $result
   *   Result containing error code and message
   */
  protected function report_error($result) {
    watchdog('janrain_capture', 'Capture web API seems to be inaccessible due to "%error".', array('%error' => $result->code . ' ' . $result->error), WATCHDOG_WARNING);
    drupal_set_message(t('Capture web API seems to be inaccessible because of error "%error".', array('%error' => $result->code . ' ' . $result->error)), 'error');
  }

  /**
   * Perform the exchange to generate a new Access Token
   *
   * @param string $auth_code
   *   The authorization token to use for the exchange
   * @param array $arg_array
   *   The data set to pass via POST
   * @param string $access_token
   *   The client access token to use when performing user-specific calls
   */
  public function new_access_token($auth_code, $redirect_uri) {
    $command = "oauth/token";
    $arg_array = array('code' => $auth_code,
      'redirect_uri' => $redirect_uri,
      'grant_type' => 'authorization_code'
    );

    $json_data = $this->call($command, $arg_array);
    if ($json_data) {
      $this->update_capture_session($json_data);
      return true;
    }

    return false;
  }
  
  function refresh_access_token($refresh_token) {
    $command = "oauth/token";
    $arg_array = array('refresh_token' => $refresh_token,
      'grant_type' => 'refresh_token'
    );

    $json_data = $this->call($command, $arg_array);

    if ($json_data) {
      $this->update_capture_session($json_data);
      return true;
    }

    return false;
  }

  public function load_user_entity($can_refresh = true) {
    if (!$_SESSION['janrain_capture_access_token'])
      return null;

    $user_entity = null;

    $need_to_refresh = false;

    // Check if we need to refresh the access token
    if (time() >= $_SESSION['janrain_capture_expires_in'])
      $need_to_refresh = true;
    else {
      $user_entity = $this->call('entity', array(), $_SESSION['janrain_capture_access_token']);
      if (isset($user_entity['code']) && $user_entity['code'] == '414')
        $need_to_refresh = true;
    }

    // If necessary, refresh the access token and try to fetch the entity again.
    if ($need_to_refresh) {
      if ($can_refresh) {
        if ($this->refresh_access_token($_SESSION['janrain_capture_refresh_token']))
          return $this->load_user_entity(false);
        else
          return null;
      }
    }

    return $user_entity;
  }

}