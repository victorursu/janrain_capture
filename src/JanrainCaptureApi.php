<?php

namespace Drupal\janrain_capture;

use GuzzleHttp\Client;
use GuzzleHttp\Exception\GuzzleException;

/**
 * An API Client for making calls to the Janrain Capture web service.
 */
class JanrainCaptureApi {

  /**
   * API client id.
   *
   * @var string
   */
  private $clientId;

  /**
   * API client secret id.
   *
   * @var string
   */
  private $clientSecret;

  /**
   * Janrain capture address.
   *
   * @var string
   */
  private $captureAddress;

  /**
   * HTTP client.
   *
   * @var \GuzzleHttp\Client
   */
  private $httpClient;

  /**
   * Logger.
   *
   * @var \Psr\Log\LoggerInterface
   */
  private $logger;

  /**
   * JanrainCaptureApi constructor.
   *
   * @param string $clientId
   *   API client id.
   * @param string $clientSecret
   *   API client secret id.
   * @param string $captureAddress
   *   Janrain capture address.
   * @param \GuzzleHttp\Client $httpClient
   *   HTTP client.
   */
  public function __construct($clientId, $clientSecret, $captureAddress, Client $httpClient) {
    $this->clientId = $clientId;
    $this->clientSecret = $clientSecret;
    $this->captureAddress = $captureAddress;
    $this->httpClient = $httpClient;
  }

  /**
   * Performs the HTTP request to Janrain.
   *
   * @param string $command
   *   The Capture command to perform.
   * @param array $data
   *   The data set to pass via POST.
   * @param string $access_token
   *   The client access token to use when performing user-specific calls.
   *
   * @return mixed
   *   The HTTP request result data.
   */
  protected function call($command, array $data = [], $access_token = NULL) {
    $url = $this->captureAddress . "/$command";
    $headers = [];
    if (isset($access_token)) {
      $headers['Authorization'] = "OAuth $access_token";
    }
    $data = array_merge($data, [
      'client_id' => $this->clientId,
      'client_secret' => $this->clientSecret,
    ]);

    try {
      $result = $this->httpClient->post($url, [
        'headers' => $headers,
        'form_params' => $data,
      ]);
    }
    catch (GuzzleException $e) {
      $this->logger->error("Exception thrown during API call: @message", ['@message' => $e->getMessage()]);
      return FALSE;
    }

    $json_data = json_decode($result->getBody());

    if (!isset($json_data)) {
      $this->logger->error('JSON parse error for response data: @data', ['@message' => $result->getBody()]);
      return FALSE;
    }

    if ($json_data->stat == 'error') {
      $message = $json_data->error . ": " . $json_data->error_description;
      $this->logger->error("Error response received with message : @message", ['@message' => $message]);
      return FALSE;
    }

    return $json_data;
  }

  /**
   * Perform the exchange to generate and return new Access Token.
   *
   * @param string $auth_code
   *   The authorization token to use for the exchange.
   * @param string $redirect_uri
   *   The redirect_uri string to match for the exchange.
   *
   * @return string
   *   New access token.
   */
  public function getNewAccessToken($auth_code, $redirect_uri) {
    $command = "oauth/token";
    $data = [
      'code' => $auth_code,
      'redirect_uri' => $redirect_uri,
      'grant_type' => 'authorization_code',
    ];
    $json_data = $this->call($command, $data);
    if ($json_data) {
      return $json_data;
    }
    return FALSE;
  }

  /**
   * Retrieves a new access_token/refresh_token set.
   *
   * @return bool
   *   Boolean success or failure
   */
  public function getRefreshedAccessToken($refresh_token) {
    $command = "oauth/token";
    $data = [
      'refresh_token' => $refresh_token,
      'grant_type' => 'refresh_token',
    ];
    $json_data = $this->call($command, $data);
    if ($json_data) {
      return $json_data;
    }

    return FALSE;
  }

  /**
   * Retrieves the user entity from Capture.
   *
   * @param string $access_token
   *   Access token.
   *
   * @return mixed
   *   The entity retrieved or null
   */
  public function getUserEntity($access_token) {
    $user_entity = $this->call('entity', [], $access_token);
    return $user_entity;
  }

}
