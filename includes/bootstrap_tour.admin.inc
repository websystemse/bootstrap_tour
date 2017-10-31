<?php
/**
 * @file
 * The file for admin forms and functionality for the bootstrap_tour entity
 */

/**
 * Implements hook_form().
 */
function bootstrap_tour_form($form, &$form_state, $tour = NULL) {
  $form = array();

  if (empty($tour)) {
    $tour = new stdClass();
  }

  $form['title'] = array(
    '#weight' => 0,
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('A human readable name for this site tour.'),
    '#required' => TRUE,
    '#default_value' => !empty($tour->title) ? $tour->title : '',
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('Used for administrative purposes only.'),
    '#default_value' => !empty($tour->description) ? $tour->description : '',
    '#weight' => 0,
  );
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#description' => t('Indicate whether or not this tour is enabled or disabled.'),
    '#default_value' => (!empty($tour->enabled) || arg(3) == 'add') ? 1 : 0,
  );
  $form['roles'] = array(
    '#type' => 'select',
    '#title' => t('Roles'),
    '#description' => t('Which roles can access this tour? Leave blank for all.'),
    '#multiple' => TRUE,
    '#options' => user_roles(),
    '#default_value' => !empty($tour->roles) ? explode(',', $tour->roles) : '',
  );
  $form['wandered_off_message'] = array(
    '#type' => 'checkbox',
    '#title' => t('Wandered off message'),
    '#description' => t('Indicate whether or not a wandered off message should be dispalyed, if the user goes to an other page before ending the tour.'),
    '#default_value' => (!empty($tour->wandered_off_message) || arg(3) == 'add') ? 1 : 0,
  );
  $form['autorun'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically run'),
    '#description' => t('Should this tour start automatically the first time a user visits the path of the first step? If this is checked, the first step must have the "Path" field filled out.'),
    '#default_value' => (!empty($tour->autorun) || arg(3) == 'add') ? 1 : 0,
  );
  $form['start_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Start Path'),
    '#description' => t("The path where the tour should start if 'Automatically Run' is checked. The path setting of the first step will be ignored. Enter &lt;front&gt; for the front page."),
    '#default_value' => !empty($tour->start_path) ? $tour->start_path : '',
    '#states' => array(
      'visible' => array(
        'input[name="autorun"]' => array('checked' => TRUE),
      ),
      'required' => array(
        ':input[name=autorun]' => array('checked' => TRUE),
      ),
    ),
  );

  field_attach_form('bootstrap_tour', $tour, $form, $form_state);
  $form['field_bs_tour_description']['#weight'] = 1;

  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#required' => TRUE,
    '#size' => 50,
    '#maxlength' => 255,
    '#weight' => 0,
    '#default_value' => !empty($tour->name) ? $tour->name : '',
    '#machine_name' => array(
      'source' => array('title'),
      'exists' => '_bootstrap_tour_name_exists',
    ),
  );

  if (!empty($form['title']['#field_replacement'])) {
    $form['name']['#machine_name']['source'] = array(
      'title_field',
      title_active_language(),
      0,
      'value',
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Tour'),
    '#weight' => 20,
    '#suffix' => ' ' . l(t('Cancel'), 'admin/structure/tours'),
  );
  return $form;
}

/**
 * Implements hook_form_validate().
 */
function bootstrap_tour_form_validate($form, &$form_state) {
  if ($form_state['values']['autorun'] && empty($form_state['values']['start_path'])) {
    form_set_error('start_path', "You must provide a path if the 'Automatically Run' option is checked.");
  }
}


/**
 * Implements hook_form_submit().
 */
function bootstrap_tour_form_submit($form, &$form_state) {
  $tour = entity_ui_form_submit_build_entity($form, $form_state);
  $tour->roles = implode(',', $form_state['values']['roles']);
  $tour->save();
  drupal_set_message(t('Bootstrap tour @name has been saved.', array('@name' => $tour->name)));
  $form_state['redirect'] = 'admin/structure/tours';
}

/**
 * Function to check if the machine name already exists.
 */
function _bootstrap_tour_name_exists($name, $element, &$form_state) {
  $result = db_select('bootstrap_tour_tour', 't')
    ->condition('t.name', $name)
    ->range(0, 1)
    ->fields('t', ['bootstrap_tour_id'])
    ->execute();
  return ($result->rowCount() == 0 ? FALSE : TRUE);
}

/**
 * Admin settings form.
 */
function bootstrap_tour_admin_settings() {
  $form = array();

  $form['bootstrap_tour_use_standalone'] = array(
    '#title' => t('Use standalone JS and CSS'),
    '#description' => t('If you use this module with a non-bootstrap theme, checking this will mean the module will not require bootstrap css and js to work'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('bootstrap_tour_use_standalone'),
  );

  if (module_exists('title')) {
    $form['bootstrap_tour_create_title_override'] = array(
      '#value' => t('Toggle'),
      '#prefix' => t('<div class="form-item"><label>Toggle title integration, providing a translatable title field.</label>'),
      '#suffix' => '<div class="description">' . t('Current status: %status', array(
        '%status' => title_field_replacement_enabled('bootstrap_tour', 'bootstrap_tour', 'title') ? 'Enabled' : 'Disabled',
      )) . '</div></div>',
      '#type' => 'submit',
      '#submit' => array('_bootstrap_tour_toggle_title_field'),
    );
  }

  return system_settings_form($form);
}

/**
 * Function to toggle title override field via the Title module.
 */
function _bootstrap_tour_toggle_title_field() {
  drupal_set_message('Toggled title field.');
  title_field_replacement_toggle('bootstrap_tour', 'bootstrap_tour', 'title');
}
