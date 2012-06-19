(function ($) {

  Drupal.janrainCapture = Drupal.janrainCapture || {};

  // Override the resize method on the Drupal.janrainCapture object so
  // as to use a fancybox for it.
  Drupal.janrainCapture.resize = function(jargs) {
    var args = $.parseJSON(jargs);
    $("#fancybox-inner, #fancybox-wrap, #fancybox-content, #fancybox-frame")
      .css({
        width: args.w,
        height: args.h
      });
    $.fancybox.resize();
    $.fancybox.center();
  }

  // Override the passwordRecover method on the Drupal.janrainCapture object so
  // as to use a fancybox for it.
  Drupal.janrainCapture.passwordRecover = function(url) {
    $.fancybox({
      type: "iframe",
      href: url,
      padding: 0,
      scrolling: "no",
      autoScale: true,
      width: 666,
      autoDimensions: false
    });
  }

  Drupal.behaviors.janrainCaptureUi = {
    attach: function(context, settings) {

      // Make all Capture signin and profile links appear in a fancybox.
      if ($.fn.fancybox) {
        $(".janrain_capture_anchor", context).once("capture-ui").addClass('iframe').fancybox({
          padding: 0,
          scrolling: "no",
          autoScale: true,
          width: 666,
          autoDimensions: false
        });
      }
    }
  };
})(jQuery);
