<?php

namespace Drupal\janrain_capture\Authentication;

/**
 * The refresh token.
 */
class RefreshToken extends Token {

  /**
   * {@inheritdoc}
   */
  public function getExpiration(): ? \DateTime {
    // The refresh token is permanent therefore has no expiration.
    return NULL;
  }

}
