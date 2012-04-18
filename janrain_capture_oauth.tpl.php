<?php

/**
 * @file
 * Template to print upon completion of the Capture OAuth flow.
 */

global $base_url;
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
  <body>
    <p>Please wait...</p>
    <script type="text/javascript">
      if (window.location.href != window.parent.location.href) {
        if (window.parent.location.href.indexOf("logout") > 1) {
          window.parent.location.href = "<?php echo $base_url; ?>";
        } else {
          window.parent.location.reload();
        }
      } else {
        window.location.href = "<?php echo $base_url; ?>";
      }
    </script>
  </body>
</html>
