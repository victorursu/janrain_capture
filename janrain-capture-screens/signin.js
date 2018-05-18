function janrainCaptureWidgetOnLoad() {
  jQuery('a[href="/user/logout"]').addClass('capture_end_session');

  function handleCaptureLogin(result) {
    jQuery.ajax({
      url: janrain.settings.capture.redirectUri + '?code=' + result.authorizationCode,
      async: false,
      success: function(redirect_uri) {
        window.location.replace(redirect_uri);
      }
    });
  }

  janrain.events.onCaptureSessionNotFound.addHandler(function() {
    if (window.hasOwnProperty('Backplane') && Backplane.getChannelID && Backplane.getChannelID()) {
      Backplane.resetCookieChannel();
    }
  });

  janrain.events.onCaptureRegistrationSuccess.addHandler(handleCaptureLogin);
  janrain.events.onCaptureLoginSuccess.addHandler(handleCaptureLogin);
  janrain.events.onCaptureSessionEnded.addHandler(function() {
    window.location.replace('/user/logout');
  });

  janrain.capture.ui.start();
}
