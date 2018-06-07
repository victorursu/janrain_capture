<?php

namespace Drupal\janrain_capture\User;

/**
 * The user profile on Janrain.
 */
class JanrainUserProfile extends JanrainDataContainer {

  /**
   * {@inheritdoc}
   */
  public function __construct(\stdClass $data) {
    if (!isset($data->uuid, $data->email)) {
      throw new \InvalidArgumentException('An invalid user profile is given.');
    }

    parent::__construct($data);
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
   * {@inheritdoc}
   */
  public function getBirthDate(): ? \DateTime {
    return isset($this->data->birthday) ? new \DateTime($this->data->birthday) : NULL;
  }

  /**
   * {@inheritdoc}
   */
  public function getPhoneMobileNumber(): string {
    return $this->getPhoneNumber('mobile');
  }

  /**
   * {@inheritdoc}
   */
  public function getPhoneWorkNumber(): string {
    return $this->getPhoneNumber('work');
  }

  /**
   * {@inheritdoc}
   */
  public function getPhoneHomeNumber(): string {
    return $this->getPhoneNumber('home');
  }

  /**
   * {@inheritdoc}
   */
  public function getPhoneFaxNumber(): string {
    return $this->getPhoneNumber('fax');
  }

  /**
   * {@inheritdoc}
   */
  public function getPrimaryAddress(): JanrainUserAddress {
    return new JanrainUserAddress($this->data->primaryAddress);
  }

  /**
   * {@inheritdoc}
   */
  public function getHomeAddress(): JanrainUserAddress {
    return new JanrainUserAddress($this->data->homeAddress);
  }

  /**
   * {@inheritdoc}
   */
  public function getMailingAddress(): JanrainUserAddress {
    return new JanrainUserAddress($this->data->mailingAddress);
  }

  /**
   * Returns a phone number by the type.
   *
   * @param string $type
   *   Available types: "home", "fax", "work" and "mobile".
   *
   * @return string
   *   The phone number.
   */
  protected function getPhoneNumber($type = 'mobile'): string {
    return isset($this->data->phoneNumber->{$type}) ? (string) $this->data->phoneNumber->{$type} : '';
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
