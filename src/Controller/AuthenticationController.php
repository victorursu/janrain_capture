<?php

namespace Drupal\janrain_capture\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;
use Drupal\janrain_capture\JanrainCaptureApi;
use Drupal\janrain_capture\JanrainMarkupBuilder;
use GuzzleHttp\Psr7\Uri;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use function GuzzleHttp\Psr7\parse_query;

/**
 * Authentication controller.
 */
class AuthenticationController extends ControllerBase {

  /**
   * An instance of the "janrain_capture.capture_api" service.
   *
   * @var \Drupal\janrain_capture\JanrainCaptureApi
   */
  protected $captureApi;
  /**
   * An instance of the "janrain_capture.markup_builder" service.
   *
   * @var \Drupal\janrain_capture\JanrainMarkupBuilder
   */
  protected $markupBuilder;

  /**
   * {@inheritdoc}
   */
  public function __construct(
    JanrainCaptureApi $capture_api,
    JanrainMarkupBuilder $markup_builder
  ) {
    $this->captureApi = $capture_api;
    $this->markupBuilder = $markup_builder;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container): self {
    return new static(
      $container->get('janrain_capture.capture_api'),
      $container->get('janrain_capture.markup_builder')
    );
  }

  /**
   * Restore password form.
   */
  public function forgot() {
    return $this->markupBuilder->getScreenRenderArray('forgot');
  }

  /**
   * Logout user from the system.
   */
  public function logout() {
    user_logout();

    $output = <<<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
  <body>
    <p>You have been logged out.</p>
  </body>
</html>
EOF;

  /**
   * Cross site receiver page.
   */
  public function xdcomm() {

    $output = <<<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
    <head>
        <title>Cross-Domain Receiver Page</title>
    </head>
    <body>
        <script type="text/javascript">
            var xdcommJs = (("https:" === document.location.protocol) ? "https://d1lqe9temigv1p.cloudfront.net/js/lib/xdcomm.js" : "http://cdn.janraincapture.com/js/lib/xdcomm.js");
            document.write(decodeURIComponent("%3Cscript src='" + xdcommJs + "' type='text/javascript'%3E%3C/script%3E"));
        </script>
    </body>
</html>
EOF;

    return new Response($output);
  }

  /**
   * Login or reset a password for a user using Janrain API.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The incoming HTTP request.
   *
   * @return \Symfony\Component\HttpFoundation\Response
   *   The response.
   */
  public function login(Request $request): Response {
    // Usually, this controller should return a URI to redirect a user to.
    // This is valid for authentication. When the password reset requested
    // a user will receive an email with the link and, opening it in a
    // browser, this controller must show the real HTML page instead of
    // just a URI.
    $response_class = Response::class;
    // Read (and remove) authorization code from the request.
    $authorization_code = $this->getAuthorizationCode($request);
    // Form destination URL here since the "$request" is modified above.
    $destination_url = $this->getDestinationUrl($request);

    // No authorization code - no further logic.
    if ($authorization_code === NULL) {
      drupal_set_message($this->t('Malformed request. Authorization code is missing.'), 'error');
    }
    else {
      try {
        // The authentication can throw exceptions so their messages
        // will be exposed on the frontend.
        $this->captureApi->authenticate($authorization_code, $request->getUri());
      }
      catch (\Throwable $e) {
        if ($e->getMessage() !== '') {
          drupal_set_message($e->getMessage(), 'error');
        }
      }

      // A user has used a one-time login link.
      if ($request->get('url_type') === 'forgot') {
        // Now we're going to redirect a user to the previous location (front
        // page if missing).
        $response_class = RedirectResponse::class;

        // The authentication request ended with an error.
        if (isset($e)) {
          // Add the "code" to the URL since it's expected by the external,
          // third-party script (check for the "no code supplied" error).
          /* @example script */
          /* @link https://d29usylhdk1xyu.cloudfront.net/manifest/login?version=release%2F1.117.4_widgets_1136 */
          $destination_url->setRouteParameter('code', $authorization_code);
          // This parameter is here for recognizing the custom action that
          // potentially can be taken on the frontend.
          $destination_url->setRouteParameter('changePassword', 'no');
        }
        else {
          drupal_set_message($this->t('You have been successfully logged in via one-time login link.'));

          // The token goes to a frontend for initializing the password reset
          // session.
          // @todo Shouldn't we care about not exposing this value on a frontned?
          $destination_url->setRouteParameter('token', $this->captureApi->getAccessToken()->getToken());
          $destination_url->setRouteParameter('changePassword', 'yes');
        }
      }
    }

    return new $response_class($destination_url->setAbsolute()->toString());
  }

  /**
   * Returns the authorization code.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The current request.
   *
   * @return string|null
   *   The OAuth authorization code or NULL when the code is not presented
   *   in the request's GET query.
   */
  protected function getAuthorizationCode(Request $request): string {
    // If the request has no "code" it means it's malformed.
    if (!$request->query->has('code')) {
      return NULL;
    }

    $code = trim($request->query->get('code'));

    // The code must be read first and then removed from the request. This
    // is required for an operation, for instance, for resetting the password.
    // The link that user will get via email will look the following:
    // https://a.com/janrain_capture/oauth?url_type=forgot&code=8uy9j8quyj3tam
    // The Janrain will expect "redirect_uri" without the "code":
    // https://a.com/janrain_capture/oauth?url_type=forgot
    // If the domain will differ, OAuth will throw the "redirect_uri does not
    // match expected value" error.
    $request->query->remove('code');
    // Override global variables to ensure the "code" is no longer presented.
    $request->overrideGlobals();

    // Return ejected value.
    return $code ?: NULL;
  }

  /**
   * Returns the URL to redirect to.
   *
   * @param \Symfony\Component\HttpFoundation\Request $request
   *   The current request.
   *
   * @return \Drupal\Core\Url
   *   The URL to redirect to.
   */
  protected function getDestinationUrl(Request $request): Url {
    // See whether the request has HTTP referer.
    if ($request->server->has('HTTP_REFERER')) {
      $request_uri = new Uri($request->getUri());
      $referer_uri = new Uri($request->server->get('HTTP_REFERER'));

      // Make sure we'll not redirect out of the current origin.
      if ($referer_uri->getAuthority() === $request_uri->getAuthority()) {
        return Url::fromUserInput($referer_uri->getPath(), [
          'query' => parse_query($referer_uri->getQuery()),
        ]);
      }
    }

    // Fallback to the front page.
    return Url::fromRoute('<front>');
  }

}
