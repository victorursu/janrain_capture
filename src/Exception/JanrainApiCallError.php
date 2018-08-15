<?php

namespace Drupal\janrain_capture\Exception;

/**
 * The error to throw when an API call to Janrain ended unexpectedly.
 */
class JanrainApiCallError extends \RuntimeException {

  /**
   * The response from Janrain.
   *
   * @var \stdClass
   */
  protected $response;

  /**
   * {@inheritdoc}
   */
  public function __construct($message, int $code, \stdClass $response) {
    parent::__construct($message);

    $this->response = $response;
  }

  /**
   * {@inheritdoc}
   */
  public function getResponse(): \stdClass {
    return $this->response;
  }

}
