<?php
// $Id$

/**
 * @file panels_page.write.inc
 *
 * Functions responsible for writing panels_page data to the database.
 */

/**
 * Fetch all panel pages in the system.
 *
 * For the moment, this function does not cache. It may be integrated into the
 * panels_page_loaded_cache() caching mechanism at some point, but its utility
 * in D6 is significantly lessened vs. D5, so there is much less need to do so.
 */
function panels_page_load_all($names = array(), $page_size = 0) {
  $pages = $dids = array();
  $query = "SELECT * FROM {panels_page}";

  if (!empty($names)) {
    $query .= " WHERE name IN (" . implode(', ', array_fill(0, count($names), "'%s'")) . ")";
  }

  if ($page_size) {
    $result = pager_query($query, $page_size, 0, $names);
  }
  else {
    $result = db_query($query, $names);
  }

  while ($page = db_fetch_object($result)) {
    $page->access           = ($page->access ? explode(', ', $page->access) : array());
    $page->arguments        = (!empty($page->arguments)) ? unserialize($page->arguments) : array();
    $page->displays         = (!empty($page->displays)) ? unserialize($page->displays) : array();
    $page->contexts         = (!empty($page->contexts)) ? unserialize($page->contexts) : array();
    $page->relationships    = (!empty($page->relationships)) ? unserialize($page->relationships) : array();
    $page->switcher_options = (!empty($page->switcher_options)) ? unserialize($page->switcher_options) : array();
    $page->type             = t('Local');
    $page->loader_flags     = (int) $page->loader_flags;

    $pages[$page->name]     = panels_page_sanitize($page);
  }

  $status = variable_get('panel_page_defaults', array());
  foreach (panels_page_default_panels() as $page) {
    // Determine if default panel is enabled or disabled.
    if (isset($status[$page->name])) {
      $page->disabled = $status[$page->name];
    }

    if (!empty($pages[$page->name])) {
      $pages[$page->name]->type = t('Overridden');
    }
    else {
      $page->type = t('Default');
      $pages[$page->name] = $page;
    }
  }
  return $pages;
}

/**
 * Load a panel page.
 */
function panels_page_load($id, $load_display = FALSE) {
  $cache = panels_page_loaded_cache($id);

  if ($cache) {
    if ($load_display && empty($cache->display)) {
      $cache->display = panels_load_display($cache->did);
      panels_page_loaded_cache($id, $cache);
    }
    return $cache;
  }

  if (!is_numeric($id)) {
    $where = "name = '%s'";
  }
  else {
    $where = 'pid = %d';
  }
  $page = db_fetch_object(db_query("SELECT * FROM {panels_page} WHERE $where", $id));
  if (!$page) {
    $defaults = panels_page_default_panels();
    if (isset($defaults[$id])) {
      // Default panel pages will always only be identified by name, so no need
      // for the both-ids-point-to-same-object trick. And, they're in code, so
      // no real need to statically cache them, either.
      $page = $defaults[$id];
      return $page;
    }
    return;
  }

  $page->access           = ($page->access ? explode(', ', $page->access) : array());
  $page->arguments        = (!empty($page->arguments)) ? unserialize($page->arguments) : array();
  $page->displays         = (!empty($page->displays)) ? unserialize($page->displays) : array();
  $page->contexts         = (!empty($page->contexts)) ? unserialize($page->contexts) : array();
  $page->relationships    = (!empty($page->relationships)) ? unserialize($page->relationships) : array();
  $page->switcher_options = (!empty($page->switcher_options)) ? unserialize($page->switcher_options) : array();
  $page->loader_flags     = (int) $page->loader_flags;

  if ($load_display) {
    $page->primary = panels_load_display($page->did);
  }

  panels_page_loaded_cache($id, $page);
  return $page;
}

function panels_page_loaded_cache($id, $panel_page = NULL) {
  static $cache = array();
  if (is_object($panel_page)) {
    $cache[$id] = $panel_page;
    // Make sure that we've statically cached the loaded page for both possible
    // unique identifiers - $page->pid AND $page->name.
    $other_id = is_numeric($id) ? $panel_page->name : $panel_page->pid;
    $cache[$other_id] =& $cache[$id];
  }
  return array_key_exists($id, $cache) ? $cache[$id] : FALSE;
}


/**
 * Get all 'default' panels.
 *
 * @ingroup HookInvokers
 */
function panels_page_default_panels() {
  $panels = module_invoke_all('default_panel_pages');
  if (!is_array($panels)) {
    $panels = array();
  }

  return $panels;
}


function panels_page_set_current(&$panel_page) {
  static $cache = NULL;
  // Only allow it to be set once per request.
  if (!isset($cache) && isset($panel_page)) {
    $cache =& $panel_page;
  }
  return $cache;
}

/**
 * Internal panels_page API function; directs the current display
 * ($panel_page->display) to the primary display ($panel_page->primary),
 * loading the primary display if necessary.
 *
 * @param object $panel_page
 */
function panels_page_fetch_primary_display(&$panel_page) {
  if (!is_a($panel_page->primary, 'panels_display')) {
    $panel_page->primary            = panels_load_display($panel_page->did);
  }
  $panel_page->display =& $panel_page->primary;
  // Update the cache.
  panels_page_loaded_cache($panel_page->name, $panel_page);
}

/**
 * Load a display based upon information from the $displays list.
 *
 * The family of panels_page_fetch*() functions are specifically dedicated to
 * retrieving a particular display and placing them in $panels_page->display
 * via reference to its permanent home in the $panels_page object. The
 * permanent homes vary:
 *
 *   -# For the primary display, that location is $panels_page->primary.
 *   -# For alternate default displays, that location is in
 *      $panels_page->alternates['defaults'][$did].
 *   -# For alternate non-default displays, that location is in
 *      $panels_page->alternates['all'][$did].
 */
function panels_page_fetch_display(&$panel_page, $id = NULL) {
  // No id is provided - we want the primary display. Get it and get out.
  if (empty($id)) {
    panels_page_fetch_primary_display($panel_page);
    return;
  }

  // FIXME I don't grok this part of the logic, and am assuming this is OK.
  $info = $panel_page->displays[$id];
  if (panels_fetch_display_from_info($panel_page, $info)) {
    return;
  }

  // The requested display does not exist. First, we try to fall back to the
  // default display for the context we're switching on.
  if (!empty($info['default']) && !empty($panel_page->displays[$info['default']])) {
    panels_page_fetch_display_from_info($panel_page, $panel_page->displays[$info['default']]);
  }
  // Otherwise, use the primary display. Load it if it's not already loaded.
  else {
    panels_page_fetch_primary_display($panel_page);
  }
  // For callers who care that we've fallen back on a default.
  return TRUE;
}

/**
 * Get a display based on whether it's already in code or needs to be loaded.
 */
function panels_page_fetch_display_from_info(&$panel_page, $info) {
  $ret = NULL;
  // If the 'display' is set it's the result of an export/default
  if (isset($info['display'])) {
    $ret = $info['display'];
  }

  if (is_numeric($info['did'])) {
    $ret = panels_load_display($info['did']);
  }

  if (is_object($ret)) {
    $panel_page->alternates[$ret->name] = $ret; // By val doesn't matter here
    $panel_page->display =& $panel_page->alternates[$ret->name];
    panels_page_loaded_cache($panel_page->name, $panel_page);
    return TRUE;
  }
  return FALSE;
}

/**
 * Determine if the specified user has access to a panel.
 */
function panels_page_access($panel_page, $account = NULL) {
  if (!$account) {
    global $user;
    $account = $user;
  }

  // Administrator privileges
  if (user_access('access all panel-pages', $account)) {
    return TRUE;
  }

  // All panels displays with an empty access setting are available to all roles.
  if (empty($panel_page->access) || !is_array($panel_page->access)) {
    return TRUE;
  }

  // Otherwise, check roles
  static $roles = array();
  if (!isset($roles[$account->uid])) {
    $roles[$account->uid] = array_keys($account->roles);
    $roles[$account->uid][] = $account->uid ? DRUPAL_AUTHENTICATED_RID : DRUPAL_ANONYMOUS_RID;
  }

  return array_intersect($panel_page->access, $roles[$account->uid]);
}

