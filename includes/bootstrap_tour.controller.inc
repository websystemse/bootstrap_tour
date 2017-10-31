<?php

/**
 * @file
 * Defines the inline entity form controller for BootstrapTourStep entities.
 */

/**
 * A controller implementing exportables stored in the database.
 */
class BootstrapTourEntityController extends EntityAPIControllerExportable {

  /**
   * Overrides EntityAPIControllerExportable::export()
   *
   * Export step data instead of references and remove entity_translation data
   * .
   * {@inheritDoc}
   */
  public function export($entity, $prefix = '') {
    $wrapper = entity_metadata_wrapper('bootstrap_tour', $entity);

    // If Title module exists, export the default title as the original.
    if (module_exists('title') && !empty($entity->title_original)) {
      $entity->title = $entity->title_original;
      unset($entity->title_original);
    }

    $entity->field_bootstrap_tour_step = $wrapper->field_bootstrap_tour_step->value();
    unset($entity->entity_translation_handler_id);

    foreach ($entity->field_bootstrap_tour_step as &$step) {
      unset($step->item_id);
      unset($step->revision_id);
      unset($step->uuid);
      unset($step->entity_translation_handler_id);

      foreach ($step->translations->data as &$translation) {
        unset($translation['entity_id']);
        unset($translation['changed']);
        unset($translation['revision_id']);
      }
    }

    foreach ($entity->translations->data as &$translation) {
      unset($translation['entity_id']);
      unset($translation['changed']);
      unset($translation['revision_id']);
    }

    return parent::export($entity, $prefix);
  }

  /**
   * Overrides EntityAPIControllerInterface::import()
   *
   * We delete current step entities if the Tour already exists.
   *
   * {@inheritDoc}
   */
  public function import($export) {
    $vars = drupal_json_decode($export);
    if (is_array($vars)) {
      // Create step entities separately.
      $steps = $vars['field_bootstrap_tour_step'];
      unset($vars['field_bootstrap_tour_step']);
      $entity = $this->create($vars);
      if (!empty($entity->translations)) {
        $entity->translations = (object) $entity->translations;
      }
      foreach ($steps as $step) {
        /* @var \FieldCollectionItemEntity $field_collection_item */
        $field_collection_item = entity_create('field_collection_item', $step);
        $field_collection_item->setHostEntity('bootstrap_tour', $entity);
        if (!empty($field_collection_item->translations)) {
          $field_collection_item->translations = (object) $field_collection_item->translations;
        }
      }

      return $entity;
    }

    return NULL;
  }

  /**
   * Overrides EntityAPIControllerInterface::save()
   *
   * We save JavaScript callbacks to a file if they are set.
   *
   * {@inheritDoc}
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (!empty($entity->{$this->nameKey}) && !isset($entity->original)) {
      $entity->original = entity_load_unchanged($this->entityType, $entity->{$this->nameKey});
    }

    if (!empty($entity->{$this->idKey}) && !isset($entity->original)) {
      $entity->original = entity_load_unchanged($this->entityType, $entity->{$this->nameKey});
    }


    if (!empty($entity->original)) {
      $entity->{$this->idKey} = $entity->original->{$this->idKey};

      if (!empty($entity->original->field_bootstrap_tour_step[LANGUAGE_NONE])) {
        // @TODO: Förstå varför entities tas bort vid första import och feature står som overridden.
        entity_delete_multiple('field_collection_item', array_column($entity->original->field_bootstrap_tour_step[LANGUAGE_NONE], 'value'));
      }
    }

    $return = parent::save($entity, $transaction);

    if ($return) {
      if (empty($entity->field_bootstrap_tour_step[LANGUAGE_NONE])) {
        return $return;
      }
      $entity_ids = array_column($entity->field_bootstrap_tour_step[LANGUAGE_NONE], 'value');
      $entities = field_collection_item_load_multiple($entity_ids);
      $has_content = FALSE;
      $script = "(function($){\nDrupal.bs_tours.callbacks['$entity->name'] = {};\nDrupal.bs_tours.callbacks['$entity->name'].steps = {};";
      foreach ($entities as $step_entity) {
        if (!empty($step_entity->field_bs_tour_step_callbacks[LANGUAGE_NONE][0]['value'])) {
          $has_content = TRUE;
          $callback = $step_entity->field_bs_tour_step_callbacks[LANGUAGE_NONE][0]['value'];
          $script .= "\nDrupal.bs_tours.callbacks['$entity->name'].steps['$step_entity->item_id'] = $callback";
        }
      }
      $script .= "\n})(jQuery);";
      $directory = 'public://js/bootstrap_tour/';

      if ($has_content) {
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
        file_unmanaged_save_data($script, $directory . $entity->name . '.js', FILE_EXISTS_REPLACE);
      }
      else {
        file_unmanaged_delete($directory . $entity->name . '.js');
      }
    }

    return $return;
  }

}

/**
 * An override of the Default UI controller providing admin UI.
 */
class BootstrapTourUIController extends EntityDefaultUIController {

  /**
   * Overrides EntityDefaultUIController::overviewTableHeaders()
   *
   * {@inheritDoc}
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = array()) {
    $additional_header[] = t('Enabled');
    $additional_header[] = t('Description');
    $additional_header[] = t('# Steps');
    $additional_header[] = t('Auto Run');
    $additional_header[] = t('Start Path');

    $headers = parent::overviewTableHeaders($conditions, $rows, $additional_header);
    $headers[0] = t('Tour Name');

    return $headers;
  }

  /**
   * Overrides EntityDefaultUIController::overviewTableRow()
   *
   * {@inheritDoc}
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $additional_cols[] = $entity->enabled ? t('Yes') : t('No');
    $additional_cols[] = check_plain($entity->description);
    $additional_cols[] = !empty($entity->field_bootstrap_tour_step[LANGUAGE_NONE]) ? count($entity->field_bootstrap_tour_step[LANGUAGE_NONE]) : 0;
    $additional_cols[] = $entity->autorun ? t('Yes') : t('No');
    $additional_cols[] = check_plain($entity->start_path);

    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

}
