<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_vocabulary_terms_panels_content_types() {
  if (module_exists('taxonomy')) {
    $items['vocabulary_terms'] = array(
      'title' => t('Term description'),
      'content_types' => 'panels_admin_content_types_vocabulary_terms',
      'single' => TRUE,
      'render callback' => 'panels_content_vocabulary_terms',
      'add callback' => 'panels_admin_edit_vocabulary_terms',
      'edit callback' => 'panels_admin_edit_vocabulary_terms',
      'title callback' => 'panels_admin_title_vocabulary_terms',
    );
    return $items;
  }
}

/**
 * Output function for the 'vocabulary terms' content type. Outputs a
 * list of terms for the input vocabulary.
 */
function panels_content_vocabulary_terms($subtype, $conf, $panel_args, $context) {
  $vocab = isset($context->data) ? drupal_clone($context->data) : NULL;
  $max_depth = (!empty($conf['max_depth']) ? (int)$conf['max_depth'] : NULL);
  if ($conf['tree'] == FALSE) {
    $terms = taxonomy_get_tree($vocab->vid, 0, -1, $max_depth);
    $items = array();
    foreach ($terms as $term) {
      $items[] = l($term->name, 'taxonomy/term/'. $term->tid);
    }
    $output = theme('item_list', $items);
  }
  else {
    $output = theme('item_list', _panels_content_vocabulary_terms($vocab->vid, $max_depth));
  }

  $block = new stdClass();
  $block->module  = 'node_type';
  $block->subject = $vocab->name;
  $block->content = $output;
  $block->delta   = $vocab->tid;

  return $block;
}

function _panels_content_vocabulary_terms($vid, $max_depth, $depth = -1, $tid = 0) {
  $depth++;
  if ($max_depth != NULL && $depth == $max_depth) {
    return array();
  }
  $return = array();
  $query = db_query('SELECT t.name, t.tid FROM {term_data} t INNER JOIN {term_hierarchy} h ON t.tid = h.tid WHERE t.vid = %d AND h.parent = %d ORDER BY t.weight ASC, t.name ASC', $vid, $tid);
  while ($result = db_fetch_object($query)) {
    $return[] = array(
      'data' => l($result->name, 'taxonomy/term/'. $result->tid),
      'children' => _panels_content_vocabulary_terms($vid, $max_depth, $depth, $result->tid),
    );
  }
  return $return;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_vocabulary_terms() {
  return array(
    'description' => array(
      'title' => t('Vocabulary terms'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('All the terms in a vocabulary.'),
      'required context' => new panels_required_context(t('Vocabulary'), 'vocabulary'),
      'category' => array(t('Vocabulary context'), -9),
    ),
  );
}

function panels_admin_title_vocabulary_terms($subtype, $conf, $context) {
  return t('"@s" terms', array('@s' => $context->identifier));
}

function panels_admin_edit_vocabulary_terms($id, $parents, $conf = array()) {
  // Apply defaults
  if (empty($conf)) {
    $conf = array('max_depth' => 0, 'tree' => 1);
  }

  $form['max_depth'] = array(
    '#type' => 'select',
    '#title' => t('Maximum depth'),
    '#options' => array_merge(array(t('unlimited')), range(1, 9)),
    '#default_value' => $conf['max_depth'],
    '#description' => t('Define the maximum depth of terms being displayed.'),
  );

  $form['tree'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display as tree'),
    '#default_value' => $conf['tree'],
    '#description' => t('If checked, the terms are displayed in a tree, otherwise in a flat list.'),
  );

  return $form;
}

