(function ($) {
  Drupal.behaviors.janrainCapture = {
    attach: function(context, settings) {
      window.CAPTURE = {
        resize: function(jargs) {
          var args = JSON.parse(jargs);

          $("#fancybox-inner, #fancybox-wrap, #fancybox-content, #fancybox-frame")
            .css({
              width: args.w,
              height: args.h
            });

          $.fancybox.resize();
          $.fancybox.center();
        },
        closeProfileEditor: function() {
          window.location.href = settings.janrainCapture.profile_sync_url;
        },
        closeRecoverPassword: function() {
          window.location.reload();
        },
        token_expired: function() {
          window.location.href = settings.janrainCapture.token_expired_url;
        },
        bp_ready: function() {
          if (typeof(Backplane) != 'undefined') {
            var channelId = encodeURIComponent(Backplane.getChannelID());
            $("a.janrain_capture_signin").each(function(){
              $(this).attr("href", $(this).attr("href") + "&bp_channel=" + channelId).click(function(){
                Backplane.expectMessages("identity/login");
              });
            });
          }
        },
        logout: function() {
          if (typeof(settings.janrainCapture.sso_address) != 'undefined') {
            JANRAIN.SSO.CAPTURE.logout({
              sso_server: "https://" + settings.janrainCapture.sso_address,
              logout_uri: settings.janrainCapture.logout_url
            });
          }
        }
      };
      if (typeof(settings.janrainCapture.backplane_server) != 'undefined'
        && typeof(settings.janrainCapture.backplane_bus_name) != 'undefined') {
        Backplane(CAPTURE.bp_ready);
        Backplane.init({
          serverBaseURL: settings.janrainCapture.backplane_server,
          busName: settings.janrainCapture.backplane_bus_name
        });
      }
      $(".fancy").fancybox({
        padding: 0,
        scrolling: "no",
        autoScale: true,
        width: 666,
        autoDimensions: false
      });
    }
  };
})(jQuery);
