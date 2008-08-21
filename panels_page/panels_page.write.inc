<?php
// $Id$

/**
 * @file panels_page.write.inc
 *
 * Functions responsible for writing panels_page data to the database.
 */

// Ugly, but whenever we need to write, we need to read, so...
require_once dirname(__FILE__) . '/panels_page.read.inc';

function panels_page_fields() {
  $fields = array();
  $schema = drupal_get_schema('panels_page');
  foreach ($schema['fields'] as $field => $info) {
    $fields[$field] = db_type_placeholder($info['type']);
  }
  return $fields;
}

/**
 * Check to see if the panel page needs to be saved due to a display having
 * been just saved.
 *
 * Wrapper for panels_page_save().
 */
function panels_page_save_display(&$panel_page, $id) {
  if (empty($panel_page->displays[$panel_page->current]['did']) || $panel_page->displays[$panel_page->current]['did'] == 'new') {
    return panels_page_save($panel_page);
  }
}

/**
 * Prepare an imported panel page for insertion into the database, then
 * insert it.
 *
 * Wrapper for panels_page_save().
 */
function panels_page_save_import(&$panel_page) {
  foreach ($panel_page->displays as $id => $info) {
    $imported_display = panels_save_display($info['display']);
    $panel_page->displays[$id]['did'] = $imported_display->did;
    unset($panel_page->displays[$id]['display']);
  }
  return panels_page_save($panel_page);
}

/**
 * Main entry point for panels_page save functions.
 */
function panels_page_save(&$panel_page) {
  $function = '_panels_page_' . ($panel_page->pid == 'new' ? 'insert' : 'save');
  return $function($panel_page);
}

/**
 * Insert a new panel page into the database.
 *
 */
function _panels_page_insert(&$panel_page) {
  $fields = $types = $values = array();
  panels_page_sanitize($panel_page);

  // Save the primary display, thus creating a $display->did.
  panels_save_display($panel_page->primary);
  foreach (panels_page_fields() as $field => $type) {
    if (isset($panel_page->$field)) {
      $fields[] = $field;
      $types[] = $type;

      $serialize = in_array($field, array('arguments', 'displays', 'contexts', 'relationships', 'switcher_options'));
      $values[] = $serialize ? serialize($panel_page->$field) : $panel_page->$field;
    }
  }
  // TODO horrible, ugly to do it this way, but we shouldn't change the value in $panel_page. Why were we storing access this way, anyway? Why not just serialize?
  $values[array_search('access', $fields)] = implode(',', $values[array_search('access', $fields)]);

  // Build the query adding the new pid and did.
  $panel_page->pid = db_next_id('{panels_page}_pid');
  $sql = 'INSERT INTO {panels_page} (' . implode(', ', $fields) . ', did, pid) VALUES (' . implode(', ', $types) . ', %d, %d)';
  $values[] = $panel_page->primary->did;

  db_query($sql, $values);
  // Return the pid for convenience.
  return $panel_page->pid;
}

/**
 * Save an existing panel page to the database.
 */
function _panels_page_save(&$panel_page) {
  $values = $pairs = array();

  // Save the display if one was given to us.
  if (!empty($panel_page->display)) {
    panels_save_display($panel_page->display);
    if ($panel_page->current != 'primary') {
      $panel_page->displays[$panel_page->current]['did'] = $panel_page->display->did;
      unset($panel_page->displays[$panel_page->current]['display']);
    }
  }

  panels_page_sanitize($panel_page);

  // Build arrays of fields and types (resp. pairs of both) and of values.
  foreach (panels_page_fields() as $field => $type) {
    // Skip empty values.
    if (isset($panel_page->$field)) {
      $pairs[] = "{$field} = {$type}";

      // Build the $values array, serializing some fields.
      $serialize = in_array($field, array('arguments', 'displays', 'contexts', 'relationships', 'switcher_options'));
      $values[] = $serialize ? serialize($panel_page->$field) : $panel_page->$field;
  }
  // TODO horrible, ugly to do it this way, but we shouldn't change the value in $panel_page. Why were we storing access this way, anyway? Why not just serialize?
  $values[array_search('access', $fields)] = implode(',', $values[array_search('access', $fields)]);

  // Build the query filtering by the primary key.
  $sql = 'UPDATE {panels_page} SET ' . implode(', ', $pairs) . ' WHERE pid = %d';
  $values[] = $panel_page->pid;

  db_query($sql, $values);
  // Return the pid for convenience.
  return $panel_page->pid;
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
  // Make sure the primary display is the current display.
  panels_page_fetch_primary_display($panel_page);
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
      $output .= $prefix . "    '$key' => " . panels_var_export($value, '    ') . ",\n";
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

