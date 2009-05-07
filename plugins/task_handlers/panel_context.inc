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
    'delete' => 'panels_panel_context_delete',
    'export' => 'panels_panel_context_export',
    'clone' => 'panels_panel_context_clone',

    'add forms' => array(
      'criteria' => t('Selection rules'),
      'choose' => t('Choose layout'),
      'content' => t('Panel content'),
    ),
    'edit forms' => array(
      'content' => t('Panel content'),
      'criteria' => t('Selection rules'),
      'settings' => t('General'),
      'context' => t('Contexts'),
      'layout' => t('Change layout'),
      'move' => '', // no title makes it a 'hidden' edit form.
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
      'contexts' => array(),
      'relationships' => array(),
    ),
  );
}

/**
 * Get the display for a task handler.
 *
 * There are three methods that the display can be found.
 * - In the database. $handler->conf['did'] will be set in this case,
 *   and $handler->conf['display'] won't be.
 * - In $handler->conf['display'], with $handler->conf['did'] empty. This
 *   will be true for a default/imported task handler as well as a handler
 *   that has just been created but has not yet been saved.
 * - in $handler->conf['display'] with $handler->conf['did' populated. This
 *   simply means that the display has been modified and is awaiting
 *   save. The modified one should always be used for editing purposes.
 * - If none of the above is true, then a new display needs to be created
 *   for the handler and pla
 */
function &panels_panel_context_get_display(&$handler) {
  if (isset($handler->conf['display'])) {
    return $handler->conf['display'];
  }

  if (isset($handler->conf['did'])) {
    $handler->conf['display'] = panels_load_display($handler->conf['did']);

    // Check for a valid display. If no valid display can be loaded, something
    // is wrong and we'll create a new one.
    if (!empty($handler->conf['display'])) {
      return $handler->conf['display'];
    }
  }

  $handler->conf['display'] = panels_new_display();

  return $handler->conf['display'];
}

/**
 * Check selection rules and, if passed, render the contexts.
 *
 * We must first check to ensure the node is of a type we're allowed
 * to render. If not, decline to render it by returning NULL.
 */
function panels_panel_context_render($handler, $base_contexts, $args, $test = TRUE) {
  // Go through arguments and see if they match.
  ctools_include('context');
  ctools_include('context-task-handler');
  // Add my contexts
  $contexts = ctools_context_handler_get_handler_contexts($base_contexts, $handler);

  // Test.
  if ($test && !ctools_context_handler_select($handler, $contexts)) {
    return;
  }

  // Load the display
  $display = panels_panel_context_get_display($handler);

  $display->context = $contexts;
  $display->args = $args;
  $display->css_id = $handler->conf['css_id'];

  // Check to see if there is any CSS.
  if (!empty($handler->conf['css'])) {
    ctools_include('css');
    $css_id = 'panel_context:' . $handler->name;
    $filename = ctools_css_retrieve($css_id);
    if (!$filename) {
      $filename = ctools_css_store($css_id, $handler->conf['css']);
    }
    drupal_add_css($filename);
  }

  // With an argument, this actually sets the display.
  panels_get_current_page_display($display);

  $info = array(
    'content' => panels_render_display($display),
    'no_blocks' => !empty($handler->conf['no_blocks']),
  );

  // If the hide title flag is set, just remove the title altogether.
  if (!empty($display->hide_title)) {
    $info['title'] = '';
  }
  else if (!empty($display->title)) {
    $info['title'] = filter_xss_admin(ctools_context_keyword_substitute($display->title, array(), $contexts));
  }

  return $info;
}

/**
 * Set up a title for the panel based upon the selection rules.
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
 * First, we have to save the display so that it becomes a real display,
 * not the fake one we started with. Second, we have to cache
 * any CSS that the display is using. This CSS can get re-cached
 * later if the file disappears, but it's imperative that we do it here
 * to make sure that old, dirty CSS cache gets removed.
 */
function panels_panel_context_save(&$handler, $update) {
  // Only save the display if we believe it has been modified.
  if (isset($handler->conf['display'])) {
    panels_save_display($handler->conf['display']);
    $handler->conf['did'] = $handler->conf['display']->did;
    unset($handler->conf['display']);
  }

  // Delete any previous CSS cache file.
  ctools_include('css');
  ctools_css_clear('panel_context:' . $handler->name);

  if (isset($page->conf['temp_layout'])) {
    unset($page->conf['temp_layout']);
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
  $display = panels_panel_context_get_display($handler);
  foreach (array('display', 'did', 'css_cache', 'temp_layout') as $item) {
    if (isset($handler->conf[$item])) {
      unset($handler->conf[$item]);
    }
  }

  $output = panels_export_display($display, $indent);
  $output .= $indent . '$handler->conf[\'display\'] = $display' . ";\n";
  return $output;
}

/**
 * When a handler is cloned, we have to clone the display.
 */
  function panels_panel_context_clone(&$handler) {
  $old_display = panels_panel_context_get_display($handler);
  $code = panels_export_display($old_display);
  eval($code);
  foreach (array('display', 'did', 'css_cache', 'temp_layout') as $item) {
    if (isset($handler->conf[$item])) {
      unset($handler->conf[$item]);
    }
  }
  $display->did = 'new';
  $handler->conf['display'] = $display;
}

/**
 * Callback to delete the display when a handler is deleted.
 */
function panels_panel_context_delete(&$handler) {
  if (!empty($handler->conf['did'])) {
    panels_delete_display($handler->conf['did']);
  }
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

  $display = panels_panel_context_get_display($handler);

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
  $form_state['allowed_layouts'] = 'panels_page';

  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);

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
  $form_state['allowed_layouts'] = 'panels_page';

  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);

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
  $display = &panels_panel_context_get_display($form_state['handler']);

  if ($form_state['values']['layout'] != $display->layout) {
    $form_state['handler']->conf['temp_layout'] = $form_state['values']['layout'];
    $form_state['clicked_button']['#next'] = 'move';
  }
}

/**
 * When a layout is changed, the user is given the opportunity to move content.
 */
function panels_panel_context_edit_move(&$form, &$form_state) {
  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);
  $form_state['layout'] = $form_state['handler']->conf['temp_layout'];

  panels_load_include('common');
  panels_load_include('display-layout');
  panels_load_include('plugins');

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form = array_merge($form, panels_change_layout($form_state));

  // Change the 'back' button to just go directly to the previous form
//  $task_id = $form_state['task']['name'];
//  $handler_id = $form_state['handler']->handler;
//  $name = $form_state['handler']->name;

  // This form is outside the normal wizard list, so we need to specify the
  // previous/next forms.
  $form['buttons']['previous']['#next'] = 'layout';
  $form['buttons']['next']['#next'] = 'content';
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

  $display = &panels_panel_context_get_display($form_state['handler']);

  $display->context = ctools_context_handler_get_all_contexts($form_state['task'], $form_state['subtask_id'], $form_state['handler']);
  $content_types = panels_common_get_allowed_types('panels_page', $display->context);

// Load the display being edited from cache, if possible.
  if (!empty($_POST) && is_object($cache = panels_cache_get('display', $display->did))) {
    $display = $cache->display;
  }
  else {
    panels_cache_clear('display', $display->did);
    $cache = new stdClass();
    $cache->display = $display;
    $cache->content_types = $content_types;
    $cache->display_title = TRUE;
    panels_cache_set('display', $display->did, $cache);
  }

  $form_state['display'] = &$display;
  $form_state['content_types'] = $content_types;
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
    '#description' => t('If checked, the administrative title will be automatically regenerated whenever the selection rules changes. If you change the title and leave this check, your changes may disappear.'),
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

