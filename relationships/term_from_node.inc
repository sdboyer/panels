<?php
// $Id$


/**
 * @file relationships/term_from_node.inc
 *
 * Plugin to provide an relationship handler for term from node
 */

function panels_term_from_node_panels_relationships() {
  $args['term_from_node'] = array(
    'title' => t("Term from node"),
    'keyword' => 'term',
    'description' => t('Adds a taxonomy term from a node context; if multiple terms are selected, this will get the "first" term only.'),
    'required context' => new panels_required_context(t('Node'), 'node'),
    'context' => 'panels_term_from_node_context',
    'settings form' => 'panels_term_from_node_settings_form',
    'settings form validate' => 'panels_term_from_node_settings_form_validate',
  );
  return $args;
}

/**
 * Return a new context based on an existing context
 */
function panels_term_from_node_context($context = NULL, $conf) {
  // If unset it wants a generic, unfilled context, which is just NULL
  if (empty($context->data)) {
    return panels_context_create_empty('term', NULL);
  }

  if (isset($context->data->taxonomy)) {
    foreach ($context->data->taxonomy as $term) {
      if ($term->vid == $conf['vid']) {
        return panels_context_create('term', $term);
      }
    }
  }
}

/**
 * Settings form for the relationship
 */
function panels_term_from_node_settings_form($conf) {
  $options = array();
  foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
    $options[$vid] = $vocabulary->name;
  }
  $form['vid'] = array(
    '#title' => t('Vocabulary'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $conf['vid'],
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );

  return $form;
}

