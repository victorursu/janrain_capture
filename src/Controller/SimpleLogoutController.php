<?php

namespace Drupal\janrain_capture\Controller;

use Symfony\Component\HttpFoundation\Response;

/**
 * Janrain logout controller.
 */
class SimpleLogoutController {

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

    return new Response($output);
  }

}
