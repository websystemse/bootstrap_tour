<?php

/**
 * @file
 * Defines the inline entity form controller for BootstrapTourStep entities.
 */

/**
 * Ensure Bootstrap Tours are imported and exported with tour steps.
 */
class BootstrapTourEntityController extends EntityAPIControllerExportable {

  /**
   * Fix the entity reference.
   */
  public function export($entity, $prefix = '') {
    $wrapper = entity_metadata_wrapper('bootstrap_tour', $entity);
    if (field_info_instance('bootstrap_tour', 'bootstrap_tour_step_reference', 'bootstrap_tour')) {
      $entity->steps = $wrapper->bootstrap_tour_step_reference->value();
      // Need to remove the step reference as they will be overriden anyway.
      // Remove step keys as they are ignored anyway.
      unset($entity->bootstrap_tour_step_reference);
      foreach ($entity->steps as $step) {
        unset($step->bootstrap_tour_step_id);
      }
    }
    return parent::export($entity, $prefix);
  }

  /**
   * Add in a presave before any else to handle imported steps.
   */
  public function invoke($hook, $entity) {
    if ($hook == 'presave' && !empty($entity->steps)) {
      $step_ids = array();
      if (!empty($entity->original)) {
        $step_ids = field_get_items('bootstrap_tour', $entity->original, 'bootstrap_tour_step_reference');
      }
      $steps = $entity->steps;
      unset($entity->steps);

      foreach ($steps as $index => $step) {
        $new_step = entity_create('bootstrap_tour_step', array(
          'path' => isset($step['path']) ? $step['path'] : '',
          'selector' => $step['selector'],
          'placement' => $step['placement'],
          'title' => $step['title'],
          'content' => $step['content'],
          'content_text_format' => $step['content_text_format'],
        ));
        // If a step of index ID existed, use existing id.
        if (!empty($step_ids[$index])) {
          $new_step->bootstrap_tour_step_id = $step_ids[$index]['target_id'];
          unset($new_step->is_new);
          unset($step_ids[$index]);
        }
        $new_step->save();
        $entity->bootstrap_tour_step_reference[LANGUAGE_NONE][$index]['target_id'] = $new_step ? $new_step->bootstrap_tour_step_id : NULL;
      }
      // Delete any orphaned steps.
      if ($step_ids) {
        entity_delete_multiple('bootstrap_tour_step', $step_ids);
      }
    }
    parent::invoke($hook, $entity);
  }

  /**
   * Save script callbacks to file.
   */
  public function save($entity) {
    parent::save($entity);

    $entity_ids = array();
    foreach ($entity->bootstrap_tour_step_reference[LANGUAGE_NONE] as $step) {
      $entity_ids[] = $step['target_id'];
    }
    $entities = entity_load('bootstrap_tour_step', $entity_ids);

    $found = FALSE;

    $script = "(function($){
    Drupal.bs_tours.callbacks['$entity->name'] = {};
    Drupal.bs_tours.callbacks['$entity->name'].steps = {};";
    foreach ($entities as $step_entity) {
      if (!empty($step_entity->script_callbacks)) {
        $found = TRUE;
        $script .= "
        Drupal.bs_tours.callbacks['$entity->name'].steps['$step_entity->bootstrap_tour_step_id'] = $step_entity->script_callbacks";
      }
    }
    $script .= '
    })(jQuery);';
    $directory = 'public://js/bootstrap_tour/';

    if ($found) {
      if (file_exists($directory . $entity->name . '.js' && $script != file_get_contents($directory . $entity->name . '.js'))) {
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
      }
      file_unmanaged_save_data($script, $directory . $entity->name . '.js', FILE_EXISTS_REPLACE);
    }
    else {
      file_unmanaged_delete($directory . $entity->name . '.js');
    }
  }

}

/**
 * Adjust the table headers when viewing the overview page.
 */
class BootstrapTourUIController extends EntityDefaultUIController {

  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header[] = t('Enabled');
    $additional_header[] = t('Description');
    $additional_header[] = t('# Steps');
    $additional_header[] = t('Auto Run');
    $additional_header[] = t('Start Path');

    $headers = parent::overviewTableHeaders($conditions, $rows, $additional_header);
    $headers[0] = 'Tour Name';

    return $headers;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableRow().
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {

    $additional_cols[] = $entity->enabled ? 'Yes' : 'No';
    $additional_cols[] = check_plain($entity->description);
    $additional_cols[] = count($entity->bootstrap_tour_step_reference[LANGUAGE_NONE]);
    $additional_cols[] = $entity->autorun ? 'Yes' : 'No';
    $additional_cols[] = check_plain($entity->start_path);

    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}

/**
 * Adjusts the Inline Entity Form used for adding/editing/removing tour steps.
 */
class BootstrapTourStepInlineEntityFormController extends EntityInlineEntityFormController {

  /**
   * Overrides EntityInlineEntityFormController::defaultLabels().
   */
  public function defaultLabels() {
    $labels = array(
      'singular' => t('step'),
      'plural' => t('steps'),
    );
    return $labels;
  }

  /**
   * Overrides EntityInlineEntityFormController::tableFields().
   */
  public function tableFields($bundles) {
    $fields = parent::tableFields($bundles);
    $fields['title']['label'] = t('Popup Title');

    $fields['selector'] = array(
      'type' => 'property',
      'label' => t('CSS Selector'),
      'weight' => 100,
    );
    $fields['placement'] = array(
      'type' => 'property',
      'label' => t('Placement'),
      'weight' => 101,
    );
    $fields['path'] = array(
      'type' => 'property',
      'label' => t('Path'),
      'weight' => 102,
    );
    return $fields;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    $step = $entity_form['#entity'];

    $entity_form['path'] = array(
      '#type' => 'textfield',
      '#title' => t('Path'),
      '#description' => t('Upon which path should this step be displayed? If subsequent steps will use the same path as previous step(s), you can leave this blank. Enter &lt;front&gt; for the front page.'),
      '#default_value' => !empty($step->path) ? $step->path : '',
    );
    $entity_form['selector'] = array(
      '#type' => 'textfield',
      '#title' => t('CSS Selector'),
      '#size' => 50,
      '#maxlength' => 255,
      '#default_value' => !empty($step->selector) ? $step->selector : '',
    );
    $entity_form['placement'] = array(
      '#type' => 'select',
      '#title' => t('Placement'),
      '#options' => array(
        'top' => t('Top'),
        'bottom' => t('Bottom'),
        'left' => t('Left'),
        'right' => t('Right'),
      ),
      '#default_value' => !empty($step->placement) ? $step->placement : '',
    );
    $entity_form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Popup Title'),
      '#required' => TRUE,
      '#default_value' => !empty($step->title) ? $step->title : '',
    );
    $entity_form['content'] = array(
      '#type' => 'text_format',
      '#base_type' => 'textarea',
      '#title' => t('Popup Content'),
      '#format' => !empty($step->content_text_format) ? $step->content_text_format : NULL,
      '#default_value' => !empty($step->content) ? $step->content : '',
    );
    $entity_form['script_callbacks'] = array(
      '#title' => 'Script callbacks',
      '#type' => 'textarea',
      '#default_value' => !empty($step->script_callbacks) ? $step->script_callbacks : NULL,
    );
    $entity_form['token_desc'] = array(
      '#markup' => '<label>' . t('Available tokens are node and user. eg [node:nid], [user:uid]') . '</label>',
    );

    $langcode = entity_language('bootstrap_tour_step', $step);
    field_attach_form('bootstrap_tour_step', $step, $entity_form, $form_state, $langcode);

    return $entity_form;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityFormSubmit().
   */
  public function entityFormSubmit(&$entity_form, &$form_state) {
    parent::entityFormSubmit($entity_form, $form_state);
    $entity = $entity_form['#entity'];
    $entity->content_text_format = $entity->content['format'];
    $entity->content = $entity->content['value'];
  }

}
