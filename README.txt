INTRODUCTION
------------

Authors:
* Bryce Hamrick (bhamrick)

This plugin implements full Janrain Capture integration with Drupal. More
information can be found at:
http://www.janrain.com/products/capture

Documentation for Janrain Capture can be found at:
http://www.janraincapture.com/docs



INSTALLATION
------------

1) The janrain_capture_ui module in this package relies on the jQuery Update
module to be installed and enabled to update jQuery to 1.5.2 and for the
Fancybox jQuery library version 1.3.4 to be installed at:
sites/all/libraries/fancybox/jquery.fancybox-1.3.4.pack.js

Also for the janrain_capture_ui module you'll need to get a copy of json2.js
from http://json.org/ and install it to:
sites/all/libraries/json2/json2.js

You also have the option of building out your own UI with your preferred iframe
tools as well.

2) Upload the janrain_capture directory to your site-specific modules directory
or to sites/all/modules

3) Enable the module(s) via the Modules screen in the admin backend

4) Visit the Janrain Capture section of your Site configuration and set your
Janrain Capture client information.

5) Include a link somewhere in your page templates to authenticate. If using
the janrain_capture_ui module you can use the PHP function
janrain_capture_url()  to construct the href and should have the classes
'fancy' and 'iframe' in order for the fancybox initialization to bind
correctly.
Example:
<a href="<?php echo janrain_capture_url(); ?>" class="iframe fancy">Log in</a>

There is also a Janrain Capture block with Login / Logout and Edit Profile
links but is generally meant for testing and may not be suitable for production
use.


HOOKS
-----

hook_janrain_capture_user_authenticated($capture_profile, $account, $newUser)

Description:
This hook is executed immediately after the user authentication method is
processed but before the page is rendered.

Args:

* $capture_profile
  The profile data returned from the users Capture record

* $account
  The local account object of the user being authenticated

* $new_user
  Returns true if this is a newly created account or false if authenticating an
  existing account.

Example:

function mymodule_janrain_capture_user_authenticated($capture_profile,
  $account,
  $new_user) {
  if ($new_user) {
    $params['account'] = $account;

    // Execute the welcome_message_mail function with key being either 'male',
    // 'female', or null
    drupal_mail('welcome_message',
      $capture_profile['gender'],
      $account->mail,
      user_preferred_language($account),
      $params);
  }
}


hook_janrain_capture_user_profile_updated($capture_profile, $account, $origin)

Description:
This hook is executed immediately after the user profile is updated in Capture
and synchronized with the local Drupal user. By default the completion of this
process will redirec the user to $origin. If you wish to prevent this behavior
you can do so by returning false from an implementation of this hook.

Args:

* $capture_profile
  The updated profile data returned from the users Capture record

* $account
  The local account object of the user being authenticated

* $origin
  The URL of the page on which the profile screen was launched.

Example:

function mymodule_janrain_capture_user_profile_updated($capture_profile,
  $account,
  $origin) {
  drupal_set_message("Profile Updated!", "status");
  drupal_goto();
  return false;
}


hook_janrain_capture_fields_array($capture_profile)

Description:
This hook is executed during the construction of the user fields array. By
default this module will sync the 'email' profile field with Drupal's 'mail'
column and whichever field is specified in the Janrain Capture settings to use
for the 'name' column. Use this hook to return an array of key => value pairs
to sync.

Args:

* $capture_profile
  The profile data returned from the users Capture record

Example:

// Sync givenName and familyName to custom fields created
// using the Drupal Profile module
function mymodule_janrain_capture_fields_array($capture_profile) {
  return array(
    'profile_first_name' => $capture_profile['givenName'],
    'profile_last_name' => $capture_profile['familyName']
  );
}


MESSAGE HOOKS
-------------

The following hooks are in place to allow customization of error presented by
the drupal_set_message function.

hook_janrain_capture_user_already_mapped
hook_janrain_capture_mapping_failed
hook_janrain_capture_user_exists
hook_janrain_capture_failed_create
hook_janrain_capture_email_unverified - passes in the URL for a resend link
hook_janrain_capture_no_oauth
hook_janrain_capture_verification_resent - redirects


When implementing any of these hooks you should return true if you would like
the original message to be displayed, or false if you do not.


Additional features

hook_janrain_capture_email_unverified will pass in 1 parameter to an
implementation of this hook. This will be a string of the URL to include in a
link for the user to have the verification email resent.

If you return false on hook_janrain_capture_verification_resent you will bypass
a redirect to the home page of the Drupal installation after calling the
drupal_set_message method. If you are returning false you will need to initiate
a redirect using the drupal_goto() method to a page of your choosing.
