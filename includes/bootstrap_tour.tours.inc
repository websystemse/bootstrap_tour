<?php

/**
 * @file
 * Contains a form which lists all accessible tours and can start them.
 */

/**
 * Implements hook_form().
 */
function bootstrap_tour_tours_form($form, &$form_state) {
  $form['description'] = [
    '#markup' => '<h2>' . t('Learn more about the features of the website through our site tours.') . '</h2>',
  ];

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

        'title' => array(
          '#markup' => '<h3>' . $tour->title . '</h3>',
        ),
        'description' => array('#label_display' => 'hidden') + field_view_field('bootstrap_tour', $tour, 'field_bs_tour_description'),
        'button' => array(
          '#type' => 'button',
          '#value' => t('Start this tour'),
          '#id' => $tour->bootstrap_tour_id,
          '#attributes' => array(
            'class' => array(
              'button-tour',
              'button',
              'button--primary',
            ),
            'data-name' => array($tour->name),
            'data-path' => array($tour->start_path !== '*' ? url($tour->start_path) : ''),
          ),
        ),
      );
    }
  }

  $path = drupal_get_path('module', 'bootstrap_tour');
  drupal_add_js($path . '/js/bootstrap-tour-tours.js', array('weight' => 1));
  return $form;
}
