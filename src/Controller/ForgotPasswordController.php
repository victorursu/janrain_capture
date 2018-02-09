<?php

namespace Drupal\janrain_capture\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\janrain_capture\JanrainMarkupBuilder;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Janrain restore password controller.
 */
class ForgotPasswordController extends ControllerBase {

  /**
   * Janrain markup builder.
   *
   * @var \Drupal\janrain_capture\JanrainMarkupBuilder
   */
  private $markupBuilder;

  /**
   * OAuthController constructor.
   *
   * @param \Drupal\janrain_capture\JanrainMarkupBuilder $markup_builder
   *   Janrain Capture API Service.
   */
  public function __construct(JanrainMarkupBuilder $markup_builder) {
    $this->markupBuilder = $markup_builder;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static($container->get('janrain_capture.markup_builder'));
  }

  /**
   * Restore password form.
   */
  public function restorePassword() {
    $page = $this->markupBuilder->getScreenRenderArray("forgot");
    return $page;
  }

}
