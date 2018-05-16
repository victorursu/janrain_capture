<?php

namespace Drupal\janrain_capture;

use Drupal\user\UserInterface;
use Symfony\Component\EventDispatcher\EventSubscriberInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpKernel\Event\GetResponseEvent;
use Symfony\Component\HttpKernel\KernelEvents;

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
    if ($request->attributes->get('_route') === 'entity.user.canonical' && empty($request->query->get('uuid'))) {
      $user = $request->attributes->get('user');

      if ($user instanceof UserInterface) {
        // The "uuid" GET parameter must be presented in order to allow
        // Janrain loading the profile.
        $request->query->set('uuid', $user->uuid());
        // Override globals to get the correct value from "getUri()".
        $request->overrideGlobals();

        $event->setResponse(new RedirectResponse($request->getUri(), RedirectResponse::HTTP_MOVED_PERMANENTLY));
      }
    }
  }

}
