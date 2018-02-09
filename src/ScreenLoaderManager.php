<?php

namespace Drupal\janrain_capture;

use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Logger\LoggerChannelFactoryInterface;
use GuzzleHttp\Client;

/**
 * Allow to load and manage Janrain screens (local and remote).
 */
class ScreenLoaderManager {

  /**
   * Directory for cached Janrain screens.
   *
   * @var string
   */
  const CACHE_DIR = 'public://janrain_capture_screens/cache';

  /**
   * Allowed janrain screen names.
   *
   * @var string[]
   */
  const ALLOWED_SCREENS = [
    'signin',
    'edit-profile',
    'public-profile',
    'forgot',
    'verify',
  ];

  /**
   * Allowed Janrain screen types.
   */
  const ALLOWED_TYPES = ['js', 'html'];

  /**
   * Path to the screens folder.
   *
   * @var string
   */
  private $path;

  /**
   * Logger.
   *
   * @var \Psr\Log\LoggerInterface
   */
  private $logger;

  /**
   * Http client.
   *
   * @var \GuzzleHttp\Client
   */
  private $httpClient;

  /**
   * ScreenLoaderManager constructor.
   *
   * @param \Drupal\Core\Config\ConfigFactoryInterface $configFactory
   *   Config factory.
   * @param \Drupal\Core\Logger\LoggerChannelFactoryInterface $logger_factory
   *   Logger factory.
   * @param \GuzzleHttp\Client $client
   *   HTTP client.
   */
  public function __construct(ConfigFactoryInterface $configFactory, LoggerChannelFactoryInterface $logger_factory, Client $client) {
    $this->path = $configFactory->get('janrain_capture.settings')
      ->get('screens.folder');
    $this->logger = $logger_factory->get("janrain_capture");
    $this->httpClient = $client;
  }

  /**
   * Check that Janrain screens path is remote.
   *
   * @return bool
   *   TRUE when the Janrain screens path is remote.
   */
  public function isRemote() {
    return (strpos($this->path, 'http') === 0);
  }

  /**
   * Get Janrain screen contents by name and type.
   *
   * @param string $name
   *   Janrain screen name.
   * @param string $type
   *   Screen type.
   *
   * @return string
   *   Janrain screen contents.
   */
  public function getScreen($name, $type) {
    if (!(in_array($name, self::ALLOWED_SCREENS) && in_array($type, self::ALLOWED_TYPES))) {
      return '';
    }
    if (!$this->isRemote()) {
      $file_name = $this->path . $name . '.' . $type;
    }
    else {
      $file_name = sprintf('%s/%s.%s', self::CACHE_DIR, $name, $type);
    }
    if (!is_readable($file_name)) {
      $this->logger->error("Unable to read @filename", ['@filename' => $file_name]);
      return '';
    }
    return file_get_contents($file_name);
  }

  /**
   * Update Janrain screens cache if screens folder is the remote one.
   */
  public function updateRemoteScreens() {
    $cache_directory = self::CACHE_DIR;
    if (!file_prepare_directory($cache_directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
      $this->logger->error("Failed to create screen cache directory: @directory", ['@directory' => $cache_directory]);
      return;
    }
    $path_template = '%s/%s.%s';
    foreach (self::ALLOWED_SCREENS as &$name) {
      foreach (self::ALLOWED_TYPES as &$ext) {
        $screen_source = sprintf($path_template, $this->path, $name, $ext);
        $screen_destination = sprintf($path_template, $cache_directory, $name, $ext);
        $response = $this->httpClient->get($screen_source);
        if ($response->getStatusCode() != 200) {
          $this->logger->error("Error during retrieving janrain remote screen (@url) ", [
            '@url' => $screen_source,
          ]);
          continue;
        }
        $success = file_unmanaged_save_data($response->getBody(), $screen_destination, FILE_EXISTS_REPLACE);
        if ($success === FALSE) {
          $this->logger->error("Failed to write @screenDest", ['@screenDest' => $screen_destination]);
        }
      }
    }
  }

}
