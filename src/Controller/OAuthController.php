<?php

namespace Drupal\janrain_capture\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;
use Drupal\janrain_capture\JanrainCaptureApiService;
use Drupal\user\Entity\User;
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
  private $apiService;

  /**
   * OAuthController constructor.
   *
   * @param \Drupal\janrain_capture\JanrainCaptureApiService $api_service
   *   Janrain Capture API Service.
   */
  public function __construct(JanrainCaptureApiService $api_service) {
    $this->apiService = $api_service;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static($container->get('janrain_capture.api_service'));
  }

  /**
   * Login user to the system using oauth code passed in request.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   Incoming HTTP request.
   *
   * @return \Symfony\Component\HttpFoundation\Response
   *   Url to frontpage.
   */
  public function login(Request $request) {
    $oauth_url = Url::fromRoute('janrain_capture.oauth', [], ['absolute' => TRUE])
      ->toString();
    $frontpage_url = Url::fromRoute('<front>', [], ['absolute' => TRUE])
      ->toString();
    $code = $request->query->get('code');
    $this->apiService->newAccessToken($code, $oauth_url);
    $user_entity = $this->apiService->getUserEntity();
    $email = $user_entity->result->email;
    $account = user_load_by_mail($email);
    if (!$account) {
      $account = User::create([
        'name' => $user_entity->result->uuid,
        'mail' => $email,
        'status' => 1,
      ]);
      $account->save();
    }
    user_login_finalize($account);

    return new Response($frontpage_url);
  }

}
