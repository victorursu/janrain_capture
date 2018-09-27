<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Url;
use Drupal\janrain_capture\User\JanrainUserProfile;
use Drupal\user\UserInterface;
use Drupal\janrain_capture\Authentication\AccessToken;

/**
 * The interface for implementing Janrain Capture authentication.
 */
interface JanrainCaptureApiInterface {

  /**
   * The grant type for refreshing the OAuth token.
   */
  public const GRANT_TYPE_REFRESH_TOKEN = 'refresh_token';
  /**
   * The grant type for requesting the OAuth token.
   */
  public const GRANT_TYPE_AUTHORIZATION_CODE = 'authorization_code';

  /**
   * Returns requested access token and set it to the current session.
   *
   * @param string $auth_code
   *   The code of authentication.
   * @param string $redirect_uri
   *   The URI to redirect to after successful call.
   *
   * @return \Drupal\user\UserInterface
   *   The Drupal account of authenticated user.
   *
   * @throws \GuzzleHttp\Exception\GuzzleException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  public function authenticate(string $auth_code, string $redirect_uri): ?Url;

  /**
   * Returns an access token from the database and prolongs it automatically.
   *
   * IMPORTANT: this method requires a user to be authenticated in Drupal.
   *
   * @param bool $force_refresh
   *   An indicator to forcibly refresh an access token.
   *
   * @return \Drupal\janrain_capture\Authentication\AccessToken
   *   The access token.
   *
   * @throws \InvalidArgumentException
   * @throws \GuzzleHttp\Exception\GuzzleException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   * @throws \Drupal\janrain_capture\Exception\JanrainUnauthorizedError
   */
  public function getAccessToken(bool $force_refresh = FALSE): AccessToken;

  /**
   * Returns the user's profile data.
   *
   * @return \Drupal\janrain_capture\User\JanrainUserProfile
   *   The user's profile data.
   *
   * @throws \InvalidArgumentException
   * @throws \GuzzleHttp\Exception\GuzzleException
   * @throws \Drupal\janrain_capture\Exception\JsonParseError
   * @throws \Drupal\janrain_capture\Exception\JanrainApiCallError
   * @throws \Drupal\janrain_capture\Exception\JanrainUnauthorizedError
   */
  public function getUserProfile(): JanrainUserProfile;

  /**
   * Returns a state whether a user has a Janrain account.
   *
   * @param \Drupal\user\UserInterface $account
   *   The Drupal user.
   *
   * @return bool
   *   The state.
   */
  public function isJanrainAccount(UserInterface $account): bool;

}
