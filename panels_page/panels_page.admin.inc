<?php
// $Id$


/**
 * @file panels_page.admin.inc
 *
 * Administrative screens and functions for panel pages.
 */

/**
 * Provide a list of panels, with links to edit or delete them.
 *
 * TODO Since we've already got the views dependency, should we just do this as
 * a view?
 */
function panels_page_list_page() {
  panels_load_include('plugins');
  $layouts = panels_get_layouts();
  $items = array();
  $sorts = array();

  $header = array(
    array('data' => t('Page title'), 'field' => 'title'),
    array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc'),
    array('data' => t('Type'), 'field' => 'type'),
    t('Layout'),
    array('data' => t('URL'), 'field' => 'url'),
    t('Operations'),
  );

  // Load all panel pages and their primary displays.
  $panel_pages = panels_page_load_all();
  $dids = array();
  foreach ($panel_pages as $panel_page) {
    if (empty($panel_page->display)) {
      $dids[] = $panel_page->did;
    }
  }
  $displays = panels_load_displays($dids);

  foreach ($panel_pages as $panel_page) {
    $ops = array();
    if (empty($panel_page->disabled)) {
      $ops[] = l(t('Edit'), "admin/panels/panel-page/{$panel_page->name}/edit");
      $ops[] = l(t('Export'), "admin/panels/panel-page/{$panel_page->name}/edit/export");
    }
    if ($panel_page->type != t('Default')) {
      $text = $panel_page->type == t('Overridden') ? t('Revert') : t('Delete');
      $ops[] = l($text, "admin/panels/panel-page/delete/{$panel_page->name}");
    }
    else {
      if (empty($panel_page->disabled)) {
        $ops[] = l(t('Disable'), "admin/panels/panel-page/disable/{$panel_page->name}", NULL, drupal_get_destination());
      }
      else {
        $ops[] = l(t('Enable'), "admin/panels/panel-page/enable/{$panel_page->name}", NULL, drupal_get_destination());
      }
    }

    $path = empty($panel_page->disabled) && strpos($panel_page->path, '%') === FALSE ? l($panel_page->path, $panel_page->path) : check_plain($panel_page->path);
    $item = array();
    $item[] = check_plain(panels_page_get_title($panel_page));
    $item[] = check_plain($panel_page->name);
    // this is safe as it's always programmatic
    $item[] = $panel_page->type;

    if (empty($panel_page->display)) {
      $panel_page->display = $displays[$panel_page->did];
    }
    $item[] = check_plain($layouts[$panel_page->display->layout]['title']);
    $item[] = $path;
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
        $sorts[] = $panel_page->type . $item[0];
        break;

      case 'url':
        $sorts[] = $panel_page->path;
        break;
    }
  }

  if (!empty($ts['sort']) && drupal_strtolower($ts['sort']) == 'desc') {
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
 * General settings configuration page for panel pages.
 */
function panels_page_settings() {
  panels_load_include('common');
  return drupal_get_form('panels_common_settings', 'panels_page');
}

/**
 * Enable a default panel_page.
 */
function panels_page_enable_page($name = NULL) {
  $defaults = panels_page_default_panels();
  if (isset($defaults[$name])) {
    $status = variable_get('panel_page_defaults', array());
    $status[$name] = FALSE;
    variable_set('panel_page_defaults', $status);
    menu_rebuild();
    drupal_set_message(t('Panel page enabled'));
  }
  drupal_goto();
}

/**
 * Disable a default panel_page.
 */
function panels_page_disable_page($name = NULL) {
  $defaults = panels_page_default_panels();
  if (isset($defaults[$name])) {
    $status = variable_get('panel_page_defaults', array());
    $status[$name] = TRUE;
    variable_set('panel_page_defaults', $status);
    drupal_set_message(t('Panel page disabled'));
    menu_rebuild();
  }
  drupal_goto();
}

/**
 * Provide a form to confirm deletion of a panel page.
 */
function panels_page_delete_confirm($panel_page) {
  $form['pid'] = array('#type' => 'value', '#value' => $panel_page->pid);
  $form['did'] = array('#type' => 'value', '#value' => $panel_page->did);
  if ($panel_page->type != t('Overridden')) {
    return confirm_form($form,
      t('Are you sure you want to delete "@title"?', array('@title' => panels_page_get_title($panel_page))),
      $_GET['destination'] ? $_GET['destination'] : 'admin/panels/panel-page',
      t('This action cannot be undone.'),
      t('Delete'), t('Cancel')
    );
  }
  else {
    return confirm_form($form,
      t('Are you sure you want to revert "@title" to its default setup?', array('@title' => panels_page_get_title($panel_page))),
      $_GET['destination'] ? $_GET['destination'] : 'admin/panels/panel-page',
      t('This action cannot be undone.'),
      t('Revert'), t('Cancel')
    );
  }
}

/**
 * Handle the submit button to delete a panel page.
 */
function panels_page_delete_confirm_submit($form_id, $form) {
  if ($form['confirm']) {
    panels_page_delete((object) $form);
    // TODO: Is this necessary or did we feed it the location already?
    return 'admin/panels/panel-page';
  }
}

/**
 * Handle the add panel-page page.
 */
function panels_page_add_page($layout = NULL) {
  panels_load_include('plugins');
  $layouts = panels_get_layouts();

  // If the layout given is invalid, unset it.
  if (is_array($layout) && isset($layouts[$layout])) {
    drupal_set_message(t('The chosen layout %layout is invalid. Please select a new one.', array('%layout' => $layout)), 'warning');
    unset($layout);
  }
  // If no page layout is given, choose one first.
  if ($layout === NULL) {
    foreach ($layouts as $id => $layout) {
      $output .= panels_print_layout_link($id, $layout, $_GET['q'] . '/' . $id);
    }
    return $output;
  }

  $panel_page = new stdClass();
  $panel_page->display = panels_new_display();
  $panel_page->display->layout = $layout;
  $panel_page->pid = 'new';
  $panel_page->did = 'new';

  drupal_set_title(t('Add panel page'));
  return drupal_get_form('panels_page_edit_form', panels_page_sanitize($panel_page));
}

/**
 * Edit a panel page.
 *
 * Called from both the add and edit points to provide for common flow.
 */
function panels_page_edit($panel_page) {
  panels_load_include('plugins');
  if (!is_object($panel_page)) {
    $panel_page = panels_page_load($panel_page);
  }

  if (empty($panel_page->display)) {
    $panel_page->display = panels_load_display($panel_page->did);
  }

  drupal_set_title(check_plain(panels_page_get_title($panel_page)));
  return drupal_get_form('panels_page_edit_form', $panel_page);
}

/**
 * The form to edit the page portion of a panel.
 */
function panels_page_edit_form($form_state, $panel_page, $next = NULL) {
  panels_load_include('common');
  drupal_add_css(panels_get_path('css/panels_admin.css'));
  $layout = panels_get_layout($panel_page->display->layout);

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $panel_page->pid,
  );

  $form['panel_page'] = array(
    '#type' => 'value',
    '#value' => $panel_page,
  );

  $form['right'] = array(
    '#prefix' => '<div class="layout-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="panel-page-info-container">',
    '#suffix' => '</div>',
  );

  $form['left']['info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page settings'),
  );

  $form['right']['layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Layout'),
  );

  $form['right']['layout']['layout-icon'] = array(
    '#value' => panels_print_layout_icon($panel_page->display->layout, $layout),
  );

  $form['right']['layout']['layout-display'] = array(
    '#value' => check_plain($layout['title']),
  );

  $panel_page->context = $panel_page->display->context = panels_context_load_contexts($panel_page);

  $form['right']['layout']['layout-content'] = array(
    '#value' => theme('panels_common_content_list', $panel_page->display), //$content,
  );

  $contexts = theme('panels_common_context_list', $panel_page);
  if ($contexts) {
    $form['right']['context'] = array(
      '#type' => 'fieldset',
      '#title' => t('Contexts'),
    );

    $form['right']['context']['context'] = array(
      '#value' => $contexts,
    );
  }

  $form['left']['info']['name'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $panel_page->name,
    '#title' => t('Panel name'),
    '#description' => t('A unique name used to identify this panel page internally. It must be only be alpha characters and underscores. No spaces, numbers or uppercase characters.'),
    '#required' => TRUE,
  );

  $form['left']['info']['title'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $panel_page->title,
    '#title' => t('Page title'),
    '#description' => t("The page title for this panels layout. It will be used as the main title on this panel page, unless it is overriden later."),
  );

  $form['left']['info']['css_id'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $panel_page->css_id,
    '#title' => t('CSS ID'),
    '#description' => t('The CSS ID to apply to this page'),
  );

  $form['left']['info']['path'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $panel_page->path,
    '#title' => t('Path'),
    '#description' => t('The URL path to give this page, i.e, path/to/page. You may use "%" as an argument placeholder: i.e, node/%/panel'),
    '#required' => TRUE,
  );

  $label = ($panel_page->pid == 'new') ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Validate a panel page edit form.
 */
function panels_page_edit_form_validate($form_id, $form_values, $form) {
  // Validate the system name; check for its presence, pass it through a preg
  // filter for allowed characters, and test for uniqueness.
  if (!$form_values['name']) {
    form_error($form['left']['info']['name'], t('Panel name is required.'));
  }
  else if (preg_match("/[^A-Za-z0-9_]/", $form_values['name'])) {
    form_error($form['left']['info']['name'], t('Panel name must be alphanumeric or underscores only.'));
  }
  else {
    if (db_result(db_query("SELECT pid FROM {panels_page} WHERE name = '%s' AND pid <> %d", $form_values['name'], $form_values['pid']))) {
      form_error($form['left']['info']['name'], t('Panel name must be unique.'));
    }
  }

  // Validate the path; check for its presence, uniqueness within panels_page,
  // and uniqueness with respect to path aliases.
  if (!$form_values['path']) {
    form_error($form['left']['info']['path'], t('Path is required.'));
  }
  else {
    if (db_result(db_query("SELECT name FROM {panels_page} WHERE path = '%s' AND pid <> %d", $form_values['path'], $form_values['pid']))) {
      form_error($form['left']['info']['path'], t("'Path' must be unique among Panels Page paths. The path '@path' is currently in owned by '@name'.",
        array('@path' => $form_values['path'], '@name' => $result)));
    }
    if (db_result(db_query("SELECT src FROM {url_alias} WHERE dst = '%s'", $form_values['path']))) {
      form_error($form['left']['info']['path'], t('The path you have chosen is already aliased to @src.', array('@src' => $result)));
    }
  }
}

/**
 * Process submission of the panel page edit form
 */
function panels_page_edit_form_submit($form_id, $form_values) {
  $panel_page = $form_values['panel_page'];

  $panel_page->title  = $form_values['title'];
  $panel_page->name   = preg_replace("/[\W]+/", '', $form_values['name']);
  $panel_page->css_id = $form_values['css_id'];
  $panel_page->path   = $form_values['path'];

  if ($panel_page->pid == 'new') {
    unset($_SESSION['pp_import']);
    drupal_set_message(t('Your new panel page %title has been saved.', array('%title' => $panel_page->title)));
    $pid = panels_page_save($panel_page);
    $GLOBALS['form_values']['pid'] = $pid;
    $layout = panels_get_layout($panel_page->display->layout);

    if ($layout['settings form']) {
      return "admin/panels/panel-page/{$panel_page->name}/edit/settings/next";
    }
    return "admin/panels/panel-page/{$panel_page->name}/edit/advanced/next";
  }
  else {
    drupal_set_message(t('Your changes have been saved.'));
    panels_page_save($panel_page);
  }
}

/**
 * Edit advanced settings of a panel page.
 */
function panels_page_edit_advanced($panel_page, $next = NULL) {
  $form_state = array(
    'panel_page' => &$panel_page,
    'next' => $next,
  );
  return drupal_get_form('panels_page_advanced_form', $panel_page, $next);
}

/**
 * The form to edit the advanced settings of a panel page.
 */
function panels_page_advanced_form($form_state, $panel_page, $next) {
  drupal_add_css(panels_get_path('css/panels_admin.css'));
  $form_state['panel_page'] =& $panel_page;

  $form['right'] = array(
    '#prefix' => '<div class="right-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="left-container">',
    '#suffix' => '</div>',
  );

  $form['right']['advanced'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#title' => t('Advanced settings'),
  );

  $form['right']['advanced']['no_blocks'] = array(
    '#type' => 'checkbox',
    '#default_value' => $panel_page->no_blocks,
    '#title' => t('Disable Drupal blocks/regions'),
    '#description' => t('Check this to have the panel page disable all regions displayed in the theme.'),
  );

  $rids = array();
  $result = db_query("SELECT r.rid, r.name FROM {role} r ORDER BY r.name");
  while ($obj = db_fetch_object($result)) {
    $rids[$obj->rid] = $obj->name;
  }

  $form['right']['advanced']['access'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Access'),
    '#default_value' => $panel_page->access,
    '#options' => $rids,
    '#description' => t('Only the checked roles will be able to see this panel in any form; if no roles are checked, access will not be restricted.'),
  );

  $form['right']['advanced']['css'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#description' => t('Enter well-formed CSS code here; this code will be embedded into the page, and should only be used for minor adjustments; it is usually better to try to put CSS for the page into the theme if possible.'),
    '#default_value' => $panel_page->css,
  );

  $form['left']['menu-info'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#title' => t('Menu'),
  );

  $form['left']['menu-info']['menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Menu'),
    '#return_value' => 1,
    '#default_value' => $panel_page->menu,
    '#description' => t('If checked this panel be given a menu entry in the Drupal menu system. If not checked the data in this group will be ignored.'),
  );

  $form['left']['menu-info']['menu_tab'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide Menu as Tab'),
    '#return_value' => 1,
    '#default_value' => $panel_page->menu_tab,
    '#description' => t("If checked this panel's menu entry will be provided as a tab rather than in the main menu system."),
  );

  $form['left']['menu-info']['menu_tab_weight'] = array(
    '#type' => 'textfield',
    '#title' => t('Tab Weight'),
    '#default_value' => $panel_page->menu_tab_weight,
    '#size' => 5,
    '#description' => t('If this is a menu tab, select the weight; lower numbers will be further to the left.'),
  );

  $form['left']['menu-info']['menu_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Menu Title'),
    '#default_value' => $panel_page->menu_title,
    '#size' => 35,
    '#maxlength' => 255,
    '#description' => t('Enter the title to use for the menu entry or tab. If blank, the page title will be used.'),
  );

  $form['left']['menu-info']['default-tab'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Default Menu Tab'),
  );

  $form['left']['menu-info']['default-tab']['menu_tab_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Make Default Menu Tab'),
    '#return_value' => 1,
    '#default_value' => $panel_page->menu_tab_default,
    '#description' => t("If checked this panel's menu entry will be provided as a tab, and will be the default tab for that URL path. For example, if the URL is 'tracker/all' and it is set as the default menu tab, it will be put into the menu as 'tracker' and 'tracker/all' will be the default tab. The following settings allow you to customize the parent item, for example 'tracker'. For tabs to work properly, one tab in the group must be set as the default."),
  );

  $form['left']['menu-info']['default-tab']['menu_tab_default_parent_type'] = array(
    '#type' => 'select',
    '#title' => t('Parent Menu Item Type'),
    '#default_value' => $panel_page->menu_tab_default_parent_type,
    '#options' => array(
      'tab' => t("Tab"),
      'normal' => t("Normal menu item"),
      'existing' => t("Already exists (don't create)"),
    ),
    '#description' => t("Select type of parent item to use for this default menu tab. You can either specify the parent should be a tab (the default), a normal menu item, or to use the menu item that already exists at the specified URL. For example, if the URL for the default tab is 'tracker/all', then 'tracker' would already have to be a valid menu item to use this final choice."),
  );

  $form['left']['menu-info']['default-tab']['menu_parent_tab_weight'] = array(
    '#type' => 'textfield',
    '#title' => t('Tab Weight'),
    '#default_value' => $panel_page->menu_parent_tab_weight,
    '#size' => 5,
    '#description' => t('If the parent menu item is a tab, select the weight; lower numbers will be further to the left.'),
  );

  $form['left']['menu-info']['default-tab']['menu_parent_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent Menu Item Title'),
    '#default_value' => $panel_page->menu_parent_title,
    '#size' => 35,
    '#maxlength' => 255,
    '#description' => t('If the Parent Menu Item is being defined by this panel (if you set the %type_field to either %tab or %menu), you can specify its title here.  If blank, the menu title will be used if that is defined, or the page title if not.', array('%type_field' => t('Parent Menu Item Type'), '%tab' => t('Tab'), '%menu' => t('Normal menu item'))),
  );

  $label = ($next) ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Process submission of the panel page edit form.
 */
function panels_page_advanced_form_submit($form_id, $form_values) {
  $panel_page = $form_values['panel_page'];

  $panel_page->css                          = $form_values['css'];
  $panel_page->no_blocks                    = $form_values['no_blocks'];
  $panel_page->menu                         = $form_values['menu'];
  $panel_page->menu_tab                     = $form_values['menu_tab'];
  $panel_page->menu_tab_weight              = $form_values['menu_tab_weight'];
  $panel_page->menu_title                   = $form_values['menu_title'];
  $panel_page->menu_tab_default             = $form_values['menu_tab_default'];
  $panel_page->menu_tab_default_parent_type = $form_values['menu_tab_default_parent_type'];
  $panel_page->menu_parent_title            = $form_values['menu_parent_title'];
  $panel_page->menu_parent_tab_weight       = $form_values['menu_parent_tab_weight'];

  $panel_page->access = array_keys(array_filter($form_values['access']));

  drupal_set_message(t('Your changes have been saved.'));
  panels_page_save($panel_page);
  if ($form_values['submit'] == t('Save and proceed')) {
    return "admin/panels/panel-page/{$panel_page->name}/edit/context/next";
  }
}

/**
 * Edit advanced settings of a panel page.
 */
function panels_page_edit_context($name, $next = NULL) {
  if (!empty($_POST)) {
    $panel_page = panels_cache_get('panel_object:panel_page', $name);
  }
  else {
    $panel_page = panels_page_load($name);
    panels_cache_set('panel_object:panel_page', $name, $panel_page);
  }

  drupal_set_title(check_plain(panels_page_get_title($panel_page)));
  return drupal_get_form('panels_page_context_form', $panel_page, $next);
}

/**
 * The form to edit the context settings of a panel page.
 */
function panels_page_context_form($panel_page, $next = NULL) {
  drupal_add_css(panels_get_path('css/panels_admin.css'));
  $layout = panels_get_layout($panel_page->display->layout);

  $form['panel_page'] = array(
    '#type' => 'value',
    '#value' => $panel_page,
  );

  $form['right'] = array(
    '#prefix' => '<div class="right-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="left-container">',
    '#suffix' => '</div>',
  );

  panels_load_include('common-context');

  $settings = panels_common_add_argument_form('panel_page', $form, $form['left']['arguments_table'], $panel_page);
  $settings += panels_common_add_context_form('panel_page', $form, $form['right']['contexts_table'], $panel_page);
  $settings += panels_common_add_relationship_form('panel_page', $form, $form['left']['relationships_table'], $panel_page);
  panels_common_add_context_js($settings);

  $label = ($next) ? t('Save and proceed') : t('Save');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );

  return $form;
}

/**
 * Process submission of the panel page edit form.
 */
function panels_page_context_form_submit($form_id, $form_values) {
  $panel_page = $form_values['panel_page'];

  // Organize these from the common form.
  panels_common_save_context('argument', $panel_page->arguments, $form_values);
  panels_common_save_context('context', $panel_page->contexts, $form_values);
  panels_common_save_context('relationship', $panel_page->relationships, $form_values);

  // Match up our displays, carry them forward and add new ones.
  $old_displays = $panel_page->displays;
  $panel_page->displays = array();

  foreach ($panel_page->arguments as $id => $argument) {
    $def = panels_get_argument($argument['name']);
    if (function_exists($def['displays'])) {
      // Figure out which instance of this particular argument type we're using.

      $displays = $def['displays']($argument['argument_settings'], $argument['id']);
      foreach ($displays as $did => $info) {
        $pdid = "argument_$id" . '-' . $did;
        if (isset($old_displays[$pdid])) {
          $panel_page->displays[$pdid] = $old_displays[$pdid];
          unset($old_displays[$pdid]);
          // ensure titles get updated if necessary
          $panel_page->displays[$pdid]['title'] = $info['title'];
        }
        else {
          $panel_page->displays[$pdid] = array(
            'did' => 'new',
            'title' => $info['title'],
            'default' => "argument_$id" . '-' . $info['default'],
            'argument_id' => $id,
            'context' => $info['context'],
          );
        }
      }
    }
  }

  // Remove remaining old displays.
  foreach ($old_displays as $id => $info) {
    if (is_numeric($info['did'])) {
      panels_delete_display($info['did']);
      drupal_set_message(t('Removed unused display @title', $info['title']));
    }
  }

  drupal_set_message(t('Your changes have been saved.'));
  panels_page_save($panel_page);
  panels_cache_clear('panel_object:panel_page', $panel_page->name);
  if ($form_values['submit'] == t('Save and proceed')) {
    return "admin/panels/panel-page/{$panel_page->name}/edit/content";
  }
}

/**
 * Pass through to the panels layout editor.
 */
function panels_page_edit_layout($panel_page, $did = NULL) {
  if (!is_object($panel_page)) {
    $panel_page = panels_page_load($panel_page);
  }

  $display = panels_page_fetch_display($panel_page, $did);

  // The following form will return the $display upon successful submit, if
  // we didn't send the $dest. Which we don't, here. That way we can update
  // our panel_page if necessary and do our own goto.
  $output = panels_edit_layout($panel_page->display, t('Save'));
  if (is_object($output)) {
    // If this is a default page, and was saved, we have to make it real.
    panels_page_set_display($panel_page, $did, $output);

    $dest = "admin/panels/panel-page/edit/{$panel_page->name}/layout";
    if ($did) {
      $dest .= "/$did";
    }
    drupal_goto($dest);
  }

  drupal_set_title(check_plain(panels_page_get_title($panel_page)));
  return $output;
}

/**
 * Pass through to the panels layout settings editor.
 */
function panels_page_edit_layout_settings($panel_page, $did = NULL, $next = NULL) {
  if (!is_object($panel_page)) {
    $panel_page = panels_page_load($panel_page);
  }

  $display = panels_page_fetch_display($panel_page, $did);

  if (empty($next)) {
    $button = t('Save');
    $dest = "admin/panels/panel-page/{$panel_page->name}/edit/settings";
    if ($did) {
      $dest .= "/$did";
    }
  }
  else {
    $button = t('Save and proceed');
    $dest = "admin/panels/panel-page/{$panel_page->name}/edit/advanced/next";
  }

  // The following form will return the $display upon successful submit, if
  // we didn't send the $dest. Which we don't, here.
  $output = panels_edit_layout_settings($display, $button, NULL, $panel_page->title);
  if (is_object($output)) {
    panels_page_set_display($panel_page, $did, $output);
    drupal_goto($dest);
  }

  drupal_set_title(check_plain(panels_page_get_title($panel_page)));
  return $output;
}

/**
 * Pass through to the panels content editor.
 */
function panels_page_edit_content($panel_page, $did = NULL) {
  if (!is_object($panel_page)) {
    $panel_page = panels_page_load($panel_page);
  }

  $display = panels_page_fetch_display($panel_page, $did);

  // Collect a list of contexts required by the arguments on this page.
  $contexts = panels_context_load_contexts($panel_page);

  $display->context = $contexts;
  $display->args = array();

  // Get a list of available content.
  require_once drupal_get_path('module', 'panels') . '/includes/common.inc';
  $content_types = panels_common_get_allowed_types('panels_page', $contexts);

  $output = panels_edit($display, NULL, $content_types);
  if (is_object($output)) {
    // If this is a default page, and was saved, we have to make it real.
    panels_page_set_display($panel_page, $did, $output);

    // And do the drupal_goto that a submit function ordinarily would have.
    $dest = "admin/panels/panel-page/{$panel_page->name}/edit/content";
    if ($did) {
      $dest .= "/$did";
    }
    drupal_goto($dest);
  }

  // Print this with theme('page') so that blocks are disabled while editing a display.
  // This is important because negative margins in common block layouts (i.e, Garland)
  // messes up the drag & drop.
  drupal_set_title(check_plain(panels_page_get_title($panel_page)));
  print theme('page', $output, FALSE);
}

/**
 * Page callback to export a panel page to PHP code.
 */
function panels_page_edit_export($panel_page) {
  if (!is_object($panel_page)) {
    $panel_page = panels_page_load($panel_page);
  }
  drupal_set_title(check_plain($panel_page->title));
  $code = panels_page_export($panel_page);

  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => $panel_page->title,
    '#default_value' => $code,
    '#rows' => $lines,
  );
  return $form;
}

/**
 * Page callback to import a panel page from PHP code.
 */
function panels_page_import_page() {
  if (!empty($_POST['form_id']) && $_POST['form_id'] == 'panels_page_edit_form') {
    $panel_page = unserialize($_SESSION['pp_import']);
    drupal_set_title(t('Import panel page "@s"', array('@s' => $panel_page->title)));
    return drupal_get_form('panels_page_edit_form', $panel_page);
  }

  return drupal_get_form('panels_page_import_form');
}

/**
 * Form to for the panel page import
 */
function panels_page_import_form() {
  $form['panel_page'] = array(
    '#type' => 'textarea',
    '#title' => t('Panel page code'),
    '#cols' => 60,
    '#rows' => 15,
    '#description' => t('Cut and paste the results of an exported panel page here.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  $form['#redirect'] = FALSE;
  return $form;
}

/**
 * Handle the submit button on importing a panel page.
 */
function panels_page_import_form_submit($form_id, $form) {
  ob_start();
  eval($form['panel_page']);
  ob_end_clean();

  if (isset($page)) {
    drupal_set_title(t('Import panel page "@s"', array('@s' => $page->title)));
    // As $page contains non-stdClass objects,
    // it needs to be serialized before being stored in the session variable.
    $_SESSION['pp_import'] = serialize($page);
    $output = drupal_get_form('panels_page_edit_form', $page);
    print theme('page', $output);
    exit;
  }
  else {
    drupal_set_message(t('Unable to get a panel page out of that.'));
  }
}

function panels_page_preview_page() {
  $args = func_get_args();
  $panel_page = array_shift($args);
}

function panels_page_admin_view($panel_page, $args) {
  $form = array();

  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Administrative view: enter arguments'),
    '#collapsible' => TRUE,
  );

  $required_args = strpos($panel_page->path, '%') === false ? 0 : count(split($panel_page->path, '%'));

  $url = panels_page_get_url($panel_page, $args);
  if ($args_missing = max($required_args - count($args), 0)) {
    $value = $url;
    $description = format_plural($args_missing,
      t('1 argument is missing to generate a valid URL.'),
      t('%count arguments are missing to generate a valid URL.', array('%count' => $args_missing))
    );
  }
  else {
    $value = l($url, $url);
    $description = t('Click to see the real panel page.');
  }
  $form['fieldset']['url'] = array(
    '#type' => 'item',
    '#title' => t('Real panel URL'),
    '#value' => $value,
    '#description' => $description,
  );

  $contexts = panels_context_load_contexts($panel_page);
  $count = 0;
  foreach ($contexts as $id => $context) {
    if (substr($id, 0, 8) == 'argument') {
      $required = ($count + 1 <= $required_args) ? TRUE : FALSE;
      $description = $required ? t('Required argument (occupies %-placeholder number !position)', array('!position' => $count + 1)) : t('Optional argument');
      $form['fieldset']["arg$count"] = array(
        '#type' => 'textfield',
        '#title' => check_plain($context->identifier),
        '#default_value' => isset($args[$count]) ? $args[$count] : '',
        '#required' => $required,
        '#description' => $description,
      );
    }
    if (isset($args[$count])) {
      unset($args[$count]);
    }
    $count++;
  }

  $add = '';
  if ($args) {
    $add = implode('/', $args);
  }
  $form['fieldset']['additional'] = array(
    '#type' => 'textfield',
    '#title' => t('Additional arguments'),
    '#description' => t('Separated by /'),
    '#default_value' => $add,
  );

  $form['fieldset']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change arguments'),
  );

  $form['panel_page'] = array(
    '#type' => 'value',
    '#value' => $panel_page,
  );
  $form['contexts'] = array(
    '#type' => 'value',
    '#value' => $contexts,
  );

  return $form;
}

function panels_page_admin_view_submit($form_id, $form_values) {
  $count = 0;
  $url = "admin/panels/panel-page/" . $form_values['panel_page']->name . '/edit/preview';
  foreach ($form_values['contexts'] as $context) {
    $url .= "/" . $form_values["arg$count"];
    $count++;
  }
  if (!empty($form_values['additional'])) {
    $url .= "/" . $form_values['additional'];
  }
  return $url;
}

