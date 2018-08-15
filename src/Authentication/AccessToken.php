<?php

namespace Drupal\janrain_capture\Authentication;

/**
 * The access token.
 */
class AccessToken extends Token {

  /**
   * The refresh token.
   *
   * @var \Drupal\janrain_capture\Authentication\RefreshToken
   */
  protected $refreshToken;

  /**
   * {@inheritdoc}
   */
  public function __construct(string $token, int $expiration, RefreshToken $refresh_token) {
    parent::__construct($token);

    $this->setExpiration($expiration);
    $this->refreshToken = $refresh_token;
  }

  /**
   * {@inheritdoc}
   */
  public function getRefreshToken(): RefreshToken {
    return $this->refreshToken;
  }

}
