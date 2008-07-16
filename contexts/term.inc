<?php
// $Id$


/**
 * @file contexts/term.inc
 *
 * Plugin to provide a term context
 */

function panels_term_panels_contexts() {
  $args['term'] = array(
    'title' => t("Taxonomy term"),
    'description' => t('A single taxonomy term object.'),
    'context' => 'panels_context_create_term',
    'settings form' => 'panels_context_term_settings_form',
    'settings form validate' => 'panels_context_term_settings_form_validate',
    'keyword' => 'term',
    'no ui' => TRUE,
    'context name' => 'term',
  );
  return $args;
}

/**
 * It's important to remember that $conf is optional here, because contexts
 * are not always created from the UI.
 */
function panels_context_create_term($empty, $data = NULL, $conf = FALSE) {
  $context = new panels_context('term');
  $context->plugin = 'term';

  if ($empty) {
    return $context;
  }

  if ($conf) {
    $data = taxonomy_get_term($data['tid']);
  }

  if (!empty($data)) {
    $context->data     = $data;
    $context->title    = $data->name;
    $context->argument = $data->tid;
    return $context;
  }
}

function panels_context_term_settings_form($conf, $external = FALSE) {
  $form = array();
  if ($external) {
    $form['external'] = array(
      '#type' => 'checkbox',
      '#default_value' => $conf['external'],
      '#title' => t('Require this context from an external source (such as a containing panel page).'),
      '#description' => t('If selected, term selection (below) will be ignored'),
    );
  }
  return $form;
}

/**
 * Validate a term.
 */
function panels_context_term_settings_form_validate($form, $form_values) {
}
