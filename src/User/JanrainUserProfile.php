<?php

namespace Drupal\janrain_capture\User;

/**
 * The user profile on Janrain.
 */
class JanrainUserProfile {

  /**
   * The profile's data.
   *
   * @var \stdClass
   */
  protected $data;

  /**
   * {@inheritdoc}
   */
  public function __construct(\stdClass $data) {
    $this->data = $data;

    if (!isset($this->uuid, $this->email)) {
      throw new \InvalidArgumentException('An invalid user profile is given.');
    }
  }

  /**
   * {@inheritdoc}
   */
  public function __isset($name): bool {
    return property_exists($this->data, $name);
  }

  /**
   * {@inheritdoc}
   */
  public function __get($name) {
    return $this->data->{$name};
  }

  /**
   * {@inheritdoc}
   */
  public function __unset($name): void {
    unset($this->data->{$name});
  }

  /**
   * {@inheritdoc}
   */
  public function getEmail(): string {
    return $this->data->email;
  }

  /**
   * {@inheritdoc}
   */
  public function getUuid(): string {
    return $this->data->uuid;
  }

  /**
   * Returns the name of a user.
   *
   * @return string
   *   Returns the "Alfred Hitchcock" if first and last name specified,
   *   the "Alfred" - first name specified, "Hitchcock" - last name
   *   specified, "alfred.hitchcock@example.com" if no names exist.
   */
  public function getUsername(): string {
    $first_name = $this->getFirstName();
    $last_name = $this->getLastName();

    // The first and last name specified. Split them by a single space.
    if ($first_name !== '' && $last_name !== '') {
      $first_name .= ' ';
    }

    // Use an email as a username if neither first nor last name specified.
    return $first_name . $last_name ?: $this->getEmail();
  }

  /**
   * {@inheritdoc}
   */
  public function getFirstName(): string {
    return $this->givenName ?? '';
  }

  /**
   * {@inheritdoc}
   */
  public function getLastName(): string {
    return $this->familyName ?? '';
  }

}
