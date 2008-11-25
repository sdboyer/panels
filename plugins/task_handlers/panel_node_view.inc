<?php
// $Id$

function panels_panel_node_view_delegator_task_handlers() {
  return array(
    'panel_node_view' => array(
      'title' => t('Panel'),
      'task type' => 'node_view',
      'admin title' => 'panels_panel_node_view_title',
      'render' => 'panels_panel_node_view_render',
      'add forms' => array(
        'node_type' => t('Select node type'),
        'choose' => t('Choose layout'),
        'layout-settings' => t('Configure layout'),
        'content' => t('Panel content'),
      ),
      'save' => 'panels_panel_node_view_save',
      'export' => 'panels_panel_node_view_export',
      'edit forms' => array(
        'content' => t('Panel content'),
        'settings' => t('General'),
        'context' => t('Contexts'),
        'layout' => t('Change layout'),
        'move' => '', // no title makes it a 'hidden' edit form.
        'layout-settings' => t('Layout settings'),
        'node_type' => t('Node type'),
      ),
      'forms' => array(
        'node_type' => array(
          'form' => 'panels_panel_node_view_edit_node_type',
        ),
        'settings' => array(
          'include' => drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          'form' => 'panels_delegator_edit_settings',
        ),
        'choose' => array(
          'include' => drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          'form' => 'panels_delegator_edit_choose',
          'validate' => 'panels_delegator_edit_choose_validate',
          'submit' => 'panels_delegator_edit_choose_submit',
        ),
        'layout' => array(
          'include' => drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          'no return' => TRUE,
          'alternate next' => 'move',
          'form' => 'panels_delegator_edit_layout',
          'validate' => 'panels_delegator_edit_layout_validate',
          'submit' => 'panels_delegator_edit_layout_submit',
        ),
        'move' => array(
          'include' => array(
            drupal_get_path('module', 'panels') . '/includes/display-layout.inc',
            drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          ),
          'form' => 'panels_delegator_edit_move',
          'submit' => 'panels_change_layout_submit',
        ),
        'layout-settings' => array(
          'include' => array(
            drupal_get_path('module', 'panels') . '/includes/display-layout-settings.inc',
            drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          ),
          'form' => 'panels_delegator_edit_layout_settings',
          'validate' => 'panels_edit_layout_settings_form_validate',
          'submit' => 'panels_edit_layout_settings_form_submit',
        ),
        'content' => array(
          'include' => array(
            drupal_get_path('module', 'panels') . '/includes/display-edit.inc',
            drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          ),
          'form' => 'panels_delegator_edit_content',
          'submit' => 'panels_edit_display_form_submit',
          'no blocks' => TRUE,
        ),
        'context' => array(
          'include' => drupal_get_path('module', 'panels') . '/includes/delegator.inc',
          'form' => 'panels_delegator_edit_context',
          'submit' => 'panels_delegator_edit_context_submit',
        ),
      ),
      'default conf' => array(
        'type' => array(),
        'no_blocks' => FALSE,
        'css_id' => '',
        'css' => '',
        'css_cache_file' => '',
        'contexts' => array(),
        'relationships' => array(),
      ),

      // delegator specific function
      'get context' => 'panels_panel_node_view_get_context',
    ),
  );
}

/**
 * Attempt to render a node.
 *
 * We must first check to ensure the node is of a type we're allowed
 * to render. If not, decline to render it by returning NULL.
 */
function panels_panel_node_view_render($handler, $node) {
  if (empty($handler->conf['type'][$node->type])) {
    return;
  }

  // Load the node into a context
  panels_load_include('plugins');
  $context = panels_context_create('node', $node);
  $context->identifier = t('Node being viewed');
  $context->keyword = 'node';

  // Load the display
  $display = drupal_clone($handler->conf['display']);
  $display->context = array(
    'panel-node' => $context,
  );
  $display->css_id = $handler->conf['css_id'];

  // Check to see if there is any CSS.
  if ($handler->conf['css_cache']) {
    if (!file_exists($handler->conf['css_cache'])) {
      // This will force the task handler to re-cache the CSS and save the filename:
      delegator_save_task_handler($handler);
    }
    dsm("Addning: " . $handler->conf['css_cache']);
    drupal_add_css($handler->conf['css_cache']);
  }

  // Since we're not using node_show() we need to emulate what it used to do.
  // Update the history table, stating that this user viewed this node.
  node_tag_new($node->nid);

  $output = panels_render_display($display);
  if (!empty($handler->conf['no_blocks'])) {
    print theme('page', $output, FALSE);
    // We return TRUE to let it know we handled this but have already
    // handled the output rendering ourselves.
    return TRUE;
  }
  return $output;
}

/**
 * Call through to the delegator function for this.
 */
function panels_panel_node_view_save(&$handler, $update) {
  panels_load_include('delegator');
  return panels_delegator_save($handler, $update);
}

/**
 * Call through to the delegator function for this.
 */
function panels_panel_node_view_export(&$handler, $indent) {
  panels_load_include('delegator');
  return panels_delegator_export($handler, $indent);
}

/**
 * Get empty contexts for use with the delegator panels pages.
 */
function panels_panel_node_view_get_context() {
  $context = panels_context_create_empty('node');
  $context->identifier = t('Node being viewed');
  $context->keyword = 'node';

  return array('panel-node' => $context);
}

/**
 * Set up a title for the panel based upon the node types selected.
 */
function panels_panel_node_view_title($handler, $task, $subtask_id) {
  $types = node_get_types();

  $names = array();
  foreach (array_filter($handler->conf['type']) as $type) {
    $names[] = check_plain($types[$type]->name);
  }

  if (empty($names)) {
    return t('Panel for all node types');
  }

  return format_plural(count($names), 'Panel for node type "@types"', 'Panel for node types "@types"', array('@types' => implode(', ', $names)));
}

/**
 * Choose a node type for this panel.
 */
function panels_panel_node_view_edit_node_type(&$form, &$form_state) {
  $types = node_get_types();
  foreach ($types as $type => $info) {
    $options[$type] = check_plain($info->name);
  }

  $form['conf']['type'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('This task handler will only be active for the selected node types. If no node types are selected, it will be active for all node types.'),
    '#default_value' => $form_state['handler']->conf['type'],
  );
}
