(function($) {
  Drupal.behaviors.bootstrapTour = {
    attach: function(context) {
      var t = new Tour({
        debug: true,
        storage: window.localStorage,
      });
      t.addSteps([
        {
          title: "Who can see this content?",
          content: "This box tells you about which users can view this page.",
          element: '.btn-small[title=Edit]',
          placement: 'bottom',
          animation: false
        }, {
          title: "Who can see this content?",
          content: "This box tells you about which users can view this page.",
          element: '#panels-ipe-paneid-new-10',
          animation: false,
          placement: 'bottom'
        }
      ]);
      t.start();
    }
  }
})(jQuery);

