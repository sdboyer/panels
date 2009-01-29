<?php
// $Id$

/**
 * @file
 *
 * This is the task handler plugin to handle attaching a panel to any
 * task that advertises itself as a 'context' type, which all of the
 * basic page tasks provided by delegator.module do by default.
 *
 * Most of our config is handled by panels delegator.inc, but we do have
 * to customize it a little bit to match what we're doing with this task.
 */

/**
 * Specialized implementation of hook_delegator_task_handlers().
 */
function panels_panel_context_delegator_task_handlers() {
  return array(
    'handler type' => 'context', // is a 'context' handler type
    'title' => t('Panel'),
    'admin summary' =>'panels_panel_context_admin_summary',
    'admin title' => 'panels_panel_context_title',
    'render' => 'panels_panel_context_render',
    'save' => 'panels_panel_context_save',
    'export' => 'panels_panel_context_export',

    'add forms' => array(
      'criteria' => t('Selection criteria'),
      'choose' => t('Choose layout'),
      'layout-settings' => t('Configure layout'),
      'content' => t('Panel content'),
    ),
    'edit forms' => array(
      'content' => t('Panel content'),
      'criteria' => t('Selection criteria'),
      'settings' => t('General'),
      'context' => t('Contexts'),
      'layout' => t('Change layout'),
      'move' => '', // no title makes it a 'hidden' edit form.
      'layout-settings' => t('Layout settings'),
    ),
    'forms' => array(
      'settings' => array(
        'form id' => 'panels_panel_context_edit_settings',
      ),
      'choose' => array(
        'form id' => 'panels_panel_context_edit_choose',
      ),
      'layout' => array(
        'no return' => TRUE,
        'form id' => 'panels_panel_context_edit_layout',
      ),
      'move' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/includes/display-layout.inc',
        ),
        'form id' => 'panels_panel_context_edit_move',
        'submit' => 'panels_change_layout_submit',
      ),
      'layout-settings' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/includes/display-layout-settings.inc',
        ),
        'form id' => 'panels_panel_context_edit_layout_settings',
        'validate' => 'panels_edit_layout_settings_form_validate',
        'submit' => 'panels_edit_layout_settings_form_submit',
      ),
      'content' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/includes/display-edit.inc',
        ),
        'form id' => 'panels_panel_context_edit_content',
        'submit' => 'panels_edit_display_form_submit',
        'no blocks' => TRUE,
      ),
      'context' => array(
        'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
        'form id' => 'ctools_context_handler_edit_context',
      ),
      'criteria' => array(
        'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
        'form id' => 'ctools_context_handler_edit_criteria',
      ),
    ),
    'default conf' => array(
      'autogenerate_title' => TRUE,
      'title' => t('Panel'),
      'no_blocks' => FALSE,
      'css_id' => '',
      'css' => '',
      'css_cache_file' => '',
      'contexts' => array(),
      'relationships' => array(),
    ),
  );
}

/**
 * Check selection criteria and, if passed, render the contexts.
 *
 * We must first check to ensure the node is of a type we're allowed
 * to render. If not, decline to render it by returning NULL.
 */
function panels_panel_context_render($handler, $base_contexts) {
  // Go through arguments and see if they match.
  ctools_include('context');
  ctools_include('context-task-handler');
  // Add my contexts
  $contexts = ctools_context_handler_get_handler_contexts($base_contexts, $handler);

  // Test.
  if (!ctools_context_handler_select($handler, $contexts)) {
    return;
  }

  // Load the display
  $display = drupal_clone($handler->conf['display']);
  $display->context = $contexts;
  $display->css_id = $handler->conf['css_id'];

  // Check to see if there is any CSS.
  if ($handler->conf['css_cache']) {
    if (!file_exists($handler->conf['css_cache'])) {
      // This will force the task handler to re-cache the CSS and save the filename:
      delegator_save_task_handler($handler);
    }
    drupal_add_css($handler->conf['css_cache']);
  }

  $output = panels_render_display($display);
  if (!empty($handler->conf['no_blocks'])) {
    print theme('page', $output, FALSE);
    // We return TRUE to let it know we handled this but have already
    // handled the output rendering ourselves.
    return TRUE;
  }
  return $output;
}

/**
 * Set up a title for the panel based upon the selection criteria.
 */
function panels_panel_context_title($handler, $task, $subtask_id) {
  if (isset($handler->conf['title'])) {
    return $handler->conf['title'];
  }
  else {
    return t('Panel');
  }
}

/**
 * Callback to allow the handler to react to being saved.
 *
 * When a handler with a display is saved, two things have to happen.
 * First, we have to scrub some contexts off the display that may
 * inadvertantly have been left on it. Second, we have to cache
 * any CSS that the display is using. This CSS can get re-cached
 * later if the file disappears, but it's imperative that we do it here
 * to make sure that old, dirty CSS cache gets removed.
 */
function panels_panel_context_save(&$handler, $update) {
  // @todo Save the display

  // Scrub the display.
  if (isset($handler->conf['display']->context)) {
    unset($handler->conf['display']->context);
  }

  foreach ($handler->conf['display']->content as $id => $pane) {
    if (isset($pane->context)) {
      unset($handler->conf['display']->content[$id]->context);
    }
  }

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
 * Special handling for exporting a panel task handler.
 *
 * When a panel is exported, we need to export the display separately
 * rather than just letting its object be unpacked, which does not work
 * very well.
 */
function panels_panel_context_export(&$handler, $indent) {
  $display = $handler->conf['display'];
  unset($handler->conf['display']);

  $output = panels_export_display($display, $indent);
  $output .= $indent . '$handler->conf[\'display\'] = $display' . ";\n";
  return $output;
}

/**
 * Provide a nice little summary of what's in a panel.
 *
 * The task handler manager provides a summary of a given handler in a
 * collapsible div. This callback provides that. For a Panel, we
 * provide a summary of the layout type and content on one side, and
 * a summary of the contexts in use on the other.
 */
function panels_panel_context_admin_summary($handler, $task, $subtask_id) {
  $output = '';
  $display = drupal_clone($handler->conf['display']);

  panels_load_include('plugins');
  ctools_include('context');
  ctools_include('context-task-handler');

  $layout = panels_get_layout($display->layout);
  $icon = panels_print_layout_icon($display->layout, $layout);

  $plugin = delegator_get_task_handler($handler->handler);

  $object = ctools_context_handler_get_task_object($task, $subtask_id, $handler);
  $display->context = ctools_context_load_contexts($object, TRUE);

  $content = theme('panels_common_content_list', $display);
  $contexts = theme('ctools_context_list_no_table', $object);

  $title = '<h2 class="content-title">' . check_plain($layout['title']) . '</h2>';

  $output .= '<div class="right-container">';
  $output .= '<h2 class="context-title">' . t('Contexts') . '</h2>';
  $output .= $contexts;
  $output .= '</div>';

  $output .= '<div class="left-container">';
  $output .= $icon . $title . $content;
  $output .= '</div>';

  // This routine dirties the display which can cause problems later.
  // Scrub it clean.
  if (isset($handler->conf['display']->context)) {
    unset($handler->conf['display']->context);
  }

  foreach ($handler->conf['display']->content as $id => $pane) {
    if (isset($pane->context)) {
      unset($handler->conf['display']->content[$id]->context);
    }
  }

  return $output;
}

// --------------------------------------------------------------------------
// Forms

/**
 * General notes about forms: The handler is automatically cached by the form
 * wizard, so anything we store on $form_state['handler'] anywhere will get
 * saved and appear on the next form. The cache is a 'working' cache and
 * if the user hits cancel on any page of the multi-page wizard, all
 * changes since the last 'update/finish' click will be flushed away.
 *
 * Many of the Panels forms call through to the real Panels cousins. These
 * forms are smart enough to know that they're being wrapped in another
 * form and act appropriately. Some of them are so smart that we just let
 * their submit and validate handlers do the work rather than writing
 * additional ones here.
 */

/**
 * Choose a layout for this panel.
 *
 * This is only called during 'add', when we know that there isn't a
 * previous layout to choose from. a different, only slightly different
 * variant is called to change a pre-existing layout.
 */
function panels_panel_context_edit_choose(&$form, &$form_state) {
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
function panels_panel_context_edit_choose_validate(&$form, &$form_state) {
  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_panel_context_edit_choose_submit(&$form, &$form_state) {
  $form_state['display']->layout = $form_state['values']['layout'];
  $form_state['handler']->conf['display'] = $form_state['display'];
}


/**
 * Change the layout for this panel.
 *
 * This form is only used if a layout already exists and the user wants
 * to change to a different one. The submit handler changes the next form
 * to the move content form, which is 'hidden' so it won't be accessed
 * directly.
 */
function panels_panel_context_edit_layout(&$form, &$form_state) {
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
function panels_panel_context_edit_layout_validate(&$form, &$form_state) {
  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_panel_context_edit_layout_submit(&$form, &$form_state) {
  // By default, 'next' takes us to whatever is next, but if they changed
  // the layout we need to interject a form.
  if ($form_state['values']['layout'] != $form_state['handler']->conf['display']->layout) {
    $form_state['handler']->conf['temp_layout'] = $form_state['values']['layout'];
    $form_state['clicked_button']['#next'] = 'move';
  }
}

/**
 * When a layout is changed, the user is given the opportunity to move content.
 */
function panels_panel_context_edit_move(&$form, &$form_state) {
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

  // This form is outside the normal wizard list, so we need to specify the
  // previous/next forms.
  $form['buttons']['previous']['#next'] = 'layout';
  $form['buttons']['next']['#next'] = 'layout-settings';
}

/**
 * Proxy the layout settings form.
 */
function panels_panel_context_edit_layout_settings(&$form, &$form_state) {
  ctools_include('ajax');
  panels_load_include('display-layout-settings');
  panels_load_include('plugins');

  $form_state['display'] = &$form_state['handler']->conf['display'];
//  $display->context = ctools_context_handler_get_all_contexts($form_state['task'], $form_state['subtask_id'], $form_state['handler']);

  // Allow setting of the 'display title'
  $form_state['display_title'] = TRUE;

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Prevent notices
  $form_state['finish'] = '';

  $form = array_merge($form, panels_edit_layout_settings_form($form_state));

  // Fix up the extra Save button this page needs. Our needs are a little
  // different than the normal Panels needs.
  $form['save']['#next'] = 'layout-settings';
  $form['save']['#weight'] = -1;
  $form['buttons']['save'] = $form['save'];
  unset($form['save']);
}

/**
 * Present the panels drag & drop editor to edit the display attached
 * to the task handler.
 */
function panels_panel_context_edit_content(&$form, &$form_state) {
  ctools_include('ajax');
  panels_load_include('plugins');
  panels_load_include('common');
  ctools_include('context');
  ctools_include('context-task-handler');

  $display = &$form_state['handler']->conf['display'];

  $display->context = ctools_context_handler_get_all_contexts($form_state['task'], $form_state['subtask_id'], $form_state['handler']);
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
function panels_panel_context_edit_settings(&$form, &$form_state) {
  $conf = $form_state['handler']->conf;
  $form['conf']['autogenerate_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically generate title'),
    '#description' => t('If checked, the administrative title will be automatically regenerated whenever the selection criteria changes. If you change the title and leave this check, your changes may disappear.'),
    '#default_value' => $conf['autogenerate_title'],
  );

  $form['conf']['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['title'],
    '#title' => t('Administrative title'),
    '#description' => t('Administrative title of this task handler.'),
  );

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
 * Submit handler for general settings form.
 */
function panels_panel_context_edit_settings_submit(&$form, &$form_state) {
  $form_state['handler']->conf['no_blocks'] = $form_state['values']['no_blocks'];
  $form_state['handler']->conf['css_id'] = $form_state['values']['css_id'];
  $form_state['handler']->conf['css'] = $form_state['values']['css'];
}

