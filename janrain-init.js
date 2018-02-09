if (typeof window.janrain !== 'object') window.janrain = {};
Drupal.settings = {};
Drupal.settings.basePath = drupalSettings.path.baseUrl;
Drupal.settings.pathPrefix = drupalSettings.path.pathPrefix;
Drupal.settings.janrainCapture = {};
Drupal.settings.janrainCapture.sso_address = drupalSettings.janrain.capture.federate_server;

window.janrain.settings = {};
window.janrain.settings.capture = {};

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

function isReady() {
  janrain.ready = true;
};
if (document.addEventListener) {
  document.addEventListener("DOMContentLoaded", isReady, false);
} else {
  window.attachEvent('onload', isReady);
}

var e = document.createElement('script');
e.type = 'text/javascript';
e.id = 'janrainAuthWidget';
var url = document.location.protocol === 'https:' ? 'https://' : 'http://';
url += drupalSettings.janrain.capture.load_js_url;
e.src = url;
var s = document.getElementsByTagName('script')[0];
s.parentNode.insertBefore(e, s);
