<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Logger\LoggerChannelFactoryInterface;
use Drupal\user\PrivateTempStoreFactory;
use GuzzleHttp\Client;

/**
 * Integration of Janrain API with Drupal.
 */
class JanrainCaptureApiService {

  /**
   * Janrain Capture API.
   *
   * @var \Drupal\janrain_capture\JanrainCaptureApi
   */
  private $janrainCaptureAPI;

  /**
   * User private temp store.
   *
   * @var \Drupal\user\PrivateTempStore
   */
  private $tempStore;

  /**
   * JanrainCaptureApiService constructor.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   Configuration factory.
   * @param \Drupal\Core\Logger\LoggerChannelFactoryInterface $logger_factory
   *   Logger factory.
   * @param \GuzzleHttp\Client $http_client
   *   Http client.
   * @param \Drupal\user\PrivateTempStoreFactory $temp_store_factory
   *   User temporary store factory.
   */
  public function __construct(ConfigFactoryInterface $config_factory, LoggerChannelFactoryInterface $logger_factory, Client $http_client, PrivateTempStoreFactory $temp_store_factory) {
    $config = $config_factory->get('janrain_capture.settings');
    $this->janrainCaptureAPI = new JanrainCaptureApi($config->get('capture')['client_id'], $config->get('capture')['client_secret'], $config->get('capture')['capture_server'], $logger_factory->get('janrain_capture'), $http_client);
    $this->tempStore = $temp_store_factory->get('janrain_capture');
  }

  /**
   * Updates session variables with Capture user tokens.
   *
   * @param object $data
   *   The data received from the HTTP request containing the tokens.
   */
  public function updateCaptureSession($data) {
    $this->tempStore->set('access_token', $data->access_token);
    $this->tempStore->set('refresh_token', $data->refresh_token);
    $this->tempStore->set('expires_in', REQUEST_TIME + $data->expires_in);
  }

  /**
   * Get access token from auth code.
   *
   * @param string $auth_code
   *   Authentication code.
   * @param string $redirect_uri
   *   Redirect Uri.
   *
   * @return object|bool
   *   Token data on success.
   */
  public function newAccessToken($auth_code, $redirect_uri) {
    $token_info = $this->janrainCaptureAPI->getNewAccessToken($auth_code, $redirect_uri);
    if ($token_info) {
      $this->updateCaptureSession($token_info);
      return $token_info;
    }
    return FALSE;
  }

  /**
   * Retrieves a new access_token/refresh_token set.
   *
   * @return bool
   *   TRUE on success.
   */
  public function refreshAccessToken() {
    $access_token = $this->tempStore->get('access_token');
    $refresh_token = $this->tempStore->get('refresh_token');
    if (empty($access_token)) {
      return FALSE;
    }
    $token_info = $this->janrainCaptureAPI->getRefreshedAccessToken($refresh_token);
    if ($token_info) {
      $this->updateCaptureSession($token_info);
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Retrieves the user entity from Capture.
   *
   * @return object
   *   User entity from Capture.
   */
  public function getUserEntity() {
    $access_token = $this->tempStore->get('access_token');
    $expires_in = $this->tempStore->get('expires_in');
    if (empty($access_token)) {
      return NULL;
    }
    $need_to_refresh = FALSE;
    if (REQUEST_TIME >= $expires_in) {
      $need_to_refresh = TRUE;
    }
    else {
      $user_entity = $this->janrainCaptureAPI->getUserEntity($access_token);
      if (isset($user_entity->code) && $user_entity->code == '414') {
        $need_to_refresh = TRUE;
      }
    }
    if ($need_to_refresh) {
      if ($this->refreshAccessToken()) {
        $user_entity = $this->janrainCaptureAPI->getUserEntity($this->tempStore->get('access_token'));
      }
    }
    // Return NULL if there is an error code.
    return isset($user_entity->code) ? NULL : $user_entity;
  }

}
