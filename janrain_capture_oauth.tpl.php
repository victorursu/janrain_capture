<?php global $base_url; ?>
<?php global $user; ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
  <body>
    <p>Please wait...</p>
   <?php print $user->created; ?>
   <?php 
   if ((time() - $user->created) < 10) {
     echo "I'm new!";
   }
   ?>
    <script type="text/javascript">
      if (window.location.href != parent.window.location.href) {
        if (parent.window.location.href.indexOf("logout") > 1) {
          parent.window.location.href = "<?php echo $base_url; ?>";
        } else {
          parent.window.location.reload();
        }
      } else {
        window.location.href = "<?php echo $base_url; ?>";
      }
    </script>
  </body>
</html>
