<?php
// $Id$


/**
 * @file arguments/nid.inc
 *
 * Plugin to provide an argument handler for a node id
 */
function panels_nid_panels_arguments() {
  $args['nid'] = array(
    'title' => t("Node ID"),
    // keyword to use for %substitution
    'keyword' => 'node',
    'description' => t('Restricts the argument to a node id.'),
    'context' => 'panels_nid_context',
    'settings form' => 'panels_nid_settings_form',
    'settings form submit' => 'panels_nid_settings_form_submit',
    'displays' => 'panels_nid_displays',
    'choose display' => 'panels_nid_choose_display',
    'native path' => 'node/%node',
    'load function' => 'node',
  );
  return $args;
}

/**
 * Discover if this argument gives us the node we crave.
 */
function panels_nid_context($node = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return panels_context_create_empty('node');
  }

  if (!is_object($node)) {
    return PANELS_ARG_IS_BAD;
  }

  if (array_filter($conf['types']) && empty($conf['types'][$node->type])) {
    return PANELS_ARG_USE_FALLBACK;
  }

  return panels_context_create('node', $node);
}

/**
 * Settings form for the argument
 */
function panels_nid_settings_form($conf) {
  if (empty($conf)) {
    $conf = array(
      'types' => array(),
      'own_default' => FALSE,
      'displays' => array(),
    );
  }

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
function panels_nid_settings_form_submit(&$values) {
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
function panels_nid_displays($conf, $id) {
  $displays = array();
  if (!empty($conf['own_default'])) {
    $displays['default'] = array(
      'title' => t('Node ID @id Default', array('@id' => $id)),
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
        'title' => t('Node ID @id @type', array('@id' => $id, '@type' => $options[$type])),
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
function panels_nid_choose_display($conf, $context) {
  if (empty($context->data)) {
    return;
  }

  if (!empty($conf['displays'][$context->data->type])) {
    return $context->data->type;
  }

  // Please note that 'default' is a special display.
  if (!empty($conf['own_default'])) {
    return 'default';
  }

  // Empty return says to use the default display.
  return;
}

