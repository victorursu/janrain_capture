<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Routing\Enhancer\RouteEnhancerInterface;
use Drupal\user\UserInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\Routing\Route;

/**
 * The route enhancer.
 */
class RouteEnhancer implements RouteEnhancerInterface {

  /**
   * The property to set to route's defaults and request's attributes.
   */
  public const JANRAIN_ACCOUNT_PROPERTY = '_janrain_account';

  /**
   * An instance of the "janrain_capture.capture_api" service.
   *
   * @var \Drupal\janrain_capture\JanrainCaptureApi
   */
  protected $captureApi;

  /**
   * {@inheritdoc}
   */
  public function __construct(JanrainCaptureApiInterface $capture_api) {
    $this->captureApi = $capture_api;
  }

  /**
   * {@inheritdoc}
   */
  public function enhance(array $defaults, Request $request) {
    if ($defaults['user'] instanceof UserInterface) {
      $janrain_account = $this->captureApi->isJanrainAccount($defaults['user']);

      // The "self" is used since the name of the property
      // is closed for modifications.
      $defaults[self::JANRAIN_ACCOUNT_PROPERTY] = $janrain_account;
      $request->attributes->set(self::JANRAIN_ACCOUNT_PROPERTY, $janrain_account);
    }

    return $defaults;
  }

  /**
   * {@inheritdoc}
   */
  public function applies(Route $route) {
    return $route->getOption('parameters')['user'] ?? FALSE;
  }

}
