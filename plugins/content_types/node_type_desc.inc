<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_type_desc_panels_content_types() {
  $items['node_type_desc'] = array(
    'title' => t('Node type description'),
    'content_types' => 'panels_admin_content_types_node_type_desc',
    // only provides a single content type
    'single' => TRUE,
    'render callback' => 'panels_content_node_type_desc',
    'add callback' => 'panels_admin_edit_node_type_desc',
    'edit callback' => 'panels_admin_edit_node_type_desc',
    'title callback' => 'panels_admin_title_node_type_desc',
  );
  return $items;
}

/**
 * Output function for the 'node' content type. Outputs a node
 * based on the module and delta supplied in the configuration.
 */
function panels_content_node_type_desc($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'node_type';

  if ($node) {
    $type = node_get_types('type', $node);
    $block->subject = $type->name;
    $block->content = filter_xss_admin($type->description);
    $block->delta   = $node->type;
  }
  else {
    $block->subject = t('Node type description');
    $block->content = t('Node type description goes here.');
    $block->delta   = 'unknown';
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_type_desc() {
  return array(
    'node_type' => array(
      'title' => t('Node type description'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Node type description.'),
      'required context' => new ctools_context_required(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

function panels_admin_title_node_type_desc($subtype, $conf, $context) {
  return t('"@s" type description', array('@s' => $context->identifier));
}

