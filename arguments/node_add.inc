<?php
// $Id$


/**
 * @file arguments/nid.inc
 *
 * Plugin to provide an argument handler for a Node add form
 */
function panels_node_add_panels_arguments() {
  $args['node_add'] = array(
    'title' => t("Node add form"),
    // keyword to use for %substitution
    'keyword' => 'node_type',
    'description' => t('Displays the node add form for a content type.'),
    'context' => 'panels_node_add_context',
    'settings form' => 'panels_node_add_settings_form',
    'settings form submit' => 'panels_node_add_settings_form_submit',
    'displays' => 'panels_node_add_displays',
    'choose display' => 'panels_node_add_choose_display',
    'menu builder' => 'panels_node_add_menu_builder',
  );
  return $args;
}

/**
 * Discover if this argument gives us the node we crave.
 */
function panels_node_add_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if (!isset($arg)) {
    return panels_context_create_empty('node_add_form');
  }

  if (array_filter($conf['types']) && empty($conf['types'][$arg])) {
    return PANELS_ARG_USE_FALLBACK;
  }

  return panels_context_create('node_add_form', $arg);
}

/**
 * Settings form for the argument
 */
function panels_node_add_settings_form($conf) {
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
function panels_node_add_settings_form_submit(&$values) {
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
 * Based upon the settings and the context, choose which display to use.
 */
function panels_node_add_choose_display($conf, $context) {
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

/**
 * What additional displays does this argument provide?
 */
function panels_node_add_displays($conf, $id) {
  $displays = array();
  if (!empty($conf['own_default'])) {
    $displays['default'] = array(
      'title' => t('Node add form @id Default', array('@id' => $id)),
      'context' => 'node',
    );
  }

  if (is_array($conf['displays'])) {
    $options = array();
    foreach (node_get_types() as $type => $info) {
      $options[$type] = $info->name;
    }
    foreach (array_keys(array_filter($conf['displays'])) as $type) {
      if (!empty($options[$type])) {
        $displays[$type] = array(
          'title' => t('Node add form @id @type', array('@id' => $id, '@type' => $options[$type])),
          // Tell it to base the template for this display off of the default.
          'default' => 'default',
          'context' => 'node',
        );
      }
    }
  }

  return $displays;
}

function panels_node_add_menu_builder(&$items, $metadata) {
  // Assume that this is the only argument present, as a use case where that's
  // not true is simply beyond the pale
  list($path_prefix, $path_suffix) = explode('%', $metadata->path);

  $types = array_filter($metadata->arg_data['argument_settings']['types']);
  $types = array_keys(empty($types) ? $metadata->arg_data['argument_settings']['types'] : $types);

  foreach (node_get_types('types', NULL, TRUE) as $type) {
    if (in_array($type->type, $types)) {
      $type_url_str = str_replace('_', '-', $type->type);
      $items[$path_prefix . $type_url_str . $path_suffix] = array(
        'title' => drupal_ucfirst($type->name),
        'title callback' => 'check_plain',
        'page callback' => 'node_add',
        'page arguments' => array(2),
        'access callback' => 'node_access',
        'access arguments' => array('create', $type->type),
        'description' => $type->description,
        'file' => 'node.pages.inc',
      );
    }
  }
  // Indicate that the original path should be scrubbed
  return TRUE;
}
