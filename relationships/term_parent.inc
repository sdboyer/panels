<?php
// $Id$


/**
 * @file relationships/term_parent.inc
 *
 * Plugin to provide an relationship handler for term parent
 */

function panels_term_parent_panels_relationships() {
  $args['term_parent'] = array(
    'title' => t("Term parent"),
    'keyword' => 'parent_term',
    'description' => t('Adds a taxonomy term parent from a term context.'),
    'required context' => new panels_required_context(t('Term'), 'term'),
    'context' => 'panels_term_parent_context',
    'settings form' => 'panels_term_parent_settings_form',
    'settings form validate' => 'panels_term_parent_settings_form_validate',
  );
  return $args;
}

/**
 * Return a new context based on an existing context
 */
function panels_term_parent_context($context = NULL, $conf) {
  // If unset it wants a generic, unfilled context, which is just NULL
  if (empty($context->data)) {
    return panels_context_create_empty('term');
  }

  if (isset($context->data)) {
    $result = db_fetch_array(db_query("SELECT t1.* FROM {term_hierarchy} t1 INNER JOIN {term_hierarchy} t2 ON t1.tid = t2.parent WHERE t2.tid = %d", $context->data->tid));

    // If top level term, keep looking up until we see a top level.
    if ($conf['type'] == 'top') {
      // If looking for top level, and there are no parents at all, make sure
      // the current term is the 'top level'.
      if (empty($result)) {
        $result['tid'] = $context->data->tid;
      }
      while (!empty($result['parent'])) {
        $result = db_fetch_array(db_query("SELECT * FROM {term_hierarchy} WHERE tid = %d", $result['parent']));
      }
    }

    // load the term
    if ($result) {
      $term = taxonomy_get_term($result['tid']);
      return panels_context_create('term', $term);
    }
  }
}

/**
 * Settings form for the relationship
 */
function panels_term_parent_settings_form($conf) {
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Relationship type'),
    '#options' => array('parent' => t('Immediate parent'), 'top' => t('Top level term')),
    '#default_value' => $conf['type'],
  );

  return $form;
}

