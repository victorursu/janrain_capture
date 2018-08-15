<?php

namespace Drupal\janrain_capture\Exception;

/**
 * The error to throw when the data cannot be parsed as JSON.
 */
class JsonParseError extends \RuntimeException {

  /**
   * The content that was tried to be recognized as valid JSON.
   *
   * @var string
   */
  protected $content;

  /**
   * {@inheritdoc}
   */
  public function __construct(string $content) {
    parent::__construct('Error parsing JSON');

    $this->content = $content;
  }

  /**
   * {@inheritdoc}
   */
  public function getContent(): string {
    return $this->content;
  }

}
