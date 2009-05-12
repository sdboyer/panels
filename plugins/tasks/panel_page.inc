<?php
// $Id$

/**
 * @file
 * Handle the 'panels page' task, which creates pages but with a single
 * panel page task handler. This uses the delegator 'page' storage
 * mechanism, which means that it should be theoretically possible to
 * convert panel pages to custom pages if we provide a tool, allowing
 * users to start simple and convert to the more complex if needed.
 */

/**
 * Specialized implementation of hook_delegator_tasks(). See api-task.html for
 * more information.
 */
function panels_panel_page_delegator_tasks() {
  return array(
    'panel_page' => array(
      'title' => t('Panel pages'),
      'description' => t('Administrator created pages that have a URL path, access control and entries in the Drupal menu system and use a panel for their output.'),
      'subtasks' => TRUE,
      // These two use standard delegator callbacks.
      'subtask callback' => array(
        'file' => 'page.inc',
        'path' => drupal_get_path('module', 'delegator') . '/plugins/tasks',
        'function' => 'delegator_page_subtask',
      ),
      'subtasks callback' => array(
        'file' => 'page.inc',
        'path' => drupal_get_path('module', 'delegator') . '/plugins/tasks',
        'function' => 'delegator_page_subtasks',
      ),
      'save' => 'panels_page_save_page',
      'delete' => 'panels_page_delete_page',
      'export' => 'panels_page_export_page',
      'hook menu' => array(
        'function' => 'panels_panel_page_menu',
      ),
      'form info' => 'panels_panel_page_edit_form_info',
      // page only items
      'task type' => 'page',
      'operations' => array(
        array(
          'title' => t('Import'),
          'href' => 'admin/build/pages/import',
        ),
        array(
          'title' => t('Add panel page'),
          'href' => 'admin/build/pages/panel-page/add',
        ),
      ),
      'columns' => array(
        'storage' => array(
          'label' => t('Storage'),
          'class' => 'delegator-page-storage',
        ),
      ),
      'page type' => 'panel_page',
      // What page callback will execute this page. If it has handlers then
      // panels_panel_page_execute should be the default.
      'page callback' => 'panels_panel_page_execute',
      // context only items
      'handler type' => 'context',
      'get arguments' => array(
        'file' => 'page.inc',
        'path' => drupal_get_path('module', 'delegator') . '/plugins/tasks',
        'function' => 'delegator_page_get_arguments',
      ),
      'get context placeholders' => array(
        'file' => 'page.inc',
        'path' => drupal_get_path('module', 'delegator') . '/plugins/tasks',
        'function' => 'delegator_page_get_contexts',
      ),
    ),
  );
}

/**
 * Supply information for the multi-step wizard for both edit and add subtask
 */
function panels_panel_page_edit_form_info() {
  return array(
    'id' => 'panels_panel_page',
    'show trail' => TRUE,
    'show back' => TRUE,
    'show return' => FALSE,
    'next callback' => 'delegator_page_add_subtask_next',
    'finish callback' => 'delegator_page_add_subtask_finish',
    'return callback' => 'delegator_page_add_subtask_finish',
    'cancel callback' => 'delegator_page_add_subtask_cancel',
    'order' => array(
      'content' => t('Panel content'),
      'basic' => t('Page settings'),
      'argument' => t('Argument settings'),
      'context' => t('Contexts'),
      'layout' => t('Change layout'),
      'access' => t('Access control'),
      'menu' => t('Menu settings'),
      'move' => '', // This makes this a 'hidden' item.
    ),
    'forms' => array(
      // These are all basic page forms from the page task. Mostly.
      'basic' => array(
        'form id' => 'panels_panel_page_form_basic',
      ),
      'argument' => array(
        'form id' => 'delegator_page_form_argument',
      ),
      'access' => array(
        'form id' => 'delegator_page_form_access',
      ),
      'menu' => array(
        'form id' => 'delegator_page_form_menu',
      ),
      // These are all panel forms.
      'choose' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/plugins/task_handlers/panel_context.inc',
        ),
        'wrapper' => 'panels_panel_page_context_wrapper',
        'form id' => 'panels_panel_context_edit_choose',
      ),
      'layout' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/plugins/task_handlers/panel_context.inc',
        ),
        'wrapper' => 'panels_panel_page_context_wrapper',
        'no return' => TRUE,
        'form id' => 'panels_panel_context_edit_layout',
      ),
      'move' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/includes/display-layout.inc',
          drupal_get_path('module', 'panels') . '/plugins/task_handlers/panel_context.inc',
        ),
        'wrapper' => 'panels_panel_page_context_wrapper',
        'form id' => 'panels_panel_context_edit_move',
        'submit' => 'panels_change_layout_submit',
      ),
      'content' => array(
        'include' => array(
          drupal_get_path('module', 'panels') . '/includes/display-edit.inc',
          drupal_get_path('module', 'panels') . '/plugins/task_handlers/panel_context.inc',
        ),
        'wrapper' => 'panels_panel_page_context_wrapper',
        'form id' => 'panels_panel_context_edit_content',
        'submit' => 'panels_edit_display_form_submit',
        'no blocks' => TRUE,
      ),
      'context' => array(
        'wrapper' => 'panels_panel_page_context_wrapper',
        'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
        'form id' => 'ctools_context_handler_edit_context',
      ),
    ),
    // Items specific to the 'add path' that will get moved over:
    'add path' => 'admin/build/pages/panel-page/add/%step',
    'add order' => array(
      'basic' => t('Page settings'),
      'argument' => t('Argument settings'),
      'choose' => t('Choose layout'),
      'content' => t('Panel content'),
    ),
    // Items specific to the 'edit' routines that will get moved over:
    'edit path' => 'admin/build/pages/edit-panel-page/%page_name/%step',
  );
}

// --------------------------------------------------------------------------
// Page execution functions

/**
 * Execute a page task.
 *
 * This is the callback to entries in the Drupal menu system created by the
 * page task.
 *
 * @param $subtask_id
 *   The name of the page task used.
 * @param ...
 *   A number of context objects as specified by the user when
 *   creating named arguments in the path.
 */
function panels_panel_page_execute($page, $contexts, $args) {
  // Delegate to the panel context handler.
  if ($function = ctools_plugin_load_function('delegator', 'task_handlers', 'panel_context', 'render')) {
    // Provide a tab to edit this context:
    ctools_include('menu');
    ctools_menu_add_tab(array(
      'title' => t('Edit panel'),
      'href' => "admin/build/pages/edit-panel-page/" . $page->name,
    ));

    $info = $function($page, $contexts, $args, FALSE);
    if (isset($info['title'])) {
      drupal_set_title($info['title']);
    }

    // Only directly output if $page was set to true.
    if (!empty($info['no_blocks'])) {
      print theme('page', $info['content'], FALSE);
      return;
    }
    else {
      return $info['content'];
    }
  }
}

/**
 * Delegated implementation of hook_menu().
 */
function panels_panel_page_menu(&$items, $task) {
  // Set up access permissions.
  $access_callback = isset($task['admin access callback']) ? $task['admin access callback'] : 'user_access';
  $access_arguments = isset($task['admin access arguments']) ? $task['admin access arguments'] : array('administer delegator');

  $base = array(
    'access callback' => $access_callback,
    'access arguments' => $access_arguments,
    'file' => 'plugins/tasks/page.admin.inc',
  );

  $items['admin/build/pages/panel-page/add'] = array(
    'title' => 'Add panel page',
    'description' => 'Add a delegator page subtask.',
    'page callback' => 'delegator_page_add_subtask',
    'page arguments' => array('panel_page'),
    'type' => MENU_LOCAL_TASK,
  ) + $base;

  $form_info    = panels_panel_page_edit_form_info();
  $default_task = FALSE;
  $weight       = 0;
  foreach ($form_info['order'] as $step => $form_title) {
    // The first edit form is the default for tabs, so it gets a bit
    // of special treatment here.
    if (!$default_task) {
      $default_task = TRUE;
      // Add the callback for the default tab.
      $items["admin/build/pages/edit-panel-page/%"] = array(
        'title' => t('Edit'),
        'page callback' => 'delegator_page_edit_subtask',
        'page arguments' => array('panel_page', 4, $step),
        'type' => MENU_CALLBACK,
      ) + $base;

      // And make sure it's the default local task.
      $type = MENU_DEFAULT_LOCAL_TASK;
    }
    else {
      // This allows an empty form title to provide a hidden form
      // which is useful for doing more branch-like multi-step
      // functionality.
      $type = $form_title ? MENU_LOCAL_TASK : MENU_CALLBACK;
    }

    // Handler to edit delegator task handlers. May exist in its own UI.
    $items["admin/build/pages/edit-panel-page/%/$step"] = array(
      'title' => $form_title,
      'page callback' => 'delegator_page_edit_subtask',
      'page arguments' => array('panel_page', 4, 5),
      'type' => $type,
      'weight' => $weight++,
    ) + $base;
  }
}

// --------------------------------------------------------------------------
// Forms specific to a panel page

/**
 * Basic settings form for a panel page. This is a combination of the
 * page settings + the basic settings for a panel context task handler.
 */
function panels_panel_page_form_basic(&$form, &$form_state) {
  // Start with the basic form from page.inc
  delegator_page_form_basic($form, $form_state);
  $conf = $form_state['page']->conf;
  $form['no_blocks'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['no_blocks'],
    '#title' => t('Disable Drupal blocks/regions'),
    '#description' => t('Check this to have the panel page disable all regions displayed in the theme.'),
  );

  $form['css_id'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $conf['css_id'],
    '#title' => t('CSS ID'),
    '#description' => t('The CSS ID to apply to this page'),
  );

  $form['css'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#description' => t('Enter well-formed CSS code here; this code will be embedded into the page, and should only be used for minor adjustments; it is usually better to try to put CSS for the page into the theme if possible. This CSS will be filtered for safety so some CSS may not work.'),
    '#default_value' => $conf['css'],
  );
}

function panels_panel_page_form_basic_validate(&$form, &$form_state) {
  delegator_page_form_basic_validate($form, $form_state);
}

function panels_panel_page_form_basic_submit(&$form, &$form_state) {
  delegator_page_form_basic_submit($form, $form_state);
  $form_state['page']->conf['no_blocks'] = $form_state['values']['no_blocks'];
  $form_state['page']->conf['css_id'] = $form_state['values']['css_id'];
  $form_state['page']->conf['css'] = $form_state['values']['css'];

  // We don't actually want 'access' to be next in this version, people
  // want content more quickly.
  if ($form_state['clicked_button']['#next'] == 'access') {
    $form_state['clicked_button']['#next'] = 'choose';
  }
}

/**
 * This wrapper allows us to treat context task handler forms as though they
 * belong to the page by aliasing $form_state['handler'] as $form_state['page']
 *
 * Since all of the task handler stuff is stored in ->conf, this works
 * surprisingly well.
 */
function panels_panel_page_context_wrapper(&$form, &$form_state) {
  $form_state['handler'] = &$form_state['page'];
  $form_state['subtask_id'] = $form_state['page']->name;
}

/**
 * Callback to allow the task to react to being saved.
 *
 * When a task with a display is saved, two things have to happen.
  * First, we have to save the display so that it becomes a real display,
 * not the fake one we started with. Second, we have to cache
 * any CSS that the display is using. This CSS can get re-cached
 * later if the file disappears, but it's imperative that we do it here
 * to make sure that old, dirty CSS cache gets removed.
 */
function panels_page_save_page(&$page, $update) {
  // Only save the display if we believe it has been modified.
  if (isset($page->conf['display'])) {
    panels_save_display($page->conf['display']);
    $page->conf['did'] = $page->conf['display']->did;
    unset($page->conf['display']);
  }

  // Delete any previous CSS cache file.
  ctools_include('css');
  ctools_css_clear('panel_context:' . $page->name);

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
function panels_page_export_page(&$page, $indent) {
  // We're relying on panel context code for display handling, so make sure
  // it is loaded here.
  delegator_get_task_handler('panel_context');
  $display = &panels_panel_context_get_display($page);
  foreach (array('display', 'did', 'css_cache', 'temp_layout') as $item) {
    if (isset($page->conf[$item])) {
      unset($page->conf[$item]);
    }
  }

  $output = panels_export_display($display, $indent);
  $output .= $indent . '$page->conf[\'display\'] = $display' . ";\n";
  return $output;
}

/**
 * Callback to delete the display when a handler is deleted.
 */
function panels_page_delete_page(&$page) {
  if (!empty($page->conf['did'])) {
    panels_delete_display($page->conf['did']);
  }
}
