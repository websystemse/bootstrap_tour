(function($) {
  Drupal.behaviors.bootstrapTour = {
    attach: function(context) {
      var tourConfig = Drupal.settings.bootstrapTour.tour;
      if (!tourConfig) {
        return;
      }

      var t = new Tour({ storage: window.localStorage, debug: true });
      $.each(tourConfig.steps, function(index, step) {
        t.addSteps([{
          title: step.title,
          content: step.content,
          element: step.selector,
          placement: step.placement,
          path: '/' + step.path,
          animation: false
        }])
      });

      if (tourConfig.force && tourConfig.isFirstStep) {
        // Manually restart if "force" is true and we're on the path of the first step.
        t.restart();
      } else {
        t.start();
      }
    }
  }
})(jQuery);

