<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Render\Markup;
use Drupal\Core\Url;

/**
 * Builds needed markup for including Janrain functionality in HTML page.
 */
class JanrainMarkupBuilder {

  /**
   * Janrain screen loader manager.
   *
   * @var \Drupal\janrain_capture\ScreenLoaderManager
   */
  protected $screenLoaderManager;

  /**
   * Janrain Capture settings.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $janrainCaptureSettings;

  /**
   * JanrainMarkupBuilder constructor.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $config_factory
   *   Config factory.
   * @param \Drupal\janrain_capture\ScreenLoaderManager $screen_loader_manager
   *   Config factory.
   */
  public function __construct(ConfigFactoryInterface $config_factory, ScreenLoaderManager $screen_loader_manager) {
    $this->janrainCaptureSettings = $config_factory->get('janrain_capture.settings');
    $this->screenLoaderManager = $screen_loader_manager;
  }

  /**
   * Get needed for inclusion to HTML page Janrain scripts as attachment array.
   *
   * @return array
   *   Attachment array.
   */
  public function getPageAttachment(): array {
    global $base_url, $base_path;

    // Load entire configuration data array with overrides to allow
    // per-environment configuration using settings.[ENV].php.
    $settings = $this->janrainCaptureSettings->get();
    unset($settings['capture']['client_secret']);
    foreach ([
      'redirect_uri' => 'janrain_capture.oauth',
      'federate_logout_uri' => 'janrain_capture.simple_logout',
    ] as $setting => $route) {
      $settings['capture'][$setting] = Url::fromRoute($route)
        ->setAbsolute()
        ->toString();
    }
    // Federate.
    $settings['capture']['federate_xd_reciever'] = $base_url . $base_path . drupal_get_path('module', 'janrain_capture') . '/xdcomm.html';
    $settings['capture']['stylesheets'][] = file_create_url($settings['screens']['folder'] . '/stylesheets/janrain.css');
    // @todo Investigate docs for more info about federateSupportedSegments.
    if (isset($settings['capture']['federate_supported_segments'])) {
      $settings['capture']['federate_supported_segments'] = json_encode(explode(',', $settings['capture']['federate_supported_segments']));
    }
    return [
      'library' => [
        'janrain_capture/janrain_init',
      ],
      'drupalSettings' => [
        'janrain' => $settings,
        'acquia_env' => $_ENV['AH_SITE_ENVIRONMENT'] ?? 'local',
      ],
    ];
  }

  /**
   * Get Janrain screen render array.
   *
   * @param string $name
   *   Screen name.
   *
   * @return array
   *   Janrain screen render array.
   */
  public function getScreenRenderArray(string $name): array {
    $build = [];

    $build["{$name}_screen_html"] = [
      '#markup' => '',
      '#children' => $this->screenLoaderManager->getScreen($name, 'html'),
    ];

    $build["{$name}_screen_js"] = [
      '#tag' => 'script',
      '#type' => 'html_tag',
      '#value' => Markup::create($this->screenLoaderManager->getScreen($name, 'js')),
    ];

    return $build;
  }

}
