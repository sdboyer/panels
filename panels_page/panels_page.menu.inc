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
  $panels_wildcards = $wildcard_callbacks = array();
  foreach ($callbacks as $path => $item) {
    // Skip this item if it's something we ourselves implemented.
    if ($item['module'] == 'panels_page') {
      continue;
    }
    // Add this item if there's a wildcard.
    if (strpos($path, '%') !== FALSE) {
      $count = 0;
      // Since we can't rely on all our args always having loaders (e.g., taxo)
      // we need to normalize all data down to the raw path for comparison.
      $raw_path = preg_replace('/%([a-z_]*)/', '%', $path, -1, $count);
      $wildcard_callbacks[$raw_path]['map'] = explode('/', $path);
      $wildcard_callbacks[$raw_path]['count'] = $count;
    }
  }
  
  foreach ($items['metadata'] as $path => $data) {
    $panels_wildcards[$data['raw path']] = $data['pid'];
  }
    
  $matches = array_intersect(array_keys($wildcard_callbacks), array_keys($panels_wildcards));
  if (empty($matches)) {
    // No matches, so we bug out. TODO this is probably premature
    return;
  }
  foreach ($matches as $match) {
    
  }
/*
  $router_entries = $loader_entries = array();
  $result = db_query("SELECT * FROM {panels_page_menu}");
  while ($row = db_fetch_object($result)) {
    $router_entries[$row->pid] = $row;
  }
  
  $result = db_query("SELECT * FROM {panels_page_menu_loader}");
  while ($row = db_fetch_object($result)) {
    $router_entries[$row->pid] = $row;
  }
  */
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

function panels_page_admin_menu_items(&$items) {
  $admin = array(
    'access arguments' => array('create panel-pages'),
    'file' => 'panels_page.admin.inc',
    'page arguments' => array(3),
    'type' => MENU_LOCAL_TASK,
  );
  
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit',
    'weight' => -10,
    'type' => MENU_CALLBACK,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/settings'] = array(
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
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/advanced'] = array(
    'title' => 'Advanced',
    'page callback' => 'panels_page_edit_advanced',
    'weight' => -8,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/context'] = array(
    'title' => 'Context',
    'page callback' => 'panels_page_edit_context',
    'weight' => -6,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/layout'] = array(
    'title' => 'Layout',
    'page callback' => 'panels_page_edit_layout_settings',
    'load arguments' => array('%map'),
    'weight' => -4,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/layout_settings'] = array(
    'title' => 'Layout settings',
    'page callback' => 'panels_page_edit_layout_settings',
    // 'load arguments' => array('%map'),
    'weight' => -2,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/content'] = array(
    'title' => 'Content',
    'page callback' => 'panels_page_edit_content',
    // 'load arguments' => array('%map'),
    'weight' => 0,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/preview'] = array(
    'title' => 'Preview',
    'page callback' => 'panels_page_preview_page',
    'weight' => 2,
  ) + $admin;
  $items['menu_items']['admin/panels/panels-page/%panels_page_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'panels_page_edit_export',
    'weight' => 4,
  ) + $admin;
}


function panels_page_create_menu_structure() {
  $items['menu_items'] = $items['metadata'] = $items = array();
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
  $items['metadata'][$path]['pid']                = $panel_page->pid;
  $items['metadata'][$path]['raw path']           = $panel_page->path;
  $items['metadata'][$path]['wildcards']          = implode('-', $wildcards);
  // TODO possibly superfluous
  $items['metadata'][$path]['primary wildcard']   = $primary_wildcard = array_shift($wildcards);
  // TODO possibly superfluous
  $items['metadata'][$path]['native loader path'] = panels_page_native_loader_path($panel_page);
  $type = _panels_page_menu_type($panel_page);
  
  // Construct the dynamic menu router item. If/when we get to multiple
  // panels_pages per wildcard path, we needn't worry about overwriting here.
  $items['menu_items'][$path] = _panels_page_construct_wildcard_menu_link($panel_page, array($primary_wildcard), array('arguments' => $primary_wildcard), $type);
  // FIXME parents are borked
  _panels_page_construct_parent_menu_item($items, $panel_page, $path, $type);
}

/**
 * Helper function to create a menu item for a panel.
 */
function _panels_page_construct_wildcard_menu_link($panel_page, $page_arguments, $access, $type, $title = NULL, $weight = NULL) {
  $item = array(
    'title callback' => !empty($title) ? $title : filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'title arguments' => $page_arguments,
    // TODO $access['callback'] is currently always empty. Eliminate this ternary if we don't find a need for it presently.
    'access callback' => !empty($access['callback']) ? $access['callback'] : 'panels_page_access',
    'access arguments' => (array)$access['arguments'],
    'load arguments' => array('%map', '%index'),
    'page callback' => 'panels_page_view_handler',
    'page arguments' => $page_arguments,
    'type' => $type,
  );
  
  if ($weight !== NULL) {
    $item['weight'] = $weight;
  }
  if ($title !== NULL) {
    $item['title'] = $weight;
  }
  return $item;
}

/**
 * Build a panels_page menu entry for a static panels_page.
 */
function panels_page_construct_static_menu_link(&$items, $panel_page, $map) {
  $type = _panels_page_menu_type($panel_page);
  $items['menu_items'][$panel_page->path] = array(
    'title' => filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'access callback' => 'panels_page_access',
    'access arguments' => array('static'), // FIXME this is a ph
    'page callback' => 'panels_page_view_static_page',
    'page arguments' => array($panel_page->name),
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