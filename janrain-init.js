window.janrain = window.janrain || {};

(function (janrain) {
  'use strict';

  janrain.settings = {};
  janrain.settings.capture = {};

  janrain.settings.appUrl = drupalSettings.janrain.app_url;
  janrain.settings.capture.captureServer = drupalSettings.janrain.capture.capture_server;
  janrain.settings.capture.appId = drupalSettings.janrain.capture.app_id;
  janrain.settings.capture.clientId = drupalSettings.janrain.capture.client_id;

  // --- Engage Widget Settings ----------------------------------------------
  janrain.settings.language = drupalSettings.path.currentLanguage;
  janrain.settings.tokenUrl = janrain.settings.capture.captureServer;

  // --- Capture Widget Settings ---------------------------------------------
  janrain.settings.capture.language = drupalSettings.path.currentLanguage;
  janrain.settings.capture.redirectUri = drupalSettings.janrain.capture.redirect_uri;
  janrain.settings.capture.setProfileCookie = true;
  janrain.settings.capture.keepProfileCookieAfterLogout = true;
  janrain.settings.capture.responseType = 'code';
  janrain.settings.capture.stylesheets = drupalSettings.janrain.capture.stylesheets;

  // --- Federate ------------------------------------------------------------
  if (drupalSettings.janrain.capture.enable_sso) {
    janrain.settings.capture.federate = true;
    janrain.settings.capture.federateServer = drupalSettings.janrain.capture.federate_server;
    janrain.settings.capture.federateXdReceiver = drupalSettings.janrain.capture.federate_xd_reciever;
    janrain.settings.capture.federateLogoutUri = drupalSettings.janrain.capture.federate_logout_uri;
    janrain.settings.capture.federateEnableSafari = true;
  }

  Drupal.settings = {};
  Drupal.settings.basePath = drupalSettings.path.baseUrl;
  Drupal.settings.pathPrefix = drupalSettings.path.pathPrefix;
  Drupal.settings.janrainCapture = {};
  Drupal.settings.janrainCapture.sso_address = drupalSettings.janrain.capture.federate_server;

  var script = document.createElement('script');
  var event = document.createEvent('Event');
  var onload = function () {
    janrain.ready = true;
  };

  event.initEvent('janrainCaptureLoaded', true, true);

  script.id = 'janrainAuthWidget';
  script.src = document.location.protocol + '//' + drupalSettings.janrain.capture.load_js_url;

  if ('addEventListener' in document) {
    document.addEventListener('DOMContentLoaded', onload, false);
  }
  else {
    window.attachEvent('onload', onload);
  }

  var interval = setInterval(function() {
    if (janrain.hasOwnProperty('capture') && typeof janrain.capture.ui === 'object') {
      clearInterval(interval);
      document.dispatchEvent(event);
    }
  }, 500);

  document.head.appendChild(script);
})(window.janrain);
