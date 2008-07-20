<?php
// $Id$

/**
 * @file panels_page.menu.inc
 *
 * Functions resposnible for constructing the panels_page menu routing &
 * overriding system.
 */

function panels_page_construct_menu($hook, $items = NULL) {
  // Static so that it gets built during hook_menu() and used during
  // hook_menu_alter()
  static $alter_items, $hold_items = array();
  switch ($hook) {
    case 'menu':
      $ret_items = array();
      $items = _panels_page_construct_menu($alter_items);
      foreach ($alter_items as $path => $item) {
        // if ($alter_items[$path] ) NEED TO THINK THROUGH THE BITMASK SOME MORE
        if ($alter_items[$path] & (PANELS_HAS_FALLBACK_ROUTER | PANELS_IS_PANELS_ROUTER)) {
          // then ditch this one
          $hold_items[$path] = $item;
          unset($items[$path]);
        }
        else {
          $ret_items[$path] = $item;
        }
      }
      return $items;
    
    case 'menu_alter':
      return !empty($alter_items) ? $alter_items : FALSE;
    
/* No use case for this yet, but here as a reminder/suggestion jic
    case 'menu_link_alter':
      break;*/
  }
}

function _panels_page_construct_menu(&$alter_items) {
  $items = panels_page_create_menu_structure($alter_items);
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
    'access arguments' => TRUE,
    'file' => 'panels_page.ajax.inc',
    'page callback' => 'panels_page_ajax_passthru',
    'type' => MENU_CALLBACK,
  );
  
  // General callbacks not specific to any panels_page
  $items['admin/panels/panels-page/'] = array(
    'title' => 'Panels pages',
    'page callback' => 'panels_page_list_page',
    'type' => MENU_NORMAL_ITEM,
  ) + $admin;
  $items['admin/panels/panels-page/list'] = array(
    'title' => 'List',
    'type' => MENU_DEFAULT_LOCAL_TASK,
  ) + $admin;
  $items['admin/panels/panels-page/add'] = array(
    'title' => 'Add',
    'page callback' => 'panels_page_add_page',
  ) + $admin;
  $items['admin/panels/panels-page/import'] = array(
    'title' => 'Import',
    'page callback' => 'panels_page_import_page',
  ) + $admin;
  $items['admin/panels/panels-page/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_settings_page',
    'weight' => 5,
  ) + $admin;
  // This is technically an outlier in terms of the URL/presence of a loader,
  // but putting it here b/c it doesn't belong in the potentially-reusable
  // definitions in panels_page_admin_menu_items().
  $items['admin/panels/panels-page/%panels_page_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'panels_page_edit_export',
    'page arguments' => array(3),
    'weight' => 4,
  ) + $admin;
  
  // Callbacks for the editing of specific panels_pages

  // Ajax callback responders
  $items['panels/argument/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_argument_add'),
  ) + $ajax;
  $items['panels/argument/ajax/edit'] = array(
    'page arguments' => array('panels_page_ajax_argument_edit'),
  ) + $ajax;
  
  $items['panels/relationship/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_relationship_add'),
  ) + $ajax;
  $items['panels/relationship/ajax/edit'] = array(
    'page arguments' => array('panels_page_ajax_relationship_edit'),
  ) + $ajax;

  $items['panels/context/ajax/add'] = array(
    'page arguments' => array('panels_page_ajax_context_add'),
  ) + $ajax;
  $items['panels/context/ajax/edit'] = array(
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
  
  $items['admin/panels/panels-page/%panels_page_admin'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit',
    'weight' => -10,
    'type' => MENU_CALLBACK,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/settings'] = array(
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
  $items['admin/panels/panels-page/%panels_page_admin/advanced'] = array(
    'title' => 'Advanced',
    'page callback' => 'panels_page_edit_advanced',
    'weight' => -8,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/context'] = array(
    'title' => 'Context',
    'page callback' => 'panels_page_edit_context',
    'weight' => -6,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/layout'] = array(
    'title' => 'Layout',
    'page callback' => 'panels_page_edit_layout_settings',
    'load arguments' => array('%map'),
    'weight' => -4,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/layout_settings'] = array(
    'title' => 'Layout settings',
    'page callback' => 'panels_page_edit_layout_settings',
    // 'load arguments' => array('%map'),
    'weight' => -2,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/content'] = array(
    'title' => 'Content',
    'page callback' => 'panels_page_edit_content',
    // 'load arguments' => array('%map'),
    'weight' => 0,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/preview'] = array(
    'title' => 'Preview',
    'page callback' => 'panels_page_preview_page',
    'weight' => 2,
  ) + $admin;
  $items['admin/panels/panels-page/%panels_page_admin/export'] = array(
    'title' => 'Export',
    'page callback' => 'panels_page_edit_export',
    'weight' => 4,
  ) + $admin;
}


function panels_page_create_menu_structure(&$alter_items) {
  $items = array();
  $panels = panels_page_load_all();
  foreach ($panels as $panel_page) {
    if (empty($panel_page->disabled)) {
      $map = explode('/', $panel_page->path);
      if (strpos($panel_page->path, '%') === FALSE) {
        // TODO note - I got rid of the fourth parameter on these, it seemed to be doing nothing.
        $alter_items[$panel_page->path] = panels_page_construct_static_menu_router($items, $panel_page, $map);
      }
      else {
        $alter_items[preg_replace('%', '%panels_page_render', $panel_page->path, 1)] = panels_page_construct_wildcard_menu_router($items, $panel_page, $map);
      }
    }
  }
  return $items;
}

function panels_page_construct_wildcard_menu_router(&$items, $panel_page, $map) {
  // Replace ONLY the first wildcard entry in the path; the rest are extra
  // loader args.
  $path = preg_replace('%', '%panels_page_render', $panel_page->path, 1);
  $wildcards = array_keys($map, '%');
  $primary_wildcard = array_shift($wildcards);
  $type = _panels_page_menu_type($panel_page);
  // TODO if we're going to allow multiple panels pages per wildcard URL (to be
  // split using context), we may need to insert a check here to another
  // function that handles the logic for that. Certainly we'll need that
  // function elsewhere; in the current system, we'd need it in the validation
  // function of the settings page, and to become part of the argument ajax
  // config modal's calculations about available entities to switch on.
  //
  // (UPDATE - the data from panels_page_check_router_status() actually covers
  // the above need)
  
  // Construct the dynamic menu router item.
  $items[$path] = _panels_page_construct_menu_link($panel_page, array($primary_wildcard), array('arguments' => array($primary_wildcard)), $type);
  // FIXME I don't grok this part. When does it come into play? I suspect that
  // most of this is deprecated with the new menu system, or at least should be
  // handled elsewhere.
  if ($type == MENU_DEFAULT_LOCAL_TASK && dirname($path) && dirname($path) != '.') {
    // FIXME this is currently completely borked - if we end up inside this
    // control statement, everything will break.
    _panels_page_construct_parent_menu_router($items, $panel_page, $path, $access);
  }
  
  // Save this path/pid's router bitval status in a parallel array that we can
  // check later for updating Panels' own menu router table.
  return panels_page_check_router_status($panel_page->path, (int) $panel_page->pid);
}

/**
 * Build a panels_page menu entry for a static panels_page.
 */
function panels_page_construct_static_menu_router(&$items, $panel_page, $map) {
  $type = _panels_page_menu_type($panel_page);
  $items[$panel_page->path] = array(
    'title' => filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'access arguments' => TRUE,
    'page callback' => 'panels_page_view_static_page',
    // FIXME can we figger out how to slice in the $admin arg for
    // panels_page_view_page() from here? Not really necessary, we can do it later too
    'page arguments' => array('static', $panel_page->name),
    'type' => $type,
  );
  // Save this path/pid's router bitval status in a parallel array that we can
  // check later for updating Panels' own menu router table.
  return panels_page_check_router_status($panel_page->path, (int) $panel_page->pid);
}

/**
 * Create a parent menu item for a panel page.
 */
function _panels_page_construct_parent_menu_router(&$items, $panel_page, $path, $access) {
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
    $items[$path] = _panels_page_menu_item($path, $title, $panel_page, $args, $access, $parent_type, $weight);
  }
}

/**
 * Helper function to create a menu item for a panel.
 */
function _panels_page_construct_menu_link($panel_page, $page_arguments, $access, $type, $title = NULL, $weight = NULL) {
  $item = array(
    // 'title' => !empty($title) ? $title : filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'access callback' => !empty($access['callback']) ? $access['callback'] : 'panels_page_access',
    'access arguments' => $access['arguments'],
    'load arguments' => array('%map', '%index'),
    'page callback' => 'panels_page_view_page',
    // FIXME can we figger out how to slice in the $admin arg for
    // panels_page_view_page() from here? Not really necessary, we can do it later too
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