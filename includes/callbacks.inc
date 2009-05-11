<?php
// $Id$
/**
 * @file callbacks.inc
 * Minor menu callbacks for Panels helpers.
 */

/**
 * A central administrative page for Panels.
 */
function panels_admin_page() {
  return theme('panels_dashboard');
}

function template_preprocess_panels_dashboard(&$vars) {
  drupal_add_css(panels_get_path('css/panels-dashboard.css'));
  $vars['image_path'] = url(panels_get_path('images'));

  $vars['panel_page_description'] = t('Panel pages can be used as landing pages. They have a URL path, accept arguments and can have menu entries.');
  $vars['panel_custom_description'] = t('Advanced panel pages have everything regular pages do, and can have different layouts depending on the data being viewed.');
  $vars['panel_override_description'] = t('Customize an already existing page within your site.');
  if (module_exists('delegator')) {
    $vars['new_panel_page'] = l(t('Panel page'), 'admin/build/pages/panel-page/add');
    $vars['new_panel_custom'] = l(t('Advanced panel page'), 'admin/build/pages/add');
    $vars['new_panel_override'] = drupal_get_form('panels_dashboard_override_form');
  }
  else {
    $vars['new_panel_page'] = '<b>' . t('Panel page') . '</b><br /><em>' . t('You must activate the delegator module for this functionality.') . '</em>';
    $vars['new_panel_custom'] = '<b>' . t('Advanced Panel page') . '</b><br /><em>' . t('You must activate the delegator module for this functionality.') . '</em>';
    $vars['new_panel_override'] = '<em>' . t('You must activate the delegator module for this functionality.') . '</em>';
  }

  $vars['panel_node_description'] = t('Panel nodes are node content and appear in your searches, but are more limited than panel pages.');
  if (module_exists('panels_node')) {
    $vars['new_panel_node'] = l(t('Panel node'), 'node/add/panel');
  }
  else {
    $vars['new_panel_node'] = '<b>' . t('Panel node') . '</b><br /><em>' . t('You must activate the panel node module for this functionality.') . '</em>';
  }

  $vars['panel_mini_description'] = t('Mini panels are small content areas exposed as blocks, for when you need to have complex block layouts or layouts within layouts.');
  if (module_exists('panels_mini')) {
    $vars['new_panel_mini'] = l(t('Mini panel'), 'admin/build/panel-mini/add');
  }
  else {
    $vars['new_panel_mini'] = '<b>' . t('Mini panel') . '</b><br /><em>' . t('You must activate the Mini panels module for this functionality.') . '</em>';
  }
}

function panels_dashboard_override_form(&$form_state) {
  $options = array();
  foreach (delegator_get_tasks() as $name => $task) {
    if ($task['task type'] == 'page' && empty($task['subtasks'])) {
      $options[$name] = $task['title'];
    }
  }
  asort($options);

  $form['task'] = array(
    '#type' => 'select',
    '#options' => $options,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
  );

  return $form;
}

function panels_dashboard_override_form_submit(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/build/delegator/' . $form_state['values']['task'];
}

function panels_admin_settings_page() {
  $form = array();
  if (module_exists('delegator')) {
    foreach (delegator_get_tasks() as $task) {
      if ($function = ctools_plugin_get_function($task, 'admin settings')) {
        $function($form);
      }
    }
  }

  ctools_include('content');
  foreach (ctools_get_content_types() as $content) {
    if ($function = ctools_plugin_get_function($content, 'admin settings')) {
      $function($form);
    }
  }

  if (empty($form)) {
    return array('#value' => t('There are currently no settings to change, but additional plugins or modules may provide them in the future.'));
  }

  return system_settings_form($form);
}

/**
 * Settings for panel contexts created by the delegator.
 */
function panels_admin_panel_context_page() {
  panels_load_include('common');
  return drupal_get_form('panels_common_settings', 'panels_page');
}
