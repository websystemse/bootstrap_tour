(function ($) {
  Drupal.bs_tours = {callbacks: {}};
  Drupal.behaviors.bootstrapTour = {
    attach: function (context) {
      var tourConfig = Drupal.settings.bootstrapTour.tour;
      if (!tourConfig) {
        return;
      }

      // Take the path and remove the tour GET arguments from it.
      function cleanPath(path) {
        // Replace the '?' mark with '&' temporarily.
        path = path.replace('?', '&');
        // Remove any instance of '&tour=' or '&step'.
        path = path.replace(/&tour=[^&]*/, '');
        path = path.replace(/&step=[^&]*/, '');
        // Now, change the first '&' back to a '?' mark.
        path = path.replace('&', '?');

        return path;
      }

      function replaceWildcard(path) {
        var newPath,
          pathFragments,
          currentPathFragments,
          partCount,
          currentPath,
          i;

        newPath = [];
        pathFragments = path.split('/');
        currentPath = '/' + tourConfig.current_path;
        currentPathFragments = currentPath.split('/');

        partCount = pathFragments.length;

        if (partCount > 0) {
          for (i = 0; i < partCount; i++) {
            if (pathFragments[i] == currentPathFragments[i] || pathFragments[i] == '*') {
              newPath[i] = currentPathFragments[i];
            } else {
              newPath[i] = pathFragments[i];
            }
          }
        }

        return newPath.length !== 0 ? newPath.join('/') : path;
      }

      Tour.prototype._isRedirect = function (path, currentPath) {
        if (path == null || path == Drupal.settings.basePath) {
          return false;
        }

        // Override the isRedirect function so that we can support non-clean-URLs.
        currentPath = '/' + tourConfig.current_path;
        path = cleanPath(path.replace(Drupal.settings.basePath, '/'));

        if (/\*/.test(path)) {
          path = replaceWildcard(path);
        }

        if (path !== '/') {
          return (path !== currentPath);
        } else {
          return (currentPath.indexOf('?q=') !== -1);
        }
      };

      var basePath = Drupal.settings.basePath;
      var prev = Drupal.t("« Prev");
      var next = Drupal.t("Next »");
      var endtour = Drupal.t("End Tour");
      var shown = false;
      var t = new Tour({
        name: 'tour_' + tourConfig.name,
        storage: window.localStorage,
        basePath: basePath,
        template: "<div class='popover tour'> \
          <div class='arrow'></div> \
          <h3 class='popover-title'></h3> \
          <div class='popover-content'></div> \
          <nav class='popover-navigation'> \
              <div class='btn-group'> \
                  <button class='btn btn-default' data-role='prev'>" + prev + "</button> \
                  <button class='btn btn-default' data-role='next'>" + next + "</button> \
              </div> \
              <button class='btn btn-default' data-role='end'>" + endtour + "</button> \
          </nav> \
          </div>",
        onShown: function () {
          shown = true;
        },
        onEnd: function () {
          $.ajax(basePath + 'bootstrap_tour/ajax/end_current_tour', {async: false});
        },
        redirect: function (path) {
          var browserPath = cleanPath("" + document.location.pathname + document.location.hash),
            cleanedPath = cleanPath(path),
            // Newer versions have a this.getCurrentStep() function - this is for backcompat.
            currentIndex = this._current,
            nextStep = this.getStep(currentIndex + 1),
            nextPath = nextStep ? basePath + nextStep.path : '',
            cleanedNextPath = cleanPath(nextPath),
            currentPath = cleanPath(path.replace(Drupal.settings.basePath, '/'));


          browserPath = browserPath.replace(new RegExp('^/' + Drupal.settings.pathPrefix), '/');

          if (/\*/.test(cleanedNextPath)) {
            cleanedNextPath = replaceWildcard(cleanedNextPath);
            nextPath = replaceWildcard(nextPath);
          }

          // If we haven't shown a single step and bootstrap tour is trying to
          // redirect, well, it means we've wandered off from the tour. Ask the
          // user if they'd like to return.
          if (!shown &&
            (browserPath !== cleanedPath && browserPath !== cleanedNextPath) &&
            (currentPath !== cleanedPath && currentPath !== cleanedNextPath)) {
            var wanderedOff = Drupal.t("You have wandered off from the tour! You will be automatically redirected back to the tour. Please click 'OK' to continue, or 'Cancel' to end the tour.");
            if (tourConfig.wandered_off_message != '1' || !window.confirm(wanderedOff)) {
              // The user has opted to leave the tour!
              this.end();
              return;
            }
          }

          // If the user has shown initiative and jumped to the next step, then
          // we advance the step counter for them, before redirecting to the
          // path which has &tour= and &step= in it.
          if (!shown &&
            (browserPath !== cleanedPath && browserPath === cleanedNextPath) ||
            (currentPath !== cleanedPath && currentPath === cleanedNextPath)) {
            this.setCurrentStep(currentIndex + 1);
            path = nextPath;
          }

          // We mark this as 'shown', so we don't ask them twice.
          shown = true;

          path = path.replace(new RegExp('^/user'), '/' + Drupal.settings.pathPrefix + 'user');

          document.location.href = path;
        }
      });

      var scripts = false;
      if (Drupal.bs_tours.callbacks[tourConfig.name]) {
        scripts = true;
      }

      $.each(tourConfig.steps, function (index, step) {
        var options = {
          title: step.title,
          content: step.content,
          placement: step.placement,
          animation: true
        };
        if (step.path) {
          options.path = '';
          if (step.path.trim() != '<front>') {
            if (!tourConfig.cleanUrls) {
              options.path += '?q='; // Don't need the first / in this case.
            }
            options.path += step.path;
          }

          if (step.path.indexOf('?tour') === -1 && step.path.indexOf('&tour') === -1) {
            if (!tourConfig.cleanUrls) {
              options.path += '&';
            } else {
              options.path += '?';
            }
            options.path += 'tour=' + tourConfig.name;
            if (!(tourConfig.isFirstStep && index == 0)) {
              options.path += '&step=' + index;
            }
          }
        }

        if (step.selector == '') {
          options.orphan = true;
        }
        else {
          options.element = step.selector;
          options.onShown = function (tour) {
            $(options.element).addClass('bootstrap-tour-selected');
            shown = true;
          };
          options.onHidden = function (tour) {
            $(options.element).removeClass('bootstrap-tour-selected');
          };
        }
        if (step.onShow) {
          options.onShow = (window[step.onShow] ? window[step.onShow] : function () {
          });
        }

        if (scripts && typeof Drupal.bs_tours.callbacks[tourConfig.name].steps[step.bootstrap_tour_step_id] != 'undefined') {
          for (var v in Drupal.bs_tours.callbacks[tourConfig.name].steps[step.bootstrap_tour_step_id]) {
            if (!Drupal.bs_tours.callbacks[tourConfig.name].steps[step.bootstrap_tour_step_id].hasOwnProperty(v)) {
              continue;
            }
            options[v] = Drupal.bs_tours.callbacks[tourConfig.name].steps[step.bootstrap_tour_step_id][v];
          }
        }

        t.addSteps([options]);
      });

      t.init();

      console.log(t);

      if (tourConfig.force && tourConfig.isFirstStep) {
        // Manually restart if "force" is true and we're on the path of the first step.
        t.restart();
      } else {
        t.start();
      }
    }
  }
})(jQuery);
