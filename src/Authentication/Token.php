<?php

namespace Drupal\janrain_capture\Authentication;

/**
 * The token.
 */
class Token {

  /**
   * The token itself.
   *
   * @var string
   */
  protected $token;
  /**
   * The expiration of a token.
   *
   * @var \DateTime|null
   */
  protected $expiration;
  /**
   * The token's life in seconds.
   *
   * @var int
   */
  protected $expiresIn = 0;

  /**
   * {@inheritdoc}
   */
  public function __construct(string $token) {
    $this->token = $token;
  }

  /**
   * {@inheritdoc}
   */
  public function __toString(): string {
    return $this->getToken();
  }

  /**
   * {@inheritdoc}
   */
  public function getToken(): string {
    return $this->token;
  }

  /**
   * {@inheritdoc}
   */
  public function setExpiration(int $expiration): void {
    $this->expiresIn = $expiration;
    $this->expiration = new \DateTime();
    $this->expiration->setTimestamp(REQUEST_TIME + $expiration);
  }

  /**
   * {@inheritdoc}
   */
  public function getExpiresIn(): int {
    return $this->expiresIn;
  }

  /**
   * {@inheritdoc}
   */
  public function getExpiration(): ? \DateTime {
    return $this->expiration;
  }

  /**
   * {@inheritdoc}
   */
  public function isExpired(): bool {
    $expiration = $this->getExpiration();

    // No expiration - token is permanent.
    if ($expiration === NULL) {
      return FALSE;
    }

    return $expiration->getTimestamp() < REQUEST_TIME;
  }

}
