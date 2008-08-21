<?php
// $Id$

/**
 * @file panels_page.menu.inc
 *
 * Functions resposnible for constructing the panels_page menu routing &
 * overriding system.
 */

function panels_page_admin_static_menu_items() {
  $items = array();
  $admin = array(
    // TODO is 'create panel-pages' still the best name for this perm?
    // TODO we'll need to granulate this perm significantly in panels3.
    'access arguments' => array('create panel-pages'),
    'file' => 'panels_page.admin.inc',
    'type' => MENU_LOCAL_TASK,
  );

  $items['admin/panels/panel-page'] = array(
    'title' => 'Panels Pages',
    'page callback' => 'panels_page_list_page',
    'type' => MENU_NORMAL_ITEM,
  ) + $admin;
  $items['admin/panels/panel-page/list'] = array(
    'title' => 'List',
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'weight' => -10,
  ) + $admin;
  $items['admin/panels/panel-page/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_settings',
    'weight' => -5,
  ) + $admin;
  $items['admin/panels/panel-page/add'] = array(
    'title' => 'Add',
    'page callback' => 'panels_page_add_handler',
    'weight' => 0,
  ) + $admin;
  $items['admin/panels/panel-page/import'] = array(
    'title' => 'Import',
    'page callback' => 'panels_page_import_page',
    'weight' => 5,
  ) + $admin;
  return $items;
}

function panels_page_admin_dynamic_menu_items($items = array(), $path_prefix = NULL) {
  // TODO allowing a dynamic path prefix may be superfluous
  if (is_null($path_prefix)) {
    $path_prefix = 'admin/panels/panel-page/';
  }
  $loader_arg = count(explode('/', $path_prefix)) - 1;

  $admin = array(
    'access arguments' => array('create panel-pages'),
    'file' => 'panels_page.admin.inc',
    'page arguments' => array($loader_arg),
    'type' => MENU_LOCAL_TASK,
  );

  $items[$path_prefix . '%panels_page_admin/edit'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit',
    'weight' => -10,
    'type' => MENU_CALLBACK,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin/edit/settings'] = array(
    'title' => 'Settings',
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  ) + $admin;

// Alternate method for organizing the settings/advanced tabs. I think this is more
// intuitive; commenting for now until the menu work is done.
/*
  $items['admin/panels/panel-page/%panels_page_admin/settings/general'] = array(
    'title' => 'General Settings',
    'page callback' => 'panels_page_edit',
    'page arguments' => array(3),
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
  );
  $items['admin/panels/panel-page/%panels_page_admin/settings/advanced'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_page_edit_advanced',
    'page arguments' => array(3),
    'weight' => -8,
  );
*/
  $items[$path_prefix . '%panels_page_admin/edit/advanced'] = array(
    'title' => 'Advanced',
    'page callback' => 'panels_page_edit_advanced',
    'weight' => -8,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin_cache/edit/context'] = array(
    'title' => 'Context',
    'page callback' => 'panels_page_edit_context',
    'weight' => -6,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin/edit/layout'] = array(
    'title' => 'Layout',
    'page callback' => 'panels_page_edit_layout',
    'load arguments' => array($loader_arg + 4),
    'weight' => -4,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin_cache/edit/layout-settings'] = array(
    'title' => 'Layout settings',
    'page callback' => 'panels_page_edit_layout_settings',
    'load arguments' => array($loader_arg + 4),
    'weight' => -2,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin_cache/edit/content'] = array(
    'title' => 'Content',
    'page callback' => 'panels_page_edit_content',
    'load arguments' => array($loader_arg + 4),
    'weight' => 0,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin/edit/preview'] = array(
    'title' => 'Preview',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('panels_page_preview_page', $loader_arg),
    'weight' => 2,
  ) + $admin;
  $items[$path_prefix . '%panels_page_admin/edit/export'] = array(
    'title' => 'Export',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('panels_page_export_page', $loader_arg),
    'weight' => 4,
  ) + $admin;
  return $items;
}

function panels_page_create_menu_structure($new_items = NULL) {
  static $items = array();
  // The first time through, we _always_ construct the menu tree.
  if (empty($items)) {
    $items = _panels_page_create_menu_structure();
  }
  if (!is_null($new_items)) {
    $items = $new_items;
  }
  return $items;
}

function _panels_page_menu_alter(&$callbacks) {
  $panels_items = panels_page_create_menu_structure();
  $matches = array();
  while (list($path, $item) = each($callbacks)) {
    if (strpos($path, '%') === FALSE) {
      continue;
    }
    $raw_path = panels_page_get_raw_path($path);
    if (isset($panels_items['menu items'][$raw_path])) {
      $panels_items['menu items'][$path] = array_merge($item, $panels_items['menu items'][$raw_path]);
      $matches[$path] = TRUE;
      // TODO this is another point that hardcodes one item per path.
      // Only unset if the raw path is different (e.g. taxonomy isn't)
      if ($raw_path != $path) {
        unset($panels_items['menu items'][$raw_path]);
      }
    }
  }
  // Insert all the overridden routers into our separate table.
  _panels_page_menu_router_build($callbacks, $matches);
  // Replace all the overridden routers with our own.
  $callbacks = array_merge($callbacks, $panels_items['menu items']);

  db_query('DELETE FROM {panels_page_router_quickref}');
  foreach ($panels_items['menu items'] as $path => $item) {
    $raw_path = panels_page_get_raw_path($path);
    // Only save a router quickref if the item has metadata
    if (!empty($panels_items['metadata'][$raw_path])) {
      $metadata = &$panels_items['metadata'][$raw_path];
      $metadata->loader_flags |= isset($matches[$path]) ? PANELS_HAS_FALLBACK_ROUTER : 0;
      db_query('UPDATE {panels_page} SET loader_flags = %d WHERE pid = %d', $metadata->loader_flags, $metadata->pid);
    }
  }
  // Re-call our helper function and store updated items into the static cache.
  panels_page_create_menu_structure($panels_items);
}

function _panels_page_create_menu_structure() {
  panels_load_include('plugins');
  $items = array();
  $panels = panels_page_load_all();
  foreach ($panels as $panel_page) {
    if (empty($panel_page->disabled)) {
      $map = explode('/', $panel_page->path);
      if (strpos($panel_page->path, '%') === FALSE) {
        panels_page_construct_static_menu_link($items, $panel_page, $map);
      }
      else {
        panels_page_construct_dynamic_menu_link($items, $panel_page, $map);
      }
    }
  }
  return $items;
}

function panels_page_construct_dynamic_menu_link(&$items, $panel_page, $map) {
  $type = _panels_page_menu_type($panel_page);
  $primary_wildcard = array_search('%', $map);
  // FIXME (this data) quickref should be entirely absorbed into the panels_page table itself.
  panels_page_construct_menu_item_metadata($items, $panel_page, PANELS_IS_DYNAMIC);

  // Construct the dynamic menu router item. If/when we get to multiple
  // panels_pages per dynamic path, we needn't worry about overwriting here.
  $items['menu items'][$panel_page->path] = _panels_page_construct_dynamic_menu_link(
    $panel_page, array('pid-' . $panel_page->pid, $primary_wildcard),
    array('arguments' => array('pid-' . $panel_page->pid, $primary_wildcard)),
     $type);

  // FIXME parents are borked
  _panels_page_construct_parent_menu_item($items, $panel_page, $panel_page->path, $type);
}

/**
 * Helper function to create a menu item for a panel.
 */
function _panels_page_construct_dynamic_menu_link($panel_page, $page_arguments, $access, $type, $weight = 0) {
  return array(
    'title callback'  => 'panels_page_title_handler',
    'title arguments' => $page_arguments,
    'page callback'   => 'panels_page_render_handler',
    'page arguments'  => $page_arguments,
    'type'            => $type,
    'weight'          => $weight,
    'module'          => 'panels_page',
    'file'            => NULL, // Ensure we don't get the overriddee's file property
  );
}

/**
 * Build a panels_page menu entry for a static panels_page.
 */
function panels_page_construct_static_menu_link(&$items, $panel_page, $map) {
  $type = _panels_page_menu_type($panel_page);
  panels_page_construct_menu_item_metadata($items, $panel_page);

  $items['menu items'][$panel_page->path] = array(
    'title'            => filter_xss_admin(panels_page_get_title($panel_page, 'menu')),
    'access callback'  => 'panels_page_access_handler',
    'access arguments' => array('pid-' . $panel_page->pid),
    'page callback'    => 'panels_page_static_render_handler',
    'page arguments'   => array($panel_page->name),
    'type'             => $type,
    'module'           => 'panels_page',
    'file'             => 'panels_page.render.inc',
  );
  _panels_page_construct_parent_menu_item($items, $panel_page, $panel_page->path, $type);
}

function panels_page_construct_menu_item_metadata(&$items, $panel_page, $flags = 0) {
  $metadata = new stdClass();
  $metadata->pid          = $panel_page->pid;
  $metadata->name         = $panel_page->name;
  $metadata->type         = _panels_page_menu_type($panel_page);
  $metadata->path         = $panel_page->path;
  $metadata->loader_flags = $flags;

  $items['metadata'][$panel_page->path] = $metadata;
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

function panels_page_router_table_query_fields($table) {
  static $query_building_blocks = array();
  if (empty($query_building_blocks[$table])) {
    $schema = drupal_get_schema($table);
    foreach ($schema['fields'] as $field => $data) {
      $query_building_blocks[$table][$field] = db_type_placeholder($data['type']);
    }
  }
  return $query_building_blocks[$table];
}

function panels_page_get_raw_path($path) {
  return preg_replace('/%([a-z_]*)/', '%', $path);
}

/**
 * Modified version of _menu_router_build();
 */
function _panels_page_menu_router_build($callbacks, $matches) {
  $menu = array();
  foreach ($matches as $path => $match) {
    $item = $callbacks[$path];
    $load_functions = array();
    $to_arg_functions = array();
    $fit = 0;
    $move = FALSE;

    $parts = explode('/', $path, MENU_MAX_PARTS);
    $number_parts = count($parts);
    // We store the highest index of parts here to save some work in the fit
    // calculation loop.
    $slashes = $number_parts - 1;
    // Extract load and to_arg functions.
    foreach ($parts as $k => $part) {
      $match = FALSE;
      if (preg_match('/^%([a-z_]*)$/', $part, $matches)) {
        if (empty($matches[1])) {
          $match = TRUE;
          $load_functions[$k] = NULL;
        }
        else {
          if (function_exists($matches[1] .'_to_arg')) {
            $to_arg_functions[$k] = $matches[1] .'_to_arg';
            $load_functions[$k] = NULL;
            $match = TRUE;
          }
          if (function_exists($matches[1] .'_load')) {
            $function = $matches[1] .'_load';
            // Create an array of arguments that will be passed to the _load
            // function when this menu path is checked, if 'load arguments'
            // exists.
            $load_functions[$k] = isset($item['load arguments']) ? array($function => $item['load arguments']) : $function;
            $match = TRUE;
          }
        }
      }
      if ($match) {
        $parts[$k] = '%';
      }
      else {
        $fit |=  1 << ($slashes - $k);
      }
    }
    if ($fit) {
      $move = TRUE;
    }
    else {
      // If there is no %, it fits maximally.
      $fit = (1 << $number_parts) - 1;
    }
    $masks[$fit] = 1;
    $item['load_functions'] = empty($load_functions) ? '' : serialize($load_functions);
    $item['to_arg_functions'] = empty($to_arg_functions) ? '' : serialize($to_arg_functions);
    $item += array(
      'title' => '',
      'weight' => 0,
      'type' => MENU_NORMAL_ITEM,
      '_number_parts' => $number_parts,
      '_parts' => $parts,
      '_fit' => $fit,
    );
    $item += array(
      '_visible' => (bool)($item['type'] & MENU_VISIBLE_IN_BREADCRUMB),
      '_tab' => (bool)($item['type'] & MENU_IS_LOCAL_TASK),
    );
    if ($move) {
      $new_path = implode('/', $item['_parts']);
      $menu[$new_path] = $item;
      $sort[$new_path] = $number_parts;
    }
    else {
      $menu[$path] = $item;
      $sort[$path] = $number_parts;
    }
  }
  array_multisort($sort, SORT_NUMERIC, $menu);
  if (!$menu) {
    // We must have a serious error - there is no data to save.
    watchdog('php', 'The specialized Panels menu router rebuild failed. Some paths may not work correctly.', array(), WATCHDOG_ERROR);
    return array();
  }
  db_query('DELETE FROM {panels_page_router_store}');
  foreach ($menu as $path => $v) {
    $item = &$menu[$path];
    if (!$item['_tab']) {
      // Non-tab items.
      $item['tab_parent'] = '';
      $item['tab_root'] = $path;
    }
    for ($i = $item['_number_parts'] - 1; $i; $i--) {
      $parent_path = implode('/', array_slice($item['_parts'], 0, $i));
      if (isset($menu[$parent_path])) {

        $parent = $menu[$parent_path];

        if (!isset($item['tab_parent'])) {
          // Parent stores the parent of the path.
          $item['tab_parent'] = $parent_path;
        }
        if (!isset($item['tab_root']) && !$parent['_tab']) {
          $item['tab_root'] = $parent_path;
        }
        // If an access callback is not found for a default local task we use
        // the callback from the parent, since we expect them to be identical.
        // In all other cases, the access parameters must be specified.
        if (($item['type'] == MENU_DEFAULT_LOCAL_TASK) && !isset($item['access callback']) && isset($parent['access callback'])) {
          $item['access callback'] = $parent['access callback'];
          if (!isset($item['access arguments']) && isset($parent['access arguments'])) {
            $item['access arguments'] = $parent['access arguments'];
          }
        }
        // Same for page callbacks.
        if (!isset($item['page callback']) && isset($parent['page callback'])) {
          $item['page callback'] = $parent['page callback'];
          if (!isset($item['page arguments']) && isset($parent['page arguments'])) {
            $item['page arguments'] = $parent['page arguments'];
          }
          if (!isset($item['file']) && isset($parent['file'])) {
            $item['file'] = $parent['file'];
          }
          if (!isset($item['file path']) && isset($parent['file path'])) {
            $item['file path'] = $parent['file path'];
          }
        }
      }
    }
    if (!isset($item['access callback']) && isset($item['access arguments'])) {
      // Default callback.
      $item['access callback'] = 'user_access';
    }
    if (!isset($item['access callback']) || empty($item['page callback'])) {
      $item['access callback'] = 0;
    }
    if (is_bool($item['access callback'])) {
      $item['access callback'] = intval($item['access callback']);
    }

    $item += array(
      'access arguments' => array(),
      'access callback' => '',
      'page arguments' => array(),
      'page callback' => '',
      'block callback' => '',
      'title arguments' => array(),
      'title callback' => 't',
      'description' => '',
      'position' => '',
      'tab_parent' => '',
      'tab_root' => $path,
      'path' => $path,
      'file' => '',
      'file path' => '',
      'include file' => '',
    );

    // Calculate out the file to be included for each callback, if any.
    if ($item['file']) {
      $file_path = $item['file path'] ? $item['file path'] : drupal_get_path('module', $item['module']);
      $item['include file'] = $file_path .'/'. $item['file'];
    }

    $title_arguments = $item['title arguments'] ? serialize($item['title arguments']) : '';
    db_query("INSERT INTO {panels_page_router_store}
      (path, load_functions, to_arg_functions, access_callback,
      access_arguments, page_callback, page_arguments, fit,
      number_parts, tab_parent, tab_root,
      title, title_callback, title_arguments,
      type, block_callback, description, position, weight, file)
      VALUES ('%s', '%s', '%s', '%s',
      '%s', '%s', '%s', %d,
      %d, '%s', '%s',
      '%s', '%s', '%s',
      %d, '%s', '%s', '%s', %d, '%s')",
      $path, $item['load_functions'], $item['to_arg_functions'], $item['access callback'],
      serialize($item['access arguments']), $item['page callback'], serialize($item['page arguments']), $item['_fit'],
      $item['_number_parts'], $item['tab_parent'], $item['tab_root'],
      $item['title'], $item['title callback'], $title_arguments,
      $item['type'], $item['block callback'], $item['description'], $item['position'], $item['weight'], $item['include file']);
  }
}
