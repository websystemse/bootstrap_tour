(function ($) {
  Drupal.behaviors.bootstrapTourTours = {
    attach: function (context, settings) {
      $('.button-tour').once('stuff').click(function (e) {
        e.preventDefault();
        var tourName = 'tour_' + this.getAttribute('data-name');
        var triggeringElement = this;
        resetStatus(this.getAttribute('data-name')).done(function () {
          handleStatus(triggeringElement, tourName)
        });

        function resetStatus(name) {
          return $.ajax(Drupal.settings.basePath + 'bootstrap_tour/ajax/restart_tour/' + name);
        }

        function handleStatus(button, tourName) {
          var path = button.getAttribute('data-path');
          localStorage.removeItem(tourName + '_current_step');
          localStorage.removeItem(tourName + '_end');

          if (path.length) {
            window.location.href = path;
          } else {
            window.location.href = window.location.href;
          }
        }
      })
    }
  }
})(jQuery);
