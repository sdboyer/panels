<?php
// $Id$

/**
 * @file
 * Contains helper tasks for the delegator toolset to make it easy to apply
 * panel task handlers to a variety of tasks. This contains the common
 * API that is used regardless of what the actual content of the page
 * is meant to be.
 */

/**
 * Callback to allow the handler to react to being saved.
 */
function panels_delegator_save(&$handler, $update) {
  // @todo Save the display

  // Re-cache the CSS.
  // Delete any previous cache file.
  if (!empty($handler->conf['css_cache'])) {
    file_delete($handler->conf['css_cache']);
  }

  if (!empty($handler->conf['css'])) {
    // And generate a new one.
    ctools_include('css');
    $handler->conf['css_cache'] = ctools_css_cache($handler->conf['css']);
  }
  else {
    $handler->conf['css_cache'] = '';
  }
}



/**
 * Choose a layout for this panel.
 */
function panels_delegator_edit_choose(&$form, &$form_state) {
  panels_load_include('common');
  panels_load_include('display-layout');
  panels_load_include('plugins');

  // @todo -- figure out where/how to deal with this.
  $form_state['allowed_layouts'] = NULL;

  // Create a dummy display. We don't have one yet, since we haven't chosen
  // its layout. But we are about to!
  $form_state['display'] = panels_new_display();

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';
  $form = array_merge($form, panels_choose_layout($form_state));
}

/**
 * Validate that a layout was chosen.
 */
function panels_delegator_edit_choose_validate(&$form, &$form_state) {
  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_delegator_edit_choose_submit(&$form, &$form_state) {
  $form_state['display']->layout = $form_state['values']['layout'];
  $form_state['handler']->conf['display'] = $form_state['display'];
  // The regular submit function will automatically cache the display here.
}

/**
 * Change the layout for this panel.
 */
function panels_delegator_edit_layout(&$form, &$form_state) {
  panels_load_include('common');
  panels_load_include('display-layout');
  panels_load_include('plugins');

  // @todo -- figure out where/how to deal with this.
  $form_state['allowed_layouts'] = NULL;

  $form_state['display'] = &$form_state['handler']->conf['display'];

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';
  $form = array_merge($form, panels_choose_layout($form_state));
}

/**
 * Validate that a layout was chosen.
 */
function panels_delegator_edit_layout_validate(&$form, &$form_state) {
  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_delegator_edit_layout_submit(&$form, &$form_state) {
  $form_state['handler']->conf['temp_layout'] = $form_state['values']['layout'];
  // The regular submit function will automatically cache the display here.
}

/**
 * When a layout is changed, the user is given the opportunity to move content.
 */
function panels_delegator_edit_move(&$form, &$form_state) {
  $form_state['display'] = &$form_state['handler']->conf['display'];
  $form_state['layout'] = $form_state['handler']->conf['temp_layout'];

  panels_load_include('common');
  panels_load_include('display-layout');
  panels_load_include('plugins');

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form = array_merge($form, panels_change_layout($form_state));

  // Change the 'back' button to just go directly to the previous form
  $task_id = $form_state['task']['name'];
  $handler_id = $form_state['handler']->handler;
  $name = $form_state['handler']->name;

  $form['back']['#next'] = "admin/build/delegator/$task_id/$handler_id/$name/layout";
  $form['back']['#submit'] = array('delegator_admin_edit_task_handler_cancel');
}

/**
 * Proxy the layout settings form.
 */
function panels_delegator_edit_layout_settings(&$form, &$form_state) {
  panels_load_include('ajax');
  panels_load_include('display-layout-settings');
  panels_load_include('plugins');

  $form_state['display'] = &$form_state['handler']->conf['display'];
//  $form_state['display']->context = panels_delegator_get_context($form_state['plugin'], $form_state['handler']);

  // Allow setting of the 'display title'
  $form_state['display_title'] = TRUE;

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Prevent notices
  $form_state['finish'] = '';

  $form = array_merge($form, panels_edit_layout_settings_form($form_state));

  // Fix up the extra Save button this page needs. Our needs are a little
  // different than the normal Panels needs.

  // Sometimes there is a next button, sometimes there is a return depending
  // on how we are visited.
  $button = isset($form['buttons']['return']) ? $form['buttons']['return'] : $form['buttons']['next'];

  $form['save']['#next'] = $_GET['q'];
  $form['save']['#value'] = t('Save and edit again');
  $form['save']['#submit'] = $button['#submit'];
  $form['save']['#validate'] = $button['#validate'];
  $form['save']['#weight'] = -1;

  // Move the button into the 'buttons' category so it lines up nicely.
  $form['buttons']['save'] = $form['save'];
  unset($form['save']);
}

/**
 * Get the contexts associated with a delegator task handler using panels.
 *
 * @todo -- this probably needs more information to be truly flexible.
 */
function panels_delegator_get_context($plugin, $handler, $base_only = FALSE) {
  if ($function = ctools_plugin_get_function($plugin, 'get context')) {
    $contexts = $function();
  }
  else {
    $contexts = array();
  }

  $object = $base_only ? new stdClass : (object) $handler->conf;

  // This function expects the contexts to be on an object, but our conf
  // is an array, so we have to convert.

  return panels_context_load_contexts($object, TRUE, $contexts);
}

/**
 * Present the panels drag & drop editor to edit the display attached
 * to the task handler.
 */
function panels_delegator_edit_content(&$form, &$form_state) {
  panels_load_include('ajax');
  panels_load_include('plugins');
  panels_load_include('common');

  $display = &$form_state['handler']->conf['display'];

  $display->context = panels_delegator_get_context($form_state['plugin'], $form_state['handler']);
  $content_types = panels_common_get_allowed_types('delegator', $display->context);

  // Load the display being edited from cache, if possible.
  if (!empty($_POST) && is_object($cache = panels_cache_get('display', $display->did))) {
    $display = $cache->display;
  }
  else {
    panels_cache_clear('display', $display->did);
    $cache = new stdClass();
    $cache->display = $display;
    $cache->content_types = $content_types;
    panels_cache_set('display', $display->did, $cache);
  }

  $form_state['display'] = &$display;

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  $form = array_merge($form, panels_edit_display_form($form_state));
  // Make sure the theme will work since our form id is different.
  $form['#theme'] = 'panels_edit_display_form';
}

/**
 * General settings for the panel
 */
function panels_delegator_edit_settings(&$form, &$form_state) {
  $conf = $form_state['handler']->conf;
  $form['conf']['no_blocks'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['no_blocks'],
    '#title' => t('Disable Drupal blocks/regions'),
    '#description' => t('Check this to have the panel page disable all regions displayed in the theme.'),
  );

  $form['conf']['css_id'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $conf['css_id'],
    '#title' => t('CSS ID'),
    '#description' => t('The CSS ID to apply to this page'),
  );

  $form['conf']['css'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#description' => t('Enter well-formed CSS code here; this code will be embedded into the page, and should only be used for minor adjustments; it is usually better to try to put CSS for the page into the theme if possible. This CSS will be filtered for safety so some CSS may not work.'),
    '#default_value' => $conf['css'],
  );
}

/**
 * Edit contexts that go with this panel.
 */
function panels_delegator_edit_context(&$form, &$form_state) {
  panels_load_include('common-context');
  panels_load_include('ajax');
  panels_load_include('plugins');

  $handler = $form_state['handler'];
  // Check our context object cache.
  if (!empty($_POST)) {
    $cache = panels_cache_get('panel_object:delegator', $handler->name);
  }
  else {
    $cache = new stdClass;
    $cache->name = $handler->name;
    $cache->base_contexts = panels_delegator_get_context($form_state['plugin'], $form_state['handler'], TRUE);
    $cache->contexts = isset($handler->conf['contexts']) ? $handler->conf['contexts'] : array();
    $cache->relationships = isset($handler->conf['relationships']) ? $handler->conf['relationships'] : array();
    panels_cache_set('panel_object:delegator', $handler->name, $cache);
  }

  drupal_add_css(panels_get_path('css/panels_admin.css'));

  $form['right'] = array(
    '#prefix' => '<div class="clear-block"><div class="right-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="left-container">',
    '#suffix' => '</div></div>',
  );

  // Load proper contexts so your relationships can work from the right base.
  $settings = panels_common_add_context_form('delegator', $form, $form_state, $form['right']['contexts_table'], $cache);
  $settings += panels_common_add_relationship_form('delegator', $form, $form_state, $form['right']['relationships_table'], $cache);
  panels_common_add_context_js($settings);

  $form['left']['summary'] = array(
    '#prefix' => '<div class="delegator-contexts">',
    '#suffix' => '</div>',
    '#value' => theme('panels_common_context_list', $cache, t('Summary of contexts')),
  );

  drupal_add_css(panels_get_path('css/panels_admin.css'));
  $form_state['cache'] = &$cache;
}

/**
 * Process submission of the context edit form.
 */
function panels_delegator_edit_context_submit(&$form, &$form_state) {
  $form_state['handler']->conf['contexts'] = $form_state['cache']->contexts;
  $form_state['handler']->conf['relationships'] = $form_state['cache']->relationships;
}
