<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_custom_php_panels_content_types() {
  $items['custom_php'] = array(
    'title' => t('Custom PHP'),
    'weight' => -10,
    'single' => TRUE,
    'content_types' => 'panels_admin_content_types_custom_php',
    'render callback' => 'panels_content_custom_php',
    'editor render callback' => 'panels_admin_content_custom_php',
    'add callback' => 'panels_admin_edit_custom_php',
    'edit callback' => 'panels_admin_edit_custom_php',
    'title callback' => 'panels_admin_title_custom_php',
    'no override title' => TRUE,
  );
  return $items;
}

/**
 * Output function for the 'custom_php' content type. Allows sufficiently
 * privileged users to enter custom php code that is evaluated while
 * $context is in scope. Use with caution!
 */
function panels_content_custom_php($conf, $panel_args, $context) {
  static $delta = 0;
  $block = new stdClass();
  
  // eval() the php that has been entered by the user.
  eval($conf['body']);
  $block->delta = ++$delta;
  // Put our default member values into a separate array.
  $members = array(
    'module'  => 'custom',
    'subject' => filter_xss_admin($conf['title']),
    'content' => '',
  );
  // Iterate through the array of defaults and assign put in
  // any values that haven't already been set by the custom
  // php code.
  foreach ($members as $member => $value) {
    if (empty($block->$member)) {
      $block->$member = $value;
    }
  }
  
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
function panels_admin_content_custom_php($display, $pane) {
  $block = new stdClass();
  $block->title = filter_xss_admin($pane->configuration['title']);
  // We don't want to render php output on preview here, because if something is
  // wrong the whole display will be borked.
  $block->content = check_markup($pane->configuration['body'], 1);
  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_custom_php() {
  if (filter_access(2)) {
    return array(
      'custom_php' => array(
        'title' => t('Custom PHP content'),
        'icon' => 'icon_block_custom.png',
        'path' => panels_get_path('content_types/custom'),
        'description' => t('Custom PHP block with access to Panels context data. Use sparingly, and with caution!'),
        'category' => array(t('Custom'), -10),
      ),
    );
  }
}

/**
 * Returns an edit form for the custom_php type.
 */
function panels_admin_edit_custom_php($id, $parents, $conf = NULL) {
  if (!is_array($conf)) {
    $conf = array('title' => '', 'body' => '');
  }
  $form['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['title'],
    '#title' => t('Title'),
  );
  $body = t('Any content you want to have passed along to theme function for displaying MUST be stored in $block->content.') . "\n\n";
  $body .= t('MAKE SURE YOU ERASE ALL OF THIS HELP TEXT. Only error-free php code should be saved in this field.') . "\n\n";
  $body .= t('Do NOT use php tags.');
  
  $form['body'] = array(
    '#title' => t('Body'),
    '#type' => 'textarea',
    '#default_value' => isset($conf['body']) ? $conf['body'] : $body,
    '#rows' => 10,
    '#description' => t('The PHP code you enter here will be evaluated at render-time. Any context data present in the display will be available to this code in the $context variable. <strong>DO NOT</strong> include <em>@php</em> tags.', array('@php' => '<?php ?>')),
  );

  return $form;
}

/**
 * Returns the administrative title for a type.
 */
function panels_admin_title_custom_php($conf) {
  $output = t('Custom');
  if ($conf['title']) {
    $output .= " (" . filter_xss_admin($conf['title']) . ")";
  }
  return $output;
}

