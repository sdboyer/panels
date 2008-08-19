<?php
// $Id$


/**
 * @file arguments/term.inc
 *
 * Plugin to provide an argument handler for a Taxonomy term
 */
function panels_term_panels_arguments() {
  $args['term'] = array(
    'title' => t("Taxonomy term"),
    // keyword to use for %substitution
    'keyword' => 'term',
    'description' => t('Restricts the argument to a taxonomy term.'),
    'context' => 'panels_term_context',
    'settings form' => 'panels_term_settings_form',
    'settings form submit' => 'panels_term_settings_form_submit',
    'title function' => 'panels_term_title',
    'displays' => 'panels_term_displays',
    'choose display' => 'panels_term_choose_display',
  );
  return $args;
}

/**
 * Discover if this argument gives us the term we crave.
 */
function panels_term_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return panels_context_create_empty('term');
  }

  switch ($conf['input_form']) {
    case 'tid':
    default:
      if (!is_numeric($arg)) {
        return FALSE;
      }
      $term = taxonomy_get_term($arg);
      break;

    case 'term':
      $terms = taxonomy_get_term_by_name($arg);
      if (count($terms) != 1) {
        foreach ($terms as $potential) {
          foreach ($conf['vids'] as $vid => $active) {
            if ($active == 1 && $potential->vid == $vid) {
              $term = $potential;
              // break out of the foreaches AND the case
              break 3;
            }
          }
        }
      }
      $term = array_shift($terms);
      break;
  }

  if (empty($term)) {
    return PANELS_ARG_IS_BAD;
  }

  if (!empty($conf['vids']) && array_filter($conf['vids']) && empty($conf['vids'][$term->vid])) {
    return PANELS_ARG_USE_FALLBACK;
  }

  return panels_context_create('term', $term);
}

/**
 * Settings form for the argument
 */
function panels_term_settings_form($conf) {
  if (empty($conf)) {
    $conf = array(
      'input_form' => 'tid',
    );
  }

  $options = array();
  foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
    $options[$vid] = $vocabulary->name;
  }

  $form['input_form'] = array(
    '#title' => t('Argument type'),
    '#type' => 'radios',
    '#options' => array('tid' => t('Term ID'), 'term' => t('Term name')),
    '#default_value' => $conf['input_form'],
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );


  $form['vids'] = array(
    '#title' => t('Vocabularies'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#description' => t('You can restrict this argument to use the checked vocabularies. Arguments from non-conforming vocabularies will be ignored, and Panels will behave as if no argument were given. Leave all unchecked to impose no restriction.'),
    '#default_value' => $conf['vids'],
    '#prefix' => '<div class="clear-block">',
    '#suffix' => '</div>',
  );

  $form['own_default'] = array(
    '#title' => t('Use different default display'),
    '#type' => 'checkbox',
    '#description' => t('If checked, when this argument is present it will use its own display rather than the default. Vocabularies not selected in the "Own display" field will use this one.'),
    '#default_value' => $conf['own_default'],
  );

  $form['displays'] = array(
    '#title' => t('Own display'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $conf['displays'],
    '#description' => t('Each checked vocabulary will get its own special display to layout its content. Only vocabularies set above should be set here. Vocabularies not set here will use the default display.'),
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
function panels_term_settings_form_submit(&$values) {
  $vocs = taxonomy_get_vocabularies();
  if (!empty($values['vids'])) {
    foreach ($values['vids'] as $vid => $value) {
      if (empty($vocs[$vid])) {
        unset($values['vids'][$vid]);
      }
    }
  }
  if (!empty($values['displays'])) {
    foreach ($values['displays'] as $vid => $value) {
      if (empty($vocs[$vid])) {
        unset($values['displays'][$vid]);
      }
    }
  }
}

/**
 * What additional displays does this argument provide?
 */
function panels_term_displays($conf, $id) {
  $displays = array();
  if (!empty($conf['own_default'])) {
    $displays['default'] = array(
      'title' => t('Taxonomy term @id Default', array('@id' => $id)),
      'context' => 'term',
    );
  }

  if (is_array($conf['displays'])) {
    $options = array();
    foreach (taxonomy_get_vocabularies() as $vid => $vocabulary) {
      $options[$vid] = $vocabulary->name;
    }
    foreach (array_keys(array_filter($conf['displays'])) as $vid) {
      $displays[$vid] = array(
        'title' => t('Taxonomy term @id @vocabulary', array('@id' => $id, '@vocabulary' => $options[$vid])),
        // Tell it to base the template for this display off of the default.
        'default' => 'default',
        'context' => 'term',
      );
    }
  }

  return $displays;
}

/**
 * Based upon the settings and the context, choose which display to use.
 */
function panels_term_choose_display($conf, $context) {
  if (empty($context->data)) {
    return;
  }

  if (!empty($conf['displays'][$context->data->vid])) {
    return $context->data->vid;
  }

  // Please note that 'default' is a special display.
  if (!empty($conf['own_default'])) {
    return 'default';
  }

  // Empty return says to use the default display.
  return;
}

/**
 * Determine the title for substitution with this context
 */
function panels_term_title($context) {
  if (isset($context->data->name)) {
    return $context->data->name;
  }

  if (isset($context->page_title)) {
    return $context->page_title;
  }
}

