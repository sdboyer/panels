<?php
// $Id$

/**
 * @file panels_page.write.inc
 *
 * Functions responsible for writing panels_page data to the database.
 */

// Ugly, but whenever we need to write, we need to read, so...
require_once dirname(__FILE__). '/panels_page.read.inc';

/**
 * Page callback to import a panel page from PHP code.
 */
function panels_page_import_page() {
  if (!empty($_POST['form_id']) && $_POST['form_id'] == 'panels_page_edit_form') {
    $panel_page = unserialize($_SESSION['pp_import']);
    drupal_set_title(t('Import panel page "@s"', array('@s' => $panel_page->title)));
    return drupal_get_form('panels_page_edit_form', $panel_page);
  }

  return drupal_get_form('panels_page_import_form');
}

/**
 * Form to for the panel page import
 */
function panels_page_import_form(&$form_state) {
  $form['panel_page'] = array(
    '#type' => 'textarea',
    '#title' => t('Panel page code'),
    '#cols' => 60,
    '#rows' => 15,
    '#description' => t('Cut and paste the results of an exported panel page here.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  $form['#redirect'] = FALSE;
  return $form;
}

/**
 * Handle the submit button on importing a panel page.
 */
function panels_page_import_form_submit($form, &$form_state) {
  ob_start();
  eval($form['panel_page']);
  ob_end_clean();

  if (isset($page)) {
    drupal_set_title(t('Import panel page "@s"', array('@s' => $page->title)));
    // As $page contains non-stdClass objects,
    // it needs to be serialized before being stored in the session variable.
    $_SESSION['pp_import'] = serialize($page);
    $output = drupal_get_form('panels_page_edit_form', $page);
    print theme('page', $output);
    exit;
  }
  else {
    drupal_set_message(t('Unable to get a panel page out of that.'));
  }
}

/**
 * Check to see if the panel page needs to be saved due to a display having
 * been just saved.
 */
function panels_page_set_display($panel_page, $id, $display) {
  $save = FALSE;
  if ($panel_page->pid == 'new') {
    $save = TRUE;

    if (empty($id)) {
      $panel_page->display = $display;
      $panel_page->did = $display->did;
    }
  }

  if (!empty($id)) {
    if (empty($panel_page->displays[$id]['did']) || $panel_page->displays[$id]['did'] == 'new') {
      $save = TRUE;
      $panel_page->displays[$id]['did'] = $display->did;
      if (isset($panel_page->displays[$id]['display'])) {
        unset($panel_page->displays[$id]['display']);
      }
    }
  }

  if ($save) {
    panels_page_save($panel_page);
  }
}

/**
 * Save a panel page.
 */
function panels_page_save($panel_page) {
  $fields = $types = $values = $pairs = array();
  // Save the display if one was given to us.
  if (!empty($panel_page->display)) {
    $display = panels_save_display($panel_page->display);
  }
  // Ensure empty values get translated correctly. Also make sure we don't
  // mess up the original.
  $page_clone = drupal_clone(panels_page_sanitize($panel_page));

  // Check to see if we need to save any imported displays.
  foreach ($page_clone->displays as $did => $info) {
    if (isset($info['display'])) {
      $d = panels_save_display($info['display']);
      $page_clone->displays[$did]['did'] = $d->did;
      unset($page_clone->displays[$did]['display']);
    }
  }

  // If pid is set to our "magic value", this is an insert, otherwise an update.
  $insert = $page_clone->pid && $page_clone->pid == 'new';

  // Serialize the access string.
  $page_clone->access = $page_clone->access ? implode(', ', $page_clone->access) : '';

  // Build arrays of fields and types (resp. pairs of both) and of values.
  foreach (panels_page_fields() as $field => $type) {
    // Skip empty values.
    if (isset($page_clone->$field)) {
      if ($insert) {
        $fields[] = $field;
        $types[] = $type;
      }
      else {
        $pairs[] = "$field = $type";
      }
      // Build the $values array, serializing some fields.
      $serialize = in_array($field, array('arguments', 'displays', 'contexts', 'relationships', 'switcher_options'));
      $values[] = $serialize ? serialize($page_clone->$field) : $page_clone->$field;
    }
  }

  if ($insert) {
    // Determine the new primary key.
    $page_clone->pid = db_next_id('{panels_page}_pid');
    // Build the query adding the new primary key and the did.
    $sql = 'INSERT INTO {panels_page} (' . implode(', ', $fields) . ', did, pid) VALUES (' . implode(', ', $types) . ', %d, %d)';
    $values[] = $display->did;
  }
  else {
    // Build the query filtering by the primary key.
    $sql = 'UPDATE {panels_page} SET ' . implode(', ', $pairs) . ' WHERE pid = %d';
  }
  $values[] = $page_clone->pid;

  db_query($sql, $values);
  menu_rebuild();
  return $page_clone->pid;
}

/**
 * Delete a panel page and its associated displays.
 */
function panels_page_delete($panel_page) {
  // Delete the panel page.
  db_query("DELETE FROM {panels_page} WHERE pid = %d", $panel_page->pid);
  menu_rebuild();

  // Delete the primary display.
  panels_delete_display($panel_page->did);

  // Delete all secondary displays.
  if (!empty($panel_page->displays)) {
    foreach ($panel_page->displays as $info) {
      if ($info['did'] !== 'new') {
        panels_delete_display($info['did']);
      }
    }
  }
}

/**
 * Export a panel page into PHP code for use in import.
 *
 * The code returned from can be used directly in panels_page_save().
 */
function panels_page_export($panel_page, $prefix = '') {
  $output = '';
  $fields = panels_page_fields();
  $output .= $prefix . '$page = new stdClass()' . ";\n";
  $output .= $prefix . '$page->pid = \'new\'' . ";\n";
  foreach (array_keys($fields) as $field) {
    if (!isset($panel_page->$field) || in_array($field, array('displays', 'contexts', 'display'))) {
      continue;
    }
    $output .= $prefix . '  $page->' . $field . ' = ' . panels_var_export($panel_page->$field, '  ') . ";\n";
  }
  // Export the contexts.
  $output .= $prefix . '$page->contexts = array()' . ";\n";
  foreach ($panel_page->contexts as $id => $info) {
    $output .= $prefix . '  $page->contexts[\'' . $id . "'] = array(\n";
    foreach ($info as $key => $value) {
      $output .= $prefix . "    '$key' => " . panels_var_export($value, '    ') . ";\n";
    }
    $output .= $prefix . "  );\n";
  }

  // Export the primary display
  $display = !empty($panel_page->display) ? $panel_page->display : panels_load_display($panel_page->did);
  $output .= panels_export_display($display, $prefix);
  $output .= $prefix . '$page->display = $display' . ";\n";

  // Export all secondary displays
  $output .= $prefix . '$page->displays = array()' . ";\n";
  foreach ($panel_page->displays as $did => $info) {
    $output .= $prefix . '  $page->displays[\'' . $did . "']['title'] = '" . check_plain($info['title']) . "';\n";
    $output .= $prefix . '  $page->displays[\'' . $did . "']['argument_id'] = '" . check_plain($info['argument_id']) . "';\n";
    $display = !empty($info['display']) ? $info['display'] : panels_load_display($info['did']);
    $output .= panels_export_display($display, $prefix . '    ');
    $output .= $prefix . '  $page->displays[\'' . $did . "']['display'] =  \$display;\n";
  }
  return $output;
}

