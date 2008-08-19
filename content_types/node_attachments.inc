<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_attachments_panels_content_types() {
  $items['node_attachments'] = array(
    'title' => t('Node type description'),
    // only provides a single content type
    'single' => TRUE,
    'content_types' => 'panels_admin_content_types_node_attachments',
    'render callback' => 'panels_content_node_attachments',
//      'add callback' => 'panels_admin_edit_node_attachments',
//      'edit callback' => 'panels_admin_edit_node_attachments',
    'title callback' => 'panels_admin_title_node_attachments',
  );
  return $items;
}

function panels_content_node_attachments($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'attachments';

  $block->subject = t('Attached files');
  if ($node) {
    $block->content = theme('upload_attachments', $node->files);
    $block->delta = $node->nid;
  }
  else {
    $block->content = t('Attached files go here.');
    $block->delta = 'unknown';
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_attachments() {
  return array(
    'node_type' => array(
      'title' => t('Attached files'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('A list of files attached to the node.'),
      'required context' => new panels_required_context(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

function panels_admin_title_node_attachments($conf, $context) {
  return t('"@s" attachments', array('@s' => $context->identifier));
}

