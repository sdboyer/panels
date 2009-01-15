<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_term_description_panels_content_types() {
  $items['term_description'] = array(
    'title' => t('Term description'),
    'content_types' => 'panels_admin_content_types_term_description',
    'single' => TRUE,
    'render callback' => 'panels_content_term_description',
    'add callback' => 'panels_admin_edit_term_description',
    'edit callback' => 'panels_admin_edit_term_description',
    'title callback' => 'panels_admin_title_term_description',
  );
  return $items;
}

function panels_content_term_description($subtype, $conf, $panel_args, $context) {
  $term = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'node_type';

  $block->subject = $term->name;
  if ($term) {
    $block->content = _filter_autop(filter_xss_admin($term->description));
    $block->delta = $term->tid;

    if (user_access('administer taxonomy')) {
      $block->admin_links['update'] = array(
        'title' => t('Edit term'),
        'alt' => t("Edit this term"),
        'href' => "admin/content/taxonomy/edit/term/$term->tid",
        'query' => drupal_get_destination(),
      );
    }
  }
  else {
    $block->content = t('Term description goes here.');
    $block->delta = 'unknown';
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_term_description() {
  return array(
    'description' => array(
      'title' => t('Term description'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('plugins/content_types/node'),
      'description' => t('Term description.'),
      'required context' => new ctools_context_required(t('Term'), 'term'),
      'category' => array(t('Term context'), -9),
    ),
  );
}

function panels_admin_title_term_description($subtype, $conf, $context) {
  return t('"@s" term description', array('@s' => $context->identifier));
}

