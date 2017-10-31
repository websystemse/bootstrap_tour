<?php

/**
 * @file
 * Hooks provided by the Bootstrap Tour module.
 */

/**
 * @defgroup bootstrap_tour_api_hooks Bootstrap tour API Hooks
 * @{
 * Functions to modify Bootstrap Tour entities.
 * @}
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Respond to Tour deletion.
 * @param $tour
 *   The Tour entity that is being deleted.
 *
 */
function hook_bootstrap_tour_delete($tour) {
  db_delete('mytable')
    ->condition('tour_id', $tour->bootstrap_tour_step_id)
    ->execute();
}

/**
 * Respond to deletion of a Tour revision.
 *
 * @param $tour
 *   The Tour revision (Tour object) that is being deleted.
 *
 * @ingroup bootstrap_tour_api_hooks
 */
function hook_bootstrap_tour_revision_delete($tour) {
  db_delete('mytable')
    ->condition('vid', $tour->vid)
    ->execute();
}

/**
 * Respond to creation of a new Tour.
 *
 * @param $tour
 *   The Tour that is being created.
 *
 * @ingroup bootstrap_tour_api_hooks
 */
function hook_bootstrap_tour_insert($tour) {
  db_insert('mytable')
    ->fields(array(
      'bootstrap_tour_step_id' => $tour->bootstrap_tour_step_id,
      'extra' => $tour->extra,
    ))
    ->execute();
}

/**
 * Act on a Tour being inserted or updated.
 *
 * @param $tour
 *   The Tour that is being inserted or updated.
 *
 * @ingroup bootstrap_tour_api_hooks
 */
function hook_bootstrap_tour_presave($tour) {
  if ($tour->bootstrap_tour_step_id && $tour->moderate) {
    // Reset votes when Tour is updated:
    $tour->score = 0;
    $tour->users = '';
    $tour->votes = 0;
  }
}

/**
 * Respond to updates to a Tour.
 *
 * @param $tour
 *   The Tour that is being updated.
 *
 * @ingroup bootstrap_tour_api_hooks
 */
function hook_bootstrap_tour_update($tour) {
  db_update('mytable')
    ->fields(array('extra' => $tour->extra))
    ->condition('bootstrap_tour_step_id', $tour->bootstrap_tour_step_id)
    ->execute();
}

/**
 * @} End of "addtogroup hooks".
 */
