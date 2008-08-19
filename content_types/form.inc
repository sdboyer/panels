<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_form_panels_content_types() {
  $items['form'] = array(
    'title' => t('Generic form'),
    'content_types' => 'panels_admin_content_types_form',
    // only provides a single content type
    'single' => TRUE,
    'render callback' => 'panels_content_form',
    'add callback' => 'panels_admin_edit_form',
    'edit callback' => 'panels_admin_edit_form',
    'title callback' => 'panels_admin_title_form',
    'render last' => TRUE,
  );
  return $items;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_form() {
  return array(
    'node_type' => array(
      'title' => t('General form'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('The general parts of a form.'),
      'required context' => new panels_required_context(t('Form'), 'form'),
      'category' => array(t('Form'), -9),
    ),
  );
}

/**
 * Output function for the 'node' content type. Outputs a node
 * based on the module and delta supplied in the configuration.
 */
function panels_content_form($subtype, $conf, $panel_args, &$context) {
  $block = new stdClass();
  $block->module = 'form';

  if (isset($context->form)) {
    $block->subject = $context->form_title;
    if (!empty($context->form_id)) {
      // If this is a form, drupal_render it.
      $block->content = drupal_render($context->form);
    }
    else {
      // Otherwise just spit back what we were given. This is probably an
      // error message or something.
      $block->content = $context->form;
    }
    $block->delta = $context->form_id;
  }
  else {
    $block->subject = t('Form');
    $block->content = t('Form goes here.');
    $block->delta   = 'unknown';
  }

  return $block;
}

function panels_admin_title_form($conf, $context) {
  return t('"@s" base form', array('@s' => $context->identifier));
}

