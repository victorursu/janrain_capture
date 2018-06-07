<?php

namespace Drupal\janrain_capture\User;

/**
 * The container for the data.
 */
class JanrainDataContainer {

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

}
