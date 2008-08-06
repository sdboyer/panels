<?php
// $Id$


/**
 * @file arguments/vid.inc
 *
 * Plugin to provide an argument handler for a vocabulary id
 */
function panels_vid_panels_arguments() {
  $args['vid'] = array(
    'title' => t("Vocabulary ID"),
    // keyword to use for %substitution
    'keyword' => 'vocabulary',
    'description' => t('Loads a vocabulary object from the argument.'),
    'context' => 'panels_vid_context',
    'settings form' => 'panels_vid_settings_form',
    'settings form submit' => 'panels_vid_settings_form_submit',
    'displays' => 'panels_vid_displays',
    'choose display' => 'panels_vid_choose_display',
  );
  return $args;
}

/**
 * Discover if this argument gives us the vocabulary we crave.
 */
function panels_vid_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return panels_context_create_empty('vocabulary');
  }

  if (!is_numeric($arg)) {
    return PANELS_BAD_ARG;
  }

  $vocabulary = taxonomy_get_vocabulary($arg);
  if (!$vocabulary) {
    return PANELS_BAD_ARG;
  }

  return panels_context_create('vocabulary', $vocabulary);
}

/**
 * Settings form for the argument
 */
function panels_vid_settings_form($conf) {
  $options = array();
  foreach (taxonomy_get_vocabularies() as $vid => $voc) {
    $options[$vid] = $voc->name;
  }

  $form['displays'] = array(
    '#title' => t('Own display'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $conf['displays'],
    '#description' => t('Each checked vocabulary will get its own special display to layout its content.'),
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
function panels_vid_settings_form_submit(&$values) {
  $vocs = taxonomy_get_vocabularies();
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
function panels_vid_displays($conf, $id) {
  $displays = array();

  if (is_array($conf['displays'])) {
    $options = array();
    foreach (taxonomy_get_vocabularies() as $vid => $info) {
      $options[$vid] = $info->name;
    }
    foreach (array_keys(array_filter($conf['displays'])) as $vid) {
      $displays[$vid] = array(
        'title' => t('vocabulary ID @id @type', array('@id' => $id, '@type' => $options[$vid])),
        // Tell it to base the template for this display off of the default.
        'default' => 'default',
        'context' => 'vocabulary',
      );
    }
  }

  return $displays;
}

/**
 * Based upon the settings and the context, choose which display to use.
 */
function panels_vid_choose_display($conf, $context) {
  if (empty($context->data)) {
    return;
  }

  if (!empty($conf['displays'][$context->data->vid])) {
    return $context->data->vid;
  }

  // Empty return says to use the default display.
  return;
}

