(function ($) {

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
      var links = $('a.janrain_capture_anchor').once('capture-processed');
      var length = links.length;
      if (links.length !== 0) {
        var i, link;
        for (i = 0; i < length; i++) {
          link = links[i];
          // Add the necessary classes to ensure the link is opened in a dialog.
          $(link).addClass('iframe fancy');
        }
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
