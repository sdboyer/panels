<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_term_list_panels_content_types() {
  $items['term_list'] = array(
    'title' => t('List of related terms'),
    'content_types' => 'panels_admin_content_types_term_list',
    'single' => TRUE,
    'render callback' => 'panels_content_term_list',
    'add callback' => 'panels_admin_edit_term_list',
    'edit callback' => 'panels_admin_edit_term_list',
    'title callback' => 'panels_admin_title_term_list',
  );
  return $items;
}

function panels_content_term_list($subtype, $conf, $panel_args, $context) {
  $term = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'term-list';

  $options = panels_admin_term_list_options();
  if ($term) {
    $block->subject = $options[$conf['type']];
    $block->delta = $conf['type'];
    switch ($conf['type']) {
      case 'related':
        $terms = taxonomy_get_related($term->tid);
        break;

      case 'child':
      default:
        $terms = taxonomy_get_children($term->tid);
        break;

      case 'top':
        $terms = taxonomy_get_children(0, $term->vid);
        break;

      case 'sibling':
        $parent = db_result(db_query("SELECT parent FROM {term_hierarchy} WHERE tid = %d", $term->tid));
        $terms = taxonomy_get_children($parent, $term->vid);
        // Remove the term that started this.
        unset($terms[$term->tid]);
        break;

      case 'synonyms':
        $terms = taxonomy_get_synonyms($term->tid);
        break;
    }
    if ($terms) {
      foreach ($terms as $related) {
        $items[$related->tid] = l($related->name, taxonomy_term_path($related), array('rel' => 'tag', 'title' => strip_tags($related->description)));
      }

      $block->content = theme('item_list', $items, NULL, $conf['list_type']);
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
function panels_admin_content_types_term_list() {
  return array(
    'description' => array(
      'title' => t('List of related terms'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Terms related to an existing term; may be child, siblings or top level.'),
      'required context' => new ctools_context_required(t('Term'), 'term'),
      'category' => array(t('Term context'), -9),
    ),
  );
}

function panels_admin_term_list_options() {
  return array(
    'child' => t('Child terms'),
    'related' => t('Related terms'),
    'sibling' => t('Sibling terms'),
    'top' => t('Top level terms'),
    'synonyms' => t('Term synonyms'),
  );
}

/**
 * Returns an edit form for the custom type.
 */
function panels_admin_edit_term_list($id, $parents, $conf = array()) {
  // Apply defaults
  if (empty($conf)) {
    $conf = array('title' => '', 'type' => 'child', 'list_type' => 'ul');
  }

  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Which terms'),
    '#options' => panels_admin_term_list_options(),
    '#default_value' => $conf['type'],
    '#prefix' => '<div class="clear-block no-float">',
    '#suffix' => '</div>',
  );

  $form['list_type'] = array(
    '#type' => 'select',
    '#title' => t('List type'),
    '#options' => array('ul' => t('Unordered'), 'ol' => t('Ordered')),
    '#default_value' => $conf['list_type'],
  );

  return $form;
}

function panels_admin_title_term_list($subtype, $conf, $context) {
  $options = panels_admin_term_list_options();
  return t('"@s" @type', array('@s' => $context->identifier, '@type' => drupal_strtolower($options[$conf['type']])));
}

