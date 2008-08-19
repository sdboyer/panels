<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_book_nav_panels_content_types() {
  if (module_exists('book')) {
    $items['node_book_nav'] = array(
      'title' => t('Node book navigation'),
      'single' => TRUE,
      'content_types' => 'panels_admin_content_types_node_book_nav',
      'render callback' => 'panels_content_node_book_nav',
      'title callback' => 'panels_admin_title_node_book_nav',
    );
    return $items;
  }
}

function panels_content_node_book_nav($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'book_nav';

  $block->subject = t('Book navigation');
  if ($node) {
    $block->content = theme('book_navigation', $node);
    $block->delta = $node->nid;
  }
  else {
    $block->content = t('Book navigation goes here.');
    $block->delta = 'unknown';
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_book_nav() {
  return array(
    'node_type' => array(
      'title' => t('Book navigation'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('A list of files attached to the node.'),
      'required context' => new panels_required_context(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

function panels_admin_title_node_book_nav($conf, $context) {
  return t('"@s" book navigation', array('@s' => $context->identifier));
}

