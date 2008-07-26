<?php
// $Id$

/**
 * @file panels_page.menu.inc
 *
 * Functions resposnible for constructing the panels_page menu routing &
 * overriding system.
 */

function _panels_page_menu($for_alter = FALSE) {
  // $items is built during hook_menu() and re-used during hook_menu_alter().
  static $items = array();
  if (empty($items)) {
    $items = _panels_page_construct_menu();
  }
  return $for_alter ? $items : $items['menu_items'];
}

function _panels_page_menu_alter(&$callbacks) {
  $items = _panels_page_menu(TRUE);
  $panels_wildcards = $matches = array();
  foreach ($items['menu_alter'] as $path => $data) {
    $panels_wildcards[$data['raw path']] = $data['pid'];
  }
  
  foreach ($callbacks as $path => $item) {
    // Skip this item if it's ours, or if there's no wildcard.
    if ($item['module'] == 'panels_page' || strpos($path, '%') === FALSE) {
      continue;
    }
    $count = 0;
    // Since we can't rely on all wildcard args always having loaders (e.g.,
    // taxonomy) we normalize all the data down to raw paths for comparison.
    $raw_path = preg_replace('/%([a-z_]*)/', '%', $path, -1, $count);
    if (isset($panels_wildcards[$raw_path])) {
      $matches[$raw_path]['map']   = explode('/', $path);
      $matches[$raw_path]['count'] = $count;
      $matches[$raw_path]['pid']   = $panels_wildcards[$raw_path];
    }
  }
  
  // No matches, so we bug out. TODO this is probably premature
  if (empty($matches)) {
    return;
  }
  foreach ($matches as $raw_path => $match) {
    
  }
}

function panels_page_store_router($router_item) {
  $schema = panels_page_router_table_query_fields();
  $f = $q = $v = array();
  foreach ($schema as $field => $placeholder) {
    $f[] = $field;
    $q[] = $placeholder;
    $v[] = $router_item[strreplace('_', ' ', $field)];
  }
  db_query('INSERT INTO {panels_page_router_store} (' . implode(', ', $f) . ') VALUES (' . implode(', ', $q) . ')', $v);
}

function _panels_page_construct_menu() {
  $items = panels_page_create_menu_structure();
  panels_page_admin_menu_items($items);
  
  $admin = array(
    // FIXME is 'create panel-pages' the best name for this perm?
    // TODO we'll need to granulate this perm significantly in panels3.
    'access arguments' => array('create panel-pages'),
    'file' => 'panels_page.admin.inc',
    'type' => MENU_LOCAL_TASK,
  );
  
  // TODO can i do it this way and have all the funcs in scope? check later
  $ajax = array(
    'access callback' => TRUE,
    'file' => 'panels_page.ajax.inc',
    'page callback' => 'panels_page_ajax_passthru',
    'type' => MENU_CALLBACK,
  );
  
  // General callbacks not specific to any panels_page
  $items['menu_items']['admin/panels/panels-page'] = array(
    'title' => 'Panels Pages',
    'page callback' => 'panels_page_list_page',
    'type' => MENU_NORMAL_ITEM,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/list'] = array(
    'title' => 'List',
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -10,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_settings_page',
    'weight' => -5,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/add'] = array(
    'title' => 'Add',
    'page callback' => 'panels_page_add_page',
    'weight' => 0,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/import'] = array(
    'title' => 'Import',
    'page callback' => 'panels_page_import_page',
    'weight' => 5,
  ) + $admin;
  // This is technically an outlier in terms of the URL/presence of a loader,
  // but putting it here b/c it doesn't belong in the potentially-reusable
  // definitions in panels_page_admin_menu_items().
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'panels_page_edit_export',
    'page arguments' => array(3),
    'weight' => 4,
  ) + $admin;
  
  // Callbacks for the editing of specific panels_pages

  // Ajax callback responders
  $items['menu_items']['panels/argument/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_argument_add'),
  ) + $ajax;
  $items['menu_items']['panels/argument/ajax/edit'] = array(
    'page arguments' => array('panels_page_ajax_argument_edit'),
  ) + $ajax;
  
  $items['menu_items']['panels/relationship/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_relationship_add'),
  ) + $ajax;
  $items['menu_items']['panels/relationship/ajax/edit'] = array(
    'page arguments' => array('panels_page_ajax_relationship_edit'),
  ) + $ajax;

  $items['menu_items']['panels/context/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_context_add'),
  ) + $ajax;
  $items['menu_items']['panels/context/ajax/edit'] = array(
    'page arguments' => array('panels_page_ajax_context_edit'),
  ) + $ajax;
  return $items;
}

function panels_page_admin_menu_items(&$items, $path_prefix = NULL) {
  // TODO path prefix may be superfluous
  if (is_null($path_prefix)) {
    $path_prefix = 'admin/panels/panels-page';
  }
  $admin = array(
    'access arguments' => array('create panel-pages'),
    'file' => 'panels_page.admin.inc',
    'page arguments' => array(3),
    'type' => MENU_LOCAL_TASK,
  );
  
  $items['menu_items'][$path_prefix . '/%panels_page_admin'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit',
    'weight' => -10,
    'type' => MENU_CALLBACK,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/settings'] = array(
    'title' => 'Settings',
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );
// Alternate method for organizing the settings/advanced tabs. I think this is more
// intuitive; commenting for now until the menu work is done.
/*
  $items['admin/panels/panels-page/%panels_page_admin/settings/general'] = array(
    'title' => 'General Settings',
    'page callback' => 'panels_page_edit',
    'page arguments' => array(3),
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );
  $items['admin/panels/panels-page/%panels_page_admin/settings/advanced'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit_advanced',
    'page arguments' => array(3),
    'weight' => -8,
  );
*/
  $items['menu_items'][$path_prefix . '/%panels_page_admin/advanced'] = array(
    'title' => 'Advanced',
    'page callback' => 'panels_page_edit_advanced',
    'weight' => -8,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/context'] = array(
    'title' => 'Context',
    'page callback' => 'panels_page_edit_context',
    'weight' => -6,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/layout'] = array(
    'title' => 'Layout',
    'page callback' => 'panels_page_edit_layout_settings',
    'load arguments' => array('%map'),
    'weight' => -4,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/layout_settings'] = array(
    'title' => 'Layout settings',
    'page callback' => 'panels_page_edit_layout_settings',
    // 'load arguments' => array('%map'),
    'weight' => -2,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/content'] = array(
    'title' => 'Content',
    'page callback' => 'panels_page_edit_content',
    // 'load arguments' => array('%map'),
    'weight' => 0,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/preview'] = array(
    'title' => 'Preview',
    'page callback' => 'panels_page_preview_page',
    'weight' => 2,
  ) + $admin;
  $items['menu_items'][$path_prefix . '/%panels_page_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'panels_page_edit_export',
    'weight' => 4,
  ) + $admin;
}


function panels_page_create_menu_structure() {
  $items['menu_items'] = $items['menu_alter'] = $items['menu_link_alter'] = $items = array();
  $panels = panels_page_load_all();
  foreach ($panels as $panel_page) {
    if (empty($panel_page->disabled)) {
      $map = explode('/', $panel_page->path);
      if (strpos($panel_page->path, '%') === FALSE) {
        // TODO note - I got rid of the fourth parameter on these, it seemed to be doing nothing.
        panels_page_construct_static_menu_link($items, $panel_page, $map);
      }
      else {
        panels_page_construct_wildcard_menu_link($items, $panel_page, $map);
      }
    }
  }
  return $items;
}

function panels_page_construct_wildcard_menu_link(&$items, $panel_page, $map) {
  // Replace ONLY the first wildcard entry in the path; the rest are extra
  // loader args.
  $path = preg_replace('/%/', '%panels_page_render', $panel_page->path, 1);
  
  $wildcards = array_keys($map, '%');
  $items['menu_alter'][$path]['pid']              = $panel_page->pid;
  $items['menu_alter'][$path]['raw path']         = $panel_page->path;
  $items['menu_alter'][$path]['wildcards']        = $wildcards;
  $items['menu_alter'][$path]['primary wildcard'] = $primary_wildcard = array_shift($wildcards);
  // TODO possibly superfluous
  // $items['menu_alter'][$path]['native loader path'] = panels_page_native_loader_path($panel_page);
  $type = _panels_page_menu_type($panel_page);
  
  // Construct the dynamic menu router item. If/when we get to multiple
  // panels_pages per wildcard path, we needn't worry about overwriting here.
  $items['menu_items'][$path] = _panels_page_construct_wildcard_menu_link($panel_page, array($primary_wildcard), array('arguments' => $primary_wildcard), $type);
  $items['menu_link_alter'][$path]['type'] = $type;
  // $items['menu_link_alter'][$path]['type']
  // FIXME parents are borked
  _panels_page_construct_parent_menu_item($items, $panel_page, $path, $type);
}

/**
 * Helper function to create a menu item for a panel.
 */
function _panels_page_construct_wildcard_menu_link($panel_page, $page_arguments, $access, $type, $weight = 0) {
  return array(
    'title callback' => 'panels_page_get_title',
    'title arguments' => $page_arguments,
    // TODO $access['callback'] is currently always empty. Eliminate this ternary if we don't find a need for it presently.
    'access callback' => !empty($access['callback']) ? $access['callback'] : 'panels_page_access',
    'access arguments' => (array)$access['arguments'],
    'load arguments' => array('%map', '%index'),
    'page callback' => 'panels_page_render_handler',
    'page arguments' => $page_arguments,
    'type' => $type,
    'weight' => $weight,
  );
}

/**
 * Build a panels_page menu entry for a static panels_page.
 */
function panels_page_construct_static_menu_link(&$items, $panel_page, $map) {
  $type = _panels_page_menu_type($panel_page);
  $items['menu_items'][$panel_page->path] = array(
    'title' => filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'access callback' => 'panels_page_access',
    'access arguments' => array('static', $panel_page->name), // FIXME this is a ph
    'page callback' => 'panels_page_view_static_page',
    'page arguments' => array('static', $panel_page->name),
    'type' => $type,
  );
  _panels_page_construct_parent_menu_item($items, $panel_page, $path, $type);
}

/**
 * Create a parent menu item for a panel page.
 */
function _panels_page_construct_parent_menu_item(&$items, $panel_page, $path, $type) {
  if ($type == MENU_DEFAULT_LOCAL_TASK && dirname($path) && dirname($path) != '.') {
    // FIXME this is currently completely borked - if we end up inside this
    // control statement, everything will break. However, we should also be eliminating
    // the statement later.
    switch ($panel_page->menu_tab_default_parent_type) {
      case 'tab':
        $parent_type = MENU_LOCAL_TASK;
        break;
  
      case 'normal':
        $parent_type = MENU_NORMAL_ITEM;
        break;
  
      default:
      case 'existing':
        $parent_type = 0;
        break;
    }
    if ($parent_type) {
      $title   = filter_xss_admin(panels_page_get_title($panel_page, 'menu-parent'));
      $weight  = $panel_page->menu_parent_tab_weight;
      // FIXME this function doesn't even exist anymore.
      $items[$path] = _panels_page_menu_item($path, $title, $panel_page, $args, $access, $parent_type, $weight);
    }
  }
}

/**
 * Determine what menu type a panel needs to use.
 */
function _panels_page_menu_type($panel_page) {
  if ($panel_page->menu) {
    if ($panel_page->menu_tab_default) {
      $type = MENU_DEFAULT_LOCAL_TASK;
    }
    else if ($panel_page->menu_tab) {
      $type = MENU_LOCAL_TASK;
    }
    else {
      $type = MENU_NORMAL_ITEM;
    }
  }
  else {
    $type = MENU_CALLBACK;
  }
  return $type;
}

function panels_page_router_table_query_fields() {
  static $query_building_blocks = array();
  if (empty($query_building_blocks)) {
    $schema = drupal_get_schema('panels_page_router_store');
    foreach ($schema['fields'] as $field => $data) {
      $query_building_blocks[$field] = db_type_placeholder($data['type']);
    }
  }
  return $query_building_blocks;
}