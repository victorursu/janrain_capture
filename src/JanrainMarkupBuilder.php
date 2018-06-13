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
  private $screenLoaderManager;

  /**
   * Janrain Capture settings.
   *
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  private $janrainCaptureSettings;

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
  public function getPageAttachment() {
    global $base_url;

    $settings = $this->janrainCaptureSettings->getRawData();
    unset($settings['capture']['client_secret']);
    $settings['capture']['redirect_uri'] = Url::fromRoute('janrain_capture.oauth')
      ->setAbsolute()
      ->toString();
    $settings['capture']['stylesheets'][] = file_create_url($settings['screens']['folder'] . '/stylesheets/janrain.css');

    // Federate.
    $settings['capture']['federate_xd_reciever'] = $base_url . base_path() . drupal_get_path('module', 'janrain_capture') . '/xdcomm.html';
    $settings['capture']['federate_logout_uri'] = Url::fromRoute('janrain_capture.simple_logout', ['absolute' => TRUE]);

    // Just one-to-one port.
    // @todo Investigate docs for more info about federateSupportedSegments.
    if (isset($settings['capture']['federate_supported_segments'])) {
      $segment_names = explode(',', $settings['capture']['federate_supported_segments']);

      if ($segment_names) {
        $settings['capture']['federate_supported_segments'] = json_encode($segment_names);
      }
    }

    $attachments['drupalSettings'] = $settings;
    $attachments['library'] = 'janrain_capture/janrain_init';
    return $attachments;
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
  public function getScreenRenderArray($name) {
    $screen_html = $this->screenLoaderManager->getScreen($name, 'html');
    $screen_js = $this->screenLoaderManager->getScreen($name, 'js');

    $build["{$name}_screen_html"] = [
      '#markup' => '',
      '#children' => $screen_html,
    ];
    $build["{$name}_screen_js"] = [
      '#type' => 'html_tag',
      '#tag' => 'script',
      '#value' => Markup::create($screen_js),
    ];

    return $build;
  }

}
