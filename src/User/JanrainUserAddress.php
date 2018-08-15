<?php

namespace Drupal\janrain_capture\User;

/**
 * The user address.
 */
class JanrainUserAddress extends JanrainDataContainer {

  /**
   * {@inheritdoc}
   */
  protected function get($name): string {
    return isset($this->{$name}) ? (string) $this->{$name} : '';
  }

  /**
   * {@inheritdoc}
   */
  public function getStreet(): string {
    return sprintf(
      '%s %s %s %s',
      $this->get('streetName1'),
      $this->get('streetName2'),
      $this->get('streetName3'),
      $this->get('streetName4')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getAdministrativeArea(): string {
    return $this->get('administrativeArea');
  }

  /**
   * {@inheritdoc}
   */
  public function getSubAdministrativeArea(): string {
    return $this->get('subAdministrativeArea');
  }

  /**
   * {@inheritdoc}
   */
  public function getPostalCode(): string {
    return $this->get('postalCode');
  }

  /**
   * {@inheritdoc}
   */
  public function getCountry(): string {
    return $this->get('country');
  }

  /**
   * {@inheritdoc}
   */
  public function getWorkplace(): string {
    return $this->get('workplace');
  }

  /**
   * {@inheritdoc}
   */
  public function getRegion(): string {
    return $this->get('municipality');
  }

  /**
   * {@inheritdoc}
   */
  public function getDepartment(): string {
    return $this->get('department');
  }

  /**
   * {@inheritdoc}
   */
  public function getDistrict(): string {
    return $this->get('district');
  }

}
