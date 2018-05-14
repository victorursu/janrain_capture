<?php

namespace Drupal\janrain_capture\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Url;
use Drupal\janrain_capture\JanrainCaptureApiService;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;

/**
 * Oauth 2.0 controller used for user authentication.
 */
class OAuthController extends ControllerBase {

  /**
   * Janrain Capture API Service.
   *
   * @var \Drupal\janrain_capture\JanrainCaptureApiService
   */
  protected $apiService;
  /**
   * The storage of the "user" entities.
   *
   * @var \Drupal\user\UserStorageInterface
   */
  protected $userStorage;

  /**
   * {@inheritdoc}
   */
  public function __construct(
    JanrainCaptureApiService $api_service,
    EntityTypeManagerInterface $entity_type_manager
  ) {
    $this->apiService = $api_service;
    $this->userStorage = $entity_type_manager->getStorage('user');
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): self {
    return new static(
      $container->get('janrain_capture.api_service'),
      $container->get('entity_type.manager')
    );
  }

  /**
   * Login user to the system using oauth code passed in request.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The incoming HTTP request.
   *
   * @return \Symfony\Component\HttpFoundation\Response
   *   The URL to a front page or to a page in the "destination" GET parameter.
   *
   * @throws \Drupal\Core\Entity\EntityStorageException
   */
  public function login(Request $request): Response {
    $destination = $request->query->has('destination')
      ? Url::fromUserInput($request->query->get('destination'))
      : Url::fromRoute('<front>');

    $data = $this->apiService->newAccessToken(
      $request->query->get('code'),
      Url::fromRoute('janrain_capture.oauth')->setAbsolute()->toString()
    );

    $user_entity = $this->apiService->getUserEntity();
    $email = $user_entity->result->email;
    /* @var \Drupal\user\UserInterface $account */
    $account = user_load_by_mail($email);

    if ($account === FALSE) {
      $account = $this->userStorage->create([
        'name' => $user_entity->result->uuid,
        'mail' => $email,
        'status' => 1,
      ]);

      $this->userStorage->save($account);
    }

    user_login_finalize($account);
    $this->apiService->updateCaptureSession($data);

    return new Response($destination->setAbsolute()->toString());
  }

}
