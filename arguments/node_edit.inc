<?php
// $Id$


/**
 * @file arguments/nid.inc
 *
 * Plugin to provide an argument handler for a Node edit form
 */
function panels_node_edit_panels_arguments() {
  $args['node_edit'] = array(
    'title' => t("Node edit form"),
    // keyword to use for %substitution
    'keyword' => 'node',
    'description' => t('Displays the node edit form for a node.'),
    'context' => 'panels_node_edit_context',
    'settings form' => 'panels_node_edit_settings_form',
    'settings form submit' => 'panels_node_edit_settings_form_submit',
    'displays' => 'panels_node_edit_displays',
    'choose display' => 'panels_node_edit_choose_display',
  );
  return $args;
}

/**
 * Discover if this argument gives us the node we crave.
 */
function panels_node_edit_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return panels_context_create_empty('node_edit_form');
  }

  if (!is_numeric($arg)) {
    return FALSE;
  }

  $node = node_load($arg);
  if (!$node) {
    return FALSE;
  }

  if (array_filter($conf['types']) && empty($conf['types'][$node->type])) {
    return FALSE;
  }

  // This will perform a node_access check, so we don't have to.
  return panels_context_create('node_edit_form', $node);
}

/**
 * Settings form for the argument
 */
function panels_node_edit_settings_form($conf) {
  $options = array();
  foreach (node_get_types() as $type => $info) {
    $options[$type] = $info->name;
  }
  $form['types'] = array(
    '#title' => t('Node types'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('You can restrict this argument to use the checked node types. Arguments from non-conforming node types will be ignored, and Panels will behave as if no argument were given. Leave all unchecked to impose no restriction.'),
    '#default_value' => $conf['types'],
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );

  $form['own_default'] = array(
    '#title' => t('Use different default display'),
    '#type' => 'checkbox',
    '#description' => t('If checked, when this argument is present it will use its own display rather than the default. Node types not selected in the "Own display" field will use this one.'),
    '#default_value' => $conf['own_default'],
  );

  $form['displays'] = array(
    '#title' => t('Own display'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $conf['displays'],
    '#description' => t('Each checked node type will get its own special display to layout its content. Only node types set above should be set here. Node types not set here will use the default display.'),
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * There appears to be a bit of a bug with the way we're handling forms; it causes
 * 'checkboxes' to get invalid values added to them when empty. This takes care
 * of that.
 */
function panels_node_edit_settings_form_submit(&$values) {
  $types = node_get_types();
  if (!empty($values['types'])) {
    foreach ($values['types'] as $type => $value) {
      if (empty($types[$type])) {
        unset($values['types'][$type]);
      }
    }
  }
  if (!empty($values['displays'])) {
    foreach ($values['displays'] as $type => $value) {
      if (empty($types[$type])) {
        unset($values['displays'][$type]);
      }
    }
  }
}

/**
 * What additional displays does this argument provide?
 */
function panels_node_edit_displays($conf, $id) {
  $displays = array();
  if (!empty($conf['own_default'])) {
    $displays['default'] = array(
      'title' => t('Node edit form @id Default', array('@id' => $id)),
      'context' => 'node',
    );
  }

  if (is_array($conf['displays'])) {
    $options = array();
    foreach (node_get_types() as $type => $info) {
      $options[$type] = $info->name;
    }
    foreach (array_keys(array_filter($conf['displays'])) as $type) {
      $displays[$type] = array(
        'title' => t('Node edit form @id @type', array('@id' => $id, '@type' => $options[$type])),
        // Tell it to base the template for this display off of the default.
        'default' => 'default',
        'context' => 'node',
      );
    }
  }

  return $displays;
}

/**
 * Based upon the settings and the context, choose which display to use.
 */
function panels_node_edit_choose_display($conf, $context) {
  if (empty($context->form)) {
    return;
  }

  if (!empty($conf['displays'][$context->node_type])) {
    return $context->node_type;
  }

  // Please note that 'default' is a special display.
  if (!empty($conf['own_default'])) {
    return 'default';
  }

  // Empty return says to use the default display.
  return;
}

