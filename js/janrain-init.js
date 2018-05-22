window.janrain = window.janrain || {};

(function (janrain) {
  'use strict';

  /**
   * Returns the value of a property in an object.
   *
   * @param {Object} object
   *   The object to check properties at.
   * @param {Array.<String>} propertiesList
   *   The list of properties (nested, every next array item is a
   *   child of an object that stored in the previous property).
   *
   * @return {*|undefined}
   *   The nested or undefined (if no property at one of the levels) value.
   */
  function getProperty(object, propertiesList) {
    for (var i = 0; i < propertiesList.length; i++) {
      if (object.hasOwnProperty(propertiesList[i])) {
        object = object[propertiesList[i]];
      }
      else {
        return undefined;
      }
    }

    return object;
  }

  janrain.settings = janrain.settings || {};
  janrain.settings.capture = janrain.settings.capture || {};

  janrain.settings.appUrl = drupalSettings.janrain.app_url;
  janrain.settings.language = drupalSettings.path.currentLanguage;
  janrain.settings.tokenUrl = drupalSettings.janrain.capture.capture_server;
  /** @link https://docs.janrain.com/social/login-javascript-api/#providers */
  janrain.settings.providers = drupalSettings.janrain.capture.providers;

  // --- Capture Widget Settings ---------------------------------------------
  janrain.settings.capture.appId = drupalSettings.janrain.capture.app_id;
  janrain.settings.capture.clientId = drupalSettings.janrain.capture.client_id;
  janrain.settings.capture.captureServer = drupalSettings.janrain.capture.capture_server;
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

  // These settings are used by screens.
  // @todo Consider modifying screens to use global "janrain" object.
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

  event.initEvent('janrainCaptureReady', true, true);

  script.id = 'janrainAuthWidget';
  script.src = document.location.protocol + '//' + drupalSettings.janrain.capture.load_js_url;

  if ('addEventListener' in document) {
    document.addEventListener('DOMContentLoaded', onload, false);
  }
  else {
    window.attachEvent('onload', onload);
  }

  var interval = setInterval(function() {
    if (
      getProperty(janrain, ['engage', 'signin', 'status']) === 'loaded' &&
      typeof getProperty(janrain, ['capture', 'ui']) === 'object'
    ) {
      clearInterval(interval);
      window.janrainCaptureReady = true;
      document.dispatchEvent(event);
    }
  }, 200);

  document.head.appendChild(script);
})(window.janrain);
