<?php

/**
 * @file
 * Janrain Capture Drupal API.
 */

/**
 * React to the authentication via Janrain.
 *
 * @param \Drupal\janrain_capture\User\JanrainUserProfile $profile
 *   The profile of a user on Janrain.
 * @param \Drupal\user\UserInterface $user
 *   The user that has been authenticated.
 * @param bool $is_new
 *   The state whether an authenticated user is new in Drupal.
 */
function hook_janrain_capture_user_authenticated(\Drupal\janrain_capture\User\JanrainUserProfile $profile, \Drupal\user\UserInterface $user, bool $is_new): void {
  // The user is successfully signed in to Drupal using Janrain API.
}
