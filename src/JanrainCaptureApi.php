<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\KeyValueStore\KeyValueDatabaseFactory;
use Drupal\Core\Logger\LoggerChannelFactoryInterface;
use Drupal\Core\Session\AccountProxyInterface;
use Drupal\Core\Url;
use Drupal\janrain_capture\Authentication\AccessToken;
use Drupal\janrain_capture\Authentication\RefreshToken;
use Drupal\janrain_capture\Exception\JanrainApiCallError;
use Drupal\janrain_capture\Exception\JanrainUnauthorizedError;
use Drupal\janrain_capture\Exception\JsonParseError;
use Drupal\janrain_capture\User\JanrainUserProfile;
use Drupal\user\UserDataInterface;
use Drupal\user\UserInterface;
use GuzzleHttp\Client;
use GuzzleHttp\Exception\RequestException;
use GuzzleHttp\Exception\TransferException;

/**
 * The integration between Janrain and Drupal.
 */
class JanrainCaptureApi implements JanrainCaptureApiInterface {

  /**
   * An instance of the "http_client" service.
   *
   * @var \GuzzleHttp\Client
   */
  protected $httpClient;
  /**
   * The storage of the "user" entities.
   *
   * @var \Drupal\user\UserStorageInterface
   */
  protected $userStorage;
  /**
   * An instance of the "current_user" service.
   *
   * @var \Drupal\Core\Session\AccountProxyInterface
   */
  protected $currentUser;
  /**
   * An instance of the "user.data" service.
   *
   * @var \Drupal\user\UserDataInterface
   */
  protected $userData;
  /**
   * An instance of the "module_handler" service.
   *
   * @var \Drupal\Core\Extension\ModuleHandlerInterface
   */
  protected $moduleHandler;
  /**
   * The logger channel.
   *
   * @var \Psr\Log\LoggerInterface
   */
  protected $logger;
  /**
   * The database storage.
   *
   * @var \Drupal\Core\KeyValueStore\DatabaseStorage
   */
  protected $dbStorage;
  /**
   * The client ID of Janrain.
   *
   * @var string
   */
  protected $clientId;
  /**
   * The client secret of Janrain.
   *
   * @var string
   */
  protected $clientSecret;
  /**
   * The address for captures of Janrain.
   *
   * @var string
   */
  protected $captureAddress;

  /**
   * {@inheritdoc}
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  public function __construct(
    Client $http_client,
    AccountProxyInterface $current_user,
    UserDataInterface $user_data,
    ConfigFactoryInterface $config_factory,
    ModuleHandlerInterface $module_handler,
    KeyValueDatabaseFactory $database_factory,
    EntityTypeManagerInterface $entity_type_manager,
    LoggerChannelFactoryInterface $logger_factory
  ) {
    $config = $config_factory
      ->get('janrain_capture.settings')
      ->get('capture');

    $this->clientId = $config['client_id'] ?? '';
    $this->clientSecret = $config['client_secret'] ?? '';
    $this->captureAddress = $config['capture_server'] ?? '';
    $this->mesageCountryRestricted = $config['validate']['mesage_country_restricted'] ?? '';

    $this->logger = $logger_factory->get('janrain_capture');
    $this->userData = $user_data;
    $this->dbStorage = $database_factory->get('janrain_capture');
    $this->httpClient = $http_client;
    $this->userStorage = $entity_type_manager->getStorage('user');
    $this->currentUser = $current_user;
    $this->moduleHandler = $module_handler;
  }

  /**
   * {@inheritdoc}
   */
  public function authenticate(string $auth_code, string $redirect_uri): ?Url {
    $token = $this->getToken(static::GRANT_TYPE_AUTHORIZATION_CODE, [
      'code' => $auth_code,
      'redirect_uri' => $redirect_uri,
    ]);

    // Ideally, this method must not throw any exceptions here since
    // we're using it with a newly requested access token. If it's
    // untrue a user does not exist in Janrain.
    $profile = $this->getEntity($token);
    $email = $profile->getEmail();
    // The UUID in Drupal and on Janrain should be the same.
    $uuid = $profile->getUuid();
    // Check whether our application already knows a user.
    $accounts = $this->userStorage
      ->getQuery('OR')
      ->condition('uuid', $uuid)
      ->condition('mail', $email)
      ->execute();

    // This part will never be reached if a user doesn't exist on Janrain.
    if (empty($accounts)) {
      $is_new = TRUE;
      $account = $this->userStorage->create([
        'uuid' => $uuid,
        // The username must be unique as well as email and UUID.
        'name' => $email,
        'mail' => $email,
        'status' => TRUE,
      ]);

      $this->userStorage->save($account);
    }
    else {
      $is_new = FALSE;
      /* @var \Drupal\user\UserInterface $account */
      $account = $this->userStorage->load(reset($accounts));
    }

    $redirect = user_login_finalize($account);

    // Update the current user account in memory. This needed to provide
    // a correct user account for calls to "getAccessToken()" method in
    // the same request.
    $this->currentUser = $account;
    // Ensure the user is marked as having a Janrain account.
    $this->userData->set('janrain_capture', $account->id(), 'janrain_username', $profile->getUsername());
    // Inform subscribers about the successful authentication.
    /* @see hook_janrain_capture_user_authenticated() */
    $this->moduleHandler->invokeAll('janrain_capture_user_authenticated', [
      $profile,
      $account,
      $is_new,
    ]);

    // Save the token to the database.
    $this->cache($token);

    return $redirect;
  }

  /**
   * {@inheritdoc}
   */
  public function getAccessToken(bool $force_refresh = FALSE): AccessToken {
    /* @var \Drupal\janrain_capture\Authentication\AccessToken $access_token */
    // Read the token stored after successful authentication.
    $access_token = $this->cache();

    // No token in the database - the user is not authenticated and probably
    // has never been (could be the case the database entry was truncated).
    if ($access_token === NULL) {
      throw new JanrainUnauthorizedError('The user has never been authenticated.', -1);
    }

    // Forcible refresh wasn't requested and expiration date hasn't passed.
    if (!$force_refresh && !$access_token->isExpired()) {
      return $access_token;
    }

    // Prolong the access token and update it in the database.
    return $this->cache($this->getToken(static::GRANT_TYPE_REFRESH_TOKEN, [
      'refresh_token' => $access_token->getRefreshToken()->getToken(),
    ]));
  }

  /**
   * {@inheritdoc}
   */
  public function getUserProfile(): JanrainUserProfile {
    try {
      // Use an existing access token.
      return $this->getEntity($this->getAccessToken());
    }
    // Allow exactly one fail since an access token might be expired.
    catch (JanrainUnauthorizedError $e) {
      // Try to load an entity once again using prolonged access token.
      return $this->getEntity($this->getAccessToken(TRUE));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function isJanrainAccount(UserInterface $account): bool {
    return (bool) $this->userData->get('janrain_capture', $account->id(), 'janrain_username');
  }

  /**
   * Returns existing, newly added or updated access token from the database.
   *
   * @param \Drupal\janrain_capture\Authentication\AccessToken|null $access_token
   *   The access token. Can be omitted to read an existing one.
   *
   * @return \Drupal\janrain_capture\Authentication\AccessToken|null
   *   The access token.
   */
  protected function cache(AccessToken $access_token = NULL): ? AccessToken {
    $user_id = $this->currentUser->id();

    if ($user_id < 1) {
      throw new \LogicException('Cannot read/store an access token for an unauthenticated user.');
    }

    $cache_key = "$user_id:access_token";

    if ($access_token === NULL) {
      return $this->dbStorage->get($cache_key);
    }

    $this->dbStorage->set($cache_key, $access_token);

    return $access_token;
  }

  /**
   * Returns requested access token.
   *
   * @param string $grant_type
   *   One of the valid grant types. Use "GRANT_TYPE_" class constants.
   * @param string[] $parameters
   *   The list of additional parameters for the request.
   *
   * @return \Drupal\janrain_capture\Authentication\AccessToken
   *   The obtained access token.
   *
   * @throws \InvalidArgumentException
   * @throws \GuzzleHttp\Exception\TransferException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   *
   * @link https://docs.janrain.com/api/registration/authentication/#oauth-token
   */
  protected function getToken(string $grant_type, array $parameters): AccessToken {
    if (!in_array($grant_type, [static::GRANT_TYPE_AUTHORIZATION_CODE, static::GRANT_TYPE_REFRESH_TOKEN], TRUE)) {
      throw new \InvalidArgumentException(sprintf('The "$grant_type" argument is invalid for the "%s" method.', __METHOD__));
    }

    // Define the grant type.
    $parameters['grant_type'] = $grant_type;
    // Request a token.
    $data = $this->call('oauth/token', $parameters);

    return new AccessToken($data->access_token, $data->expires_in, new RefreshToken($data->refresh_token));
  }

  /**
   * Returns the user's profile data.
   *
   * @param \Drupal\janrain_capture\Authentication\AccessToken $access_token
   *   The access token.
   *
   * @return \Drupal\janrain_capture\User\JanrainUserProfile
   *   The user's profile data.
   *
   * @throws \GuzzleHttp\Exception\TransferException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   * @throws \Drupal\janrain_capture\Exception\JanrainUnauthorizedError
   *
   * @link https://docs.janrain.com/api/registration/entity/#entity
   * @link https://docs.janrain.com/api/registration/error-codes
   */
  protected function getEntity(AccessToken $access_token): JanrainUserProfile {
    $entity = $this->call('entity', [], $access_token->getToken());

    if (isset($entity->code)) {
      throw new JanrainUnauthorizedError($entity->error_description, $entity->code);
    }

    return new JanrainUserProfile($entity->result);
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
   * @return \stdClass
   *   The HTTP request result data.
   *
   * @throws \GuzzleHttp\Exception\TransferException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   */
  protected function call($command, array $data = [], string $access_token = NULL): \stdClass {
    $headers = [];

    if ($access_token !== NULL) {
      $headers['Authorization'] = "OAuth $access_token";
    }

    $data['client_id'] = $this->clientId;
    $data['client_secret'] = $this->clientSecret;

    try {
      $result = $this->httpClient->post($this->captureAddress . '/' . $command, [
        'headers' => $headers,
        'form_params' => $data,
      ]);
    }
    catch (RequestException $e) {
      $result = $e->getResponse();
    }
    catch (TransferException $e) {
      $this->logger->error('The exception is thrown during API call: @message', [
        '@message' => $e->getMessage(),
      ]);

      throw $e;
    }

    $body = (string) $result->getBody();
    $json = json_decode($body);

    if ($json === NULL) {
      $this->logger->error('JSON parse error for response data: @data', [
        '@data' => $body,
      ]);

      throw new JsonParseError($body);
    }

    if ($json->stat === 'error') {
      $this->logger->error('Error response received: @response', [
        '@response' => var_export($json, TRUE),
      ]);

      throw new JanrainApiCallError($json->error_description, $json->code, $json);
    }

    return $json;
  }

}
