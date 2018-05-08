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
   * Allowed Janrain screen names.
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
   *
   * @var string[]
   */
  const ALLOWED_TYPES = ['js', 'html'];

  /**
   * Path to the screens folder.
   *
   * @var string
   */
  protected $path;

  /**
   * Logger.
   *
   * @var \Psr\Log\LoggerInterface
   */
  protected $logger;

  /**
   * HTTP client.
   *
   * @var \GuzzleHttp\Client
   */
  protected $httpClient;

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
    $this->path = rtrim($configFactory->get('janrain_capture.settings')->get('screens.folder'), '/');
    $this->logger = $logger_factory->get('janrain_capture');
    $this->httpClient = $client;
  }

  /**
   * Check that Janrain screens path is remote.
   *
   * @return bool
   *   TRUE when the Janrain screens path is remote.
   */
  public function isRemote() {
    return strpos($this->path, 'http') === 0;
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
    if (!in_array($name, static::ALLOWED_SCREENS) || !in_array($type, static::ALLOWED_TYPES)) {
      return '';
    }

    $file_name = static::buildPath($this->isRemote() ? static::CACHE_DIR : $this->path, $name, $type);

    if (!is_readable($file_name)) {
      $this->logger->error('Unable to read @filename', [
        '@filename' => $file_name,
      ]);

      return '';
    }

    return file_get_contents($file_name);
  }

  /**
   * Update Janrain screens cache if screens folder is the remote one.
   */
  public function updateRemoteScreens() {
    $cache_directory = static::CACHE_DIR;

    if (!file_prepare_directory($cache_directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
      $this->logger->error('Failed to create screen cache directory: @directory', [
        '@directory' => $cache_directory,
      ]);

      return;
    }

    foreach (static::ALLOWED_SCREENS as $name) {
      foreach (static::ALLOWED_TYPES as $type) {
        $screen_source = static::buildPath($this->path, $name, $type);
        $screen_destination = static::buildPath($cache_directory, $name, $type);
        $response = $this->httpClient->get($screen_source);

        if ($response->getStatusCode() !== 200) {
          $this->logger->error('Error during retrieving Janrain remote screen (@url)', [
            '@url' => $screen_source,
          ]);

          continue;
        }

        if (file_unmanaged_save_data($response->getBody(), $screen_destination, FILE_EXISTS_REPLACE) === FALSE) {
          $this->logger->error('Failed to write @screenDest', [
            '@screenDest' => $screen_destination,
          ]);
        }
      }
    }
  }

  /**
   * Returns the constructed path.
   *
   * @param string $path
   *   The base path.
   * @param string $name
   *   The name of a file.
   * @param string $type
   *   The extension of a file.
   *
   * @return string
   *   The constructed path.
   */
  protected static function buildPath($path, $name, $type) {
    return sprintf('%s/%s.%s', $path, $name, $type);
  }

}
