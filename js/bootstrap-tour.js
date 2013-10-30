(function($) {
  Drupal.behaviors.bootstrapTour = {
    attach: function(context) {
      Tour.prototype._isRedirect = function(path, currentPath) {
        // Override the isRedirect function so that we can support non-clean-URLs.
        currentPath = '/' + (location.pathname+location.search).substr(1)
        if (path !== '/') {
          return (path && path !== currentPath);
        } else {
          return (path && currentPath.indexOf('?q=') !== -1);
        }
      }
      var tourConfig = Drupal.settings.bootstrapTour.tour;
      if (!tourConfig) {
        return;
      }
      var path = Drupal.settings.basePath;
      if (path.charAt(path.length - 1) == "/") path = path.substr(0, path.length - 1);

      var t = new Tour({
        storage: window.localStorage,
        basePath: path
      });
      $.each(tourConfig.steps, function(index, step) {
        var options = {
          title: step.title,
          content: step.content,
          placement: step.placement,
          animation: true
        }
        if (step.path) {
          options.path = '/';
          if (step.path.trim() != '<front>') {
            if (!tourConfig.cleanUrls) {
              options.path += '?q=' // Don't need the first / in this case.
            }
            options.path += step.path;
          }
          if (!(tourConfig.isFirstStep && index == 0) && step.path.indexOf('?tour') === -1 && step.path.indexOf('&tour') === -1) {
            if (!tourConfig.cleanUrls) {
              options.path += '&';
            } else {
              options.path += '?';
            }
            options.path += 'tour=' + tourConfig.name + '&step=' + index;
          }
        }
        if (step.selector == '') {
          options.orphan = true;
        } else {
          options.element = step.selector;
        }
        t.addSteps([options])
      });

      if (tourConfig.force && tourConfig.isFirstStep) {
        // Manually restart if "force" is true and we're on the path of the first step.
        t.restart();
      } else {
        t.start();
      }
      $(window).trigger('resize');
    }
  }
})(jQuery);

