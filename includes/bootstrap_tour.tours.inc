<?php
/**
 * @file
 * The file for all tours form and functionality for the bootstrap_tour entity
 */

/**
 * Implements hook_form().
 */
function bootstrap_tour_tours_form($form, &$form_state) {
  $form['description'] = array(
    '#markup' => '<h3>Forgotten about some of the websites features? Here you restart any of the site tours.</h3>',
  );

  $form['tours'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('tours-wrapper')),
  );

  $tours = bootstrap_tour_load_all();
  foreach ($tours as $tour) {
    if (bootstrap_tour_access($tour)) {
      $form['tours'][] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array(
            'tour-item',
          ),
        ),

        'button' => array(
          '#type' => 'button',
          '#value' => $tour->title,
          '#id' => $tour->bootstrap_tour_id,
          '#attributes' => array(
            'class' => array(
              'button-tour',
              'button',
              'button--primary',
            ),
            'data-name' => array($tour->name),
            'data-path' => array(url($tour->start_path)),
          ),
        ),

        'description' => array(
          '#prefix' => '<span class="description"><i>',
          '#markup' => (!empty($tour->description) ? $tour->description : 'No description available'),
          '#suffix' => '</i></span>',
        ),

      );
    }
  }

  $path = drupal_get_path('module', 'bootstrap_tour');
  drupal_add_js($path . '/js/bootstrap-tour-tours.js', array('weight' => 1));
  return $form;
}
