<?php
// $Id$

/**
 * @file
 *
 * Administrative items for the panels mini module.
 */
/**
 * Implementation of hook_menu().
 */
function _panels_mini_menu() {
  // Provide some common options to reduce code repetition.
  // By using array addition and making sure these are the rightmost
  // value, they won't override anything already set.
  $base = array(
    'access arguments' => array('create mini panels'),
    'file' => 'panels_mini.admin.inc',
  );

  $items['admin/panels/panel-mini'] = array(
    'title' => 'Mini panels',
    'page callback' => 'panels_mini_list_page',
    'description' => 'Create and administer mini panels (panels exposed as blocks).',
  ) + $base;
  $items['admin/panels/panel-mini/list'] = array(
    'title' => 'List',
    'page callback' => 'panels_mini_list_page',
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/add'] = array(
    'title' => 'Add',
    'page callback' => 'panels_mini_add_page',
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/import'] = array(
    'title' => 'Import',
    'page callback' => 'panels_mini_import_mini',
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_mini_settings',
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/disable'] = array(
    'page callback' => 'panels_mini_disable_page',
    'weight' => -1,
    'type' => MENU_CALLBACK,
  ) + $base;
  $items['admin/panels/panel-mini/enable'] = array(
    'page callback' => 'panels_mini_enable_page',
    'weight' => -1,
    'type' => MENU_CALLBACK,
  ) + $base;

  $base['access arguments'] = array('administer mini panels');

  $items['admin/panels/panel-mini/%panels_mini_admin'] = array(
    'title' => 'Preview',
    'page callback' => 'panels_mini_preview_panel',
    'page arguments' => array(3),
    'weight' => -10,
    'type' => MENU_CALLBACK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/preview'] = array(
    'title' => 'Preview',
    'page callback' => 'panels_mini_preview_panel',
    'page arguments' => array(3),
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/edit-layout'] = array(
    'title' => 'Layout',
    'page callback' => 'panels_mini_edit_layout',
    'page arguments' => array(3),
    'weight' => -9,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/edit-general'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_mini_edit',
    'page arguments' => array(3),
    'weight' => -5,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/edit-settings'] = array(
    'title' => 'Layout settings',
    'page callback' => 'panels_mini_edit_layout_settings',
    'page arguments' => array(3),
    'weight' => -3,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/edit-context'] = array(
    'title' => 'Context',
    'page callback' => 'panels_mini_edit_context',
    'page arguments' => array(3),
    'weight' => -2,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/edit-content'] = array(
    'title' => 'Content',
    'page callback' => 'panels_mini_edit_content',
    'page arguments' => array(3),
    'weight' => -1,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('panels_mini_edit_export', 3),
    'weight' => 0,
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  $items['admin/panels/panel-mini/%panels_mini_admin/delete'] = array(
    'title' => 'Delete mini panel',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('panels_mini_delete_confirm', 3),
    'type' => MENU_CALLBACK,
  ) + $base;

  return $items;
}

/**
 * Settings for mini panels.
 */
function panels_mini_settings() {
  return ('TODO');
  panels_load_include('common');
  return drupal_get_form('panels_common_settings', 'panels_mini');
}

/**
 * Provide a list of mini panels, with links to edit or delete them.
 */
function panels_mini_list_page() {
  panels_load_include('plugins');
  $layouts = panels_get_layouts();
  $items = array();
  $sorts = array();

  $header = array(
    array('data' => t('Title'), 'field' => 'title'),
    array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc'),
    array('data' => t('Type'), 'field' => 'type'),
    t('Layout'),
    t('Operations'),
  );

  // Load all mini panels and their displays.
  $panel_minis = panels_mini_load_all();
  $dids = array();
  foreach ($panel_minis as $panel_mini) {
    if (empty($panel_mini->display)) {
      $dids[] = $panel_mini->did;
    }
  }
  $displays = panels_load_displays($dids);

  if (empty($displays)) {
    return t('There are currently no mini panels.');
  }

  foreach ($panel_minis as $panel_mini) {
    $ops = array();
    if (empty($panel_mini->disabled)) {
      $ops[] = l(t('Edit'), "admin/panels/panel-mini/$panel_mini->name/edit-general");
      $ops[] = l(t('Export'), "admin/panels/panel-mini/$panel_mini->name/export");
    }
    if ($panel_mini->type != t('Default')) {
      $text = ($panel_mini->type == t('Overridden')) ? t('Revert') : t('Delete');
      $ops[] = l($text, "admin/panels/panel-mini/$panel_mini->name/delete");
    }
    else {
      if (empty($panel_mini->disabled)) {
        $ops[] = l(t('Disable'), "admin/panels/panel-mini/disable/$panel_mini->name", array('query' => drupal_get_destination()));
      }
      else {
        $ops[] = l(t('Enable'), "admin/panels/panel-mini/enable/$panel_mini->name", array('query' => drupal_get_destination()));
      }
    }

    $item = array();
    $item[] = check_plain($panel_mini->title);
    $item[] = check_plain($panel_mini->name);
    // this is safe as it's always programmatic
    $item[] = $panel_mini->type;

    if (empty($panel_mini->display)) {
      $panel_mini->display = $displays[$panel_mini->did];
    }

    $item[] = check_plain($layouts[$panel_mini->display->layout]['title']);
    $item[] = implode(' | ', $ops);
    $items[] = $item;
    $ts = tablesort_init($header);
    switch ($ts['sql']) {
      case 'title':
        $sorts[] = $item[0];
        break;

      case 'name':
      default:
        $sorts[] = $item[1];
        break;

      case 'type':
        $sorts[] = $panel_mini->type . $item[0];
        break;
    }
  }

  if (drupal_strtolower($ts['sort']) == 'desc') {
    arsort($sorts);
  }
  else {
    asort($sorts);
  }

  $i = array();
  foreach ($sorts as $id => $title) {
    $i[] = $items[$id];
  }

  $output = theme('table', $header, $i);
  return $output;
}

/**
 * Provide a form to confirm deletion of a mini panel.
 */
function panels_mini_delete_confirm(&$form_state, $panel_mini) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }
  $form['pid'] = array('#type' => 'value', '#value' => $panel_mini->pid);
  $form['did'] = array('#type' => 'value', '#value' => $panel_mini->did);
  return confirm_form($form,
    t('Are you sure you want to delete the mini panel "@title"?', array('@title' => $panel_mini->title)),
    $_GET['destination'] ? $_GET['destination'] : 'admin/panels/panel-mini',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * Handle the submit button to delete a mini panel.
 */
function panels_mini_delete_confirm_submit($form, &$form_state) {
  panels_mini_delete((object) $form_state['values']);
  $form_state['redirect'] = 'admin/panels/panel-mini';
}

/**
 * Provide an administrative preview of a mini panel.
 */
function panels_mini_preview_panel($mini) {
  $mini->display->args = array();
  $mini->display->css_id = panels_mini_get_id($mini->name);

  panels_load_include('plugins');
  $mini->context = $mini->display->context = panels_context_load_contexts($mini);

  drupal_set_title(filter_xss_admin($mini->title));
  return panels_render_display($mini->display);
}

/**
 * Page callback to export a mini panel to PHP code.
 */
function panels_mini_edit_export(&$form_state, $panel_mini) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }
  drupal_set_title(check_plain($panel_mini->title));
  $code = panels_mini_export($panel_mini);

  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => $panel_mini->title,
    '#default_value' => $code,
    '#rows' => $lines,
  );
  return $form;
}

/**
 * Page callback to import a mini panel from PHP code.
 */
function panels_mini_import_mini() {
  if ($_POST['form_id'] == 'panels_mini_edit_form') {
    $panel_mini = unserialize($_SESSION['pm_import']);
    drupal_set_title(t('Import panel mini "@s"', array('@s' => $panel_mini->title)));
    return drupal_get_form('panels_mini_edit_form', $panel_mini);
  }

  return drupal_get_form('panels_mini_import_form');
}

/**
 * Form for the mini panel import.
 */
function panels_mini_import_form() {
  $form['panel_mini'] = array(
    '#type' => 'textarea',
    '#title' => t('Panel mini code'),
    '#cols' => 60,
    '#rows' => 15,
    '#description' => t('Cut and paste the results of an exported mini panel here.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  $form['#redirect'] = NULL;
  return $form;
}

/**
 * Handle the submit button on importing a mini panel.
 */
function panels_mini_import_form_submit($form, &$form_state) {
  ob_start();
  eval($form_state['values']['panel_mini']);
  ob_end_clean();

  if (isset($mini)) {
    drupal_set_title(t('Import mini panel "@s"', array('@s' => $mini->title)));
    // As $mini contains non-stdClass objects,
    // it needs to be serialized before being stored in the session variable.
    $_SESSION['pm_import'] = serialize($mini);
    $output = drupal_get_form('panels_mini_edit_form', $mini);
    print theme('page', $output);
    exit;
  }
  else {
    drupal_set_message(t('Unable to get a mini panel out of that.'));
  }
}

/**
 * Handle the add mini panel page.
 */
function panels_mini_add_page($layout = NULL) {
  panels_load_include('plugins');
  $layouts = panels_get_layouts();
  $output = '';

  if ($layout === NULL) {
    foreach ($layouts as $id => $layout) {
      $output .= panels_print_layout_link($id, $layout, $_GET['q'] .'/'. $id);
    }
    return $output;
  }

  if (!$layouts[$layout]) {
    return drupal_not_found();
  }

  $panel_mini = new stdClass();
  $panel_mini->display = panels_new_display();
  $panel_mini->display->layout = $layout;
  $panel_mini->pid = 'new';
  $panel_mini->did = 'new';
  $panel_mini->title = '';
  $panel_mini->name = '';
  $panel_mini->category = '';

  drupal_set_title('Add mini panel');
  return panels_mini_edit($panel_mini);
}

/**
 * Edit a mini panel.
 *
 * Called from both the add and edit points to provide for common flow.
 */
function panels_mini_edit($panel_mini) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }
  if ($panel_mini) {
    drupal_set_title(check_plain($panel_mini->title));
  }

  return drupal_get_form('panels_mini_edit_form', $panel_mini);
}

/**
 * Form to edit the settings of a mini panel.
 */
function panels_mini_edit_form(&$form_state, $panel_mini) {
  panels_load_include('common');
  drupal_add_css(panels_get_path('css/panels_admin.css'));

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $panel_mini->pid,
  );
  $form['panel_mini'] = array(
    '#type' => 'value',
    '#value' => $panel_mini,
  );

  $form['right'] = array(
    '#prefix' => '<div class="layout-container">',
    '#suffix' => '</div>',
  );
  $form['left'] = array(
    '#prefix' => '<div class="info-container">',
    '#suffix' => '</div>',
  );

  $form['left']['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
  );
  $form['left']['settings']['title'] = array(
    '#type' => 'textfield',
    '#size' => 24,
    '#default_value' => $panel_mini->title,
    '#title' => t('Mini panel title'),
    '#description' => t('The title for this mini panel. It can be overridden in the block configuration.'),
  );

  $form['left']['settings']['name'] = array(
    '#type' => 'textfield',
    '#size' => 24,
    '#default_value' => $panel_mini->name,
    '#title' => t('Mini panel name'),
    '#description' => t('A unique name used to identify this panel page internally. It must be only be alpha characters and underscores. No spaces, numbers or uppercase characters.'),
  );

  $form['left']['settings']['category'] = array(
    '#type' => 'textfield',
    '#size' => 24,
    '#default_value' => $panel_mini->category,
    '#title' => t('Mini panel category'),
    '#description' => t("The category that this mini-panel will be grouped into on the Add Content form. Only upper and lower-case alphanumeric characters are allowed. If left blank, defaults to 'Mini panels'."),
  );

  panels_load_include('plugins');
  $panel_mini->context = $panel_mini->display->context = panels_context_load_contexts($panel_mini);

  $form['right']['layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Layout'),
  );

  $layout = panels_get_layout($panel_mini->display->layout);

  $form['right']['layout']['layout-icon'] = array(
    '#value' => panels_print_layout_icon($panel_mini->display->layout, $layout),
  );

  $form['right']['layout']['layout-display'] = array(
    '#value' => check_plain($layout['title']),
  );
  $form['right']['layout']['layout-content'] = array(
    '#value' => theme('panels_common_content_list', $panel_mini->display),
  );

  $contexts = theme('panels_common_context_list', $panel_mini);
  if ($contexts) {
    $form['right']['context'] = array(
      '#type' => 'fieldset',
      '#title' => t('Contexts'),
    );

    $form['right']['context']['context'] = array(
      '#value' => $contexts,
    );
  }

  $label = ($panel_mini->pid == 'new') ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Validate submission of the mini panel edit form.
 */
function panels_mini_edit_form_validate($form, &$form_state) {
  $form_values = &$form_state['values'];

  // Test uniqueness of name:
  if (!$form_values['name']) {
    form_error($form['left']['settings']['name'], t('Panel mini name is required.'));
  }
  else if (preg_match("/[^A-Za-z0-9_]/", $form_values['name'])) {
    form_error($form['left']['settings']['name'], t('Name must be alphanumeric or underscores only.'));
  }
  else if (preg_match("/[^A-Za-z0-9 ]/", $form_values['category'])) {
    form_error($form['left']['settings']['category'], t('Categories may contain only alphanumerics or spaces.'));
  }
  else {
    $query = "SELECT pid FROM {panels_mini} WHERE name = '%s'";
    if (!empty($form_values['pid']) && is_numeric($form_values['pid'])) {
      $query .= " AND pid != $form_values[pid]";
    }
    if (db_result(db_query($query, $form_values['name']))) {
      form_error($form['left']['settings']['name'], t('Panel name must be unique.'));
    }
  }
}

/**
 * Process submission of the mini panel edit form.
 */
function panels_mini_edit_form_submit($form, &$form_state) {
  $form_values = &$form_state['values'];
  $panel_mini = $form_values['panel_mini'];
  if ($panel_mini->pid != 'new' && $panel_mini->name != $form_values['name']) {
    // update all existing mini panels to point to this one appropriately.
    db_query("UPDATE {blocks} b SET delta = '%s' WHERE b.module = 'panels_mini' AND b.delta = '%s'", $form_values['name'], $panel_mini->name);

    // Above was blocks; these are actual panel panes.
    $result = db_query("SELECT * FROM {panels_pane} WHERE type = 'panels_mini' and subtype = '%s'", $panel_mini->name);
    while ($pane = db_fetch_object($result)) {
      $conf = unserialize($pane->configuration);
      $conf['name'] = $form_values['name'];
      db_query("UPDATE {panels_pane} SET configuration = '%s', subtype = '%s' WHERE pid = %d", serialize($conf), $conf['name'], $pane->pid);
    }
  }

  $panel_mini->title = $form_values['title'];
  $panel_mini->name = $form_values['name'];
  $panel_mini->category = empty($form_values['category']) ? '' : $form_values['category'];

  if ($panel_mini->pid == 'new') {
    unset($_SESSION['pm_import']);
    drupal_set_message(t('Your new mini panel %title has been saved.', array('%title' => $panel_mini->title)));
    panels_mini_save($panel_mini);
    $form_values['pid'] = $panel_mini->pid;

    $layout = panels_get_layout($panel_mini->display->layout);

    if ($layout['settings form']) {
      $form_state['redirect'] = "admin/panels/panel-mini/$panel_mini->name/edit-settings/next";
    }
    $form_state['redirect'] = "admin/panels/panel-mini/$panel_mini->name/edit-context/next";
  }
  else {
    drupal_set_message(t('Your changes have been saved.'));
    panels_mini_save($panel_mini);
  }
}

/**
 * Form to edit context features of a mini panel.
 */
function panels_mini_edit_context($panel_mini, $next = NULL) {
  if (!empty($_POST)) {
    $panel_mini = panels_cache_get('panel_object:panel_mini', $panel_mini->name);
  }
  else {
    panels_cache_set('panel_object:panel_mini', $panel_mini->name, $panel_mini);
  }

  drupal_set_title(check_plain($panel_mini->title));
  return drupal_get_form('panels_mini_context_form', $panel_mini, $next);
}

/**
 * Form to edit the context settings of a mini panel.
 */
function panels_mini_context_form(&$form_state, $panel_mini, $next = NULL) {
  drupal_add_css(panels_get_path('css/panels_admin.css'));
  panels_load_include('plugins');
  $layout = panels_get_layout($panel_mini->display->layout);

  $form['panel_mini'] = array(
    '#type' => 'value',
    '#value' => $panel_mini,
  );

  $form_state['panel_mini'] = $panel_mini;

  $form['right'] = array(
    '#prefix' => '<div class="right-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="left-container">',
    '#suffix' => '</div>',
  );

  panels_load_include('common-context');
  panels_load_include('ajax');

  $settings = panels_common_add_context_form('panel_mini', $form, $form_state, $form['right']['contexts_table'], $panel_mini);
  $settings += panels_common_add_required_context_form('panel_mini', $form, $form_state, $form['left']['required_contexts_table'], $panel_mini);
  $settings += panels_common_add_relationship_form('panel_mini', $form, $form_state, $form['right']['relationships_table'], $panel_mini);
  panels_common_add_context_js($settings);

  $label = $next ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Process submission of the mini panel edit form.
 */
function panels_mini_context_form_submit($form, &$form_state) {
  $panel_mini = $form_state['panel_mini'];

  // Organize these from the common form.
  panels_common_save_context('requiredcontext', $panel_mini->requiredcontexts, $form_state['values']);

  drupal_set_message(t('Your changes have been saved.'));
  panels_mini_save($panel_mini);
  panels_cache_clear('panel_object:panel_mini', $panel_mini->name);
  if ($form_state['values']['submit'] == t('Save and proceed')) {
    $form_state['redirect'] = "admin/panels/panel-mini/$panel_mini->name/edit-content";
  }
}

/**
 * Enable a default mini panel.
 */
function panels_mini_enable_page($name = NULL) {
  $defaults = panels_mini_default_panels();
  if (isset($defaults[$name])) {
    $status = variable_get('panel_mini_defaults', array());
    $status[$name] = FALSE;
    variable_set('panel_mini_defaults', $status);
    drupal_set_message(t('Panel mini enabled'));
  }
  drupal_goto();
}

/**
 * Disable a default mini panel.
 */
function panels_mini_disable_page($name = NULL) {
  $defaults = panels_mini_default_panels();
  if (isset($defaults[$name])) {
    $status = variable_get('panel_mini_defaults', array());
    $status[$name] = TRUE;
    variable_set('panel_mini_defaults', $status);
    drupal_set_message(t('Panel mini disabled'));
  }
  drupal_goto();
}

/**
 * Pass through to the panels content editor.
 */
function panels_mini_edit_content($panel_mini) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }
  panels_load_include('plugins');
  // Collect a list of contexts required by the arguments on this page.
  $panel_mini->display->context = $contexts = panels_context_load_contexts($panel_mini);

  panels_load_include('common');
  $content_types = panels_common_get_allowed_types('panels_mini', $contexts);

  $output = panels_edit($panel_mini->display, NULL, $content_types);
  if (is_object($output)) {
    $panel_mini->display = $output;
    $panel_mini->did = $output->did;
    panels_mini_save($panel_mini);
    drupal_goto("admin/panels/panel-mini/$panel_mini->name/edit-content");
  }
  // Print this with theme('page') so that blocks are disabled while editing a display.
  // This is important because negative margins in common block layouts (i.e, Garland)
  // messes up the drag & drop.
  drupal_set_title(check_plain($panel_mini->title));
  print theme('page', $output, FALSE);
}

/**
 * Pass through to the panels layout editor.
 */
function panels_mini_edit_layout($panel_mini) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }

  $output = panels_edit_layout($panel_mini->display, t('Save'));
  if (is_object($output)) {
    $panel_mini->display = $output;
    $panel_mini->did = $output->did;
    panels_mini_save($panel_mini);
    drupal_goto("admin/panels/panel-mini/$panel_mini->name/edit-layout");
  }

  drupal_set_title(check_plain($panel_mini->title));
  return $output;
}

/**
 * Pass through to the panels layout settings editor.
 */
function panels_mini_edit_layout_settings($panel_mini, $next = NULL) {
  if (!is_object($panel_mini)) {
    $panel_mini = panels_mini_load($panel_mini);
  }
  if (empty($next)) {
    $button = t('Save');
    $dest = "admin/panels/panel-mini/$panel_mini->name/edit-settings";
  }
  else {
    $button = t('Save and proceed');
    $dest = "admin/panels/panel-mini/$panel_mini->name/edit-context/next";
  }

  $output = panels_edit_layout_settings($panel_mini->display, $button);
  if (is_object($output)) {
    $panel_mini->display = $output;
    $panel_mini->did = $output->did;
    panels_mini_save($panel_mini);
    drupal_goto($dest);
  }

  drupal_set_title(check_plain($panel_mini->title));
  return $output;
}
