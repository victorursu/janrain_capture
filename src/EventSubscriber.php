<?php

namespace Drupal\janrain_capture;

use Drupal\user\UserInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpKernel\Event\GetResponseEvent;
use Symfony\Component\HttpKernel\KernelEvents;
use Drupal\Core\Url;

/**
 * The event subscriber.
 */
class EventSubscriber implements EventSubscriberInterface {

  /**
   * {@inheritdoc}
   */
  public static function getSubscribedEvents(): array {
    $events = [];

    $events[KernelEvents::REQUEST] = ['onRequest', 1];

    return $events;
  }

  /**
   * {@inheritdoc}
   */
  public function onRequest(GetResponseEvent $event): void {
    $request = $event->getRequest();

    // - /user -> \Drupal\user\Controller\user\UserController::userPage()
    // - /user/ID -> \Drupal\Core\Entity\Controller\EntityViewController::view()
    if (
      $request->attributes->get('_route') === 'entity.user.canonical' &&
      // Redirect to "/user/ID?uuid=UUID" only in case the user has been
      // created via Janrain API.
      $request->attributes->get(RouteEnhancer::JANRAIN_ACCOUNT_PROPERTY)
    ) {
      $user = $request->attributes->get('user');

      if ($user instanceof UserInterface) {
        $uuid = $user->uuid();

        // Restrict from passing the wrong UUID manually.
        if ($request->query->get('uuid') !== $uuid) {
          // The "uuid" GET parameter must be presented in order to allow
          // Janrain loading the profile.
          $request->query->set('uuid', $uuid);
          // Override globals to get the correct value from "getUri()".
          $request->overrideGlobals();

          // ToDo: can't figure out why /user/ID and user edit pages are not working,
          // created another pages for profile view and edit as workaround
          // $event->setResponse(new RedirectResponse($request->getUri(), RedirectResponse::HTTP_MOVED_PERMANENTLY));.
          $event->setResponse(new RedirectResponse(
            Url::fromRoute('janrain_capture.view_profile', ['uuid' => $uuid])->toString(),
            RedirectResponse::HTTP_MOVED_PERMANENTLY
          ));

        }
      }
    }
  }

}
