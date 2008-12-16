<?php
// $Id$

/**
 * Most of our config is handled by panels delegator.inc, but we do have
 * to customize it a little bit to match what we're doing with this task.
 */
function panels_panel_node_view_delegator_task_handlers() {
  panels_load_include('delegator');
  $conf = panels_delegator_get_config();

  $conf['task type'] = 'node_view';
  $conf['admin title'] = 'panels_panel_node_view_title';
  $conf['render'] = 'panels_panel_node_view_render';
  $conf['add forms']['node_type'] = t('Select node type');
  $conf['edit forms']['node_type'] = t('Select node type');
  $conf['forms']['node_type'] = array(
    'form' => 'panels_panel_node_view_edit_node_type',
  );
  $conf['default conf']['type'] = array();

  // This one is specific to Panels delegator.inc so that it knows what
  // contexts we use and can display them.
  $conf['get context'] = 'panels_panel_node_view_get_context';

  return array(
    'panel_node_view' => $conf,
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
//  panels_load_include('plugins');
  ctools_include('context');
  $context = ctools_context_create('node', $node);
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
 * Get empty contexts for use with the delegator panels pages.
 */
function panels_panel_node_view_get_context() {
  ctools_include('context');
  $context = ctools_context_create_empty('node');
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
