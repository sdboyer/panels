<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_custom_panels_content_types() {
  $items['custom'] = array(
    'title' => t('Custom'),
    'weight' => -10,
    'single' => TRUE,
    'content_types' => 'panels_admin_content_types_custom',
    'render callback' => 'panels_content_custom',
    'editor render callback' => 'panels_admin_content_custom',
    'add callback' => 'panels_admin_edit_custom',
    'edit callback' => 'panels_admin_edit_custom',
    'title callback' => 'panels_admin_title_custom',
    'no override title' => TRUE,
  );
  return $items;
}

/**
 * Output function for the 'custom' content type. Outputs a custom
 * based on the module and delta supplied in the configuration.
 */
function panels_content_custom($subtype, $conf) {
  static $delta = 0;

  $block          = new stdClass();
  $block->module  = 'custom';
  $block->delta   = ++$delta;
  $block->subject = filter_xss_admin($conf['title']);
  $block->content = check_markup($conf['body'], $conf['format'], FALSE);

  return $block;
}

/**
 * Render callback for when the custom content is in the editor so that people
 * can have a preview on the spot.
 *
 * @param panels_display $display
 * @param stdClass $pane
 * @return stdClass $block
 */
function panels_admin_content_custom($display, $pane) {
  $block = new stdClass();
  $block->title = filter_xss_admin($pane->configuration['title']);
  // We don't want to render php output on preview here, because if something is
  // wrong the whole display will be borked. So we check to see if the php
  // evaluator filter is being used, and make a temporary change to the filter
  // so that we get the printed php, not the eval'ed php.
  $php_filter = FALSE;
  foreach (filter_list_format($pane->configuration['format']) as $filter) {
    if ($filter->name == 'PHP evaluator') { // TODO stupid way to check
      $php_filter = TRUE;
    }
  }
  // If a php filter is active, pass 1 to use core's most restrictive filter.
  $block->content = check_markup($pane->configuration['body'], $php_filter ? 1 : $pane->configuration['format']);
  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_custom() {
  return array(
    'custom' => array(
      'title' => t('New custom content'),
      'icon' => 'icon_block_custom.png',
      'path' => panels_get_path('content_types/custom'),
      'description' => t('Create a completely custom piece of HTML content.'),
      'category' => array(t('Custom'), -10),
    ),
  );
}

/**
 * Returns an edit form for the custom type.
 */
function panels_admin_edit_custom($id, $parents, $conf = NULL) {
  if (empty($conf)) {
    $conf = array('title' => '', 'body' => '', 'format' => FILTER_FORMAT_DEFAULT);
  }
  $form['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['title'],
    '#title' => t('Title'),
  );
  $form['body'] = array(
    '#title' => t('Body'),
    '#type' => 'textarea',
    '#default_value' => $conf['body'],
  );
  $parents[] = 'format';
  $form['format'] = filter_form($conf['format'], 1, $parents);

  return $form;
}

/**
 * Returns the administrative title for a type.
 */
function panels_admin_title_custom($subtype, $conf) {
  $output = t('Custom');
  if (!empty($conf['title'])) {
    $output .= " (" . filter_xss_admin($conf['title']) . ")";
  }
  return $output;
}

