<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_block_panels_content_types() {
  $items['block'] = array(
    'title' => t('Block'),
    'content_types' => 'panels_admin_content_types_block',
    'render callback' => 'panels_content_block',
    'add callback' => 'panels_admin_add_block',
    'edit callback' => 'panels_admin_edit_block',
    'title callback' => 'panels_admin_title_block',
    'add submit callback' => 'panels_admin_submit_block',
    'edit submit callback' => 'panels_admin_submit_block',
    //'validate callback' => 'panels_admin_validate_block',
  );
  return $items;
}

/**
 * Output function for the 'block' content type. Outputs a block
 * based on the module and delta supplied in the configuration.
 */
function panels_content_block($subtype, $conf) {
  $block = (object) module_invoke($conf['module'], 'block', 'view', $conf['delta']);
  if (empty($block)) {
    return;
  }

  $block->module = $conf['module'];
  $block->delta = $conf['delta'];

  if (user_access('administer blocks')) {
    $block->admin_links = array(
      array(
        'title' => t('Configure block'),
        'alt' => t("Configure this pane's 'block settings' in administer >> site building >> blocks"),
        'href' => "admin/build/block/configure/$block->module/$block->delta",
        'query' => drupal_get_destination(),
      ),
    );
  }

  // This seems extra but it prevents an unnecessary query sometimes.
  if (empty($conf['block_visibility']) && $block->module != 'block') {
    return $block;
  }

  // Test for block visibility
  $result = db_query("SELECT title, pages, visibility FROM {blocks} WHERE module = '%s' AND delta = '%s'", $block->module, $block->delta);
  $block_visibility = db_fetch_object($result);

  if ($block->module == 'block') {
    $block->title = $block_visibility->title;
  }

  if (empty($conf['block_visibility'])) {
    return $block;
  }

  if ($block_visibility && $block_visibility->pages) {
    if ($block_visibility->visibility < 2) {
      $path       = drupal_get_path_alias($_GET['q']);
      $regexp     = '/^('. preg_replace(array('/(\r\n?|\n)/', '/\\\\\*/', '/(^|\|)\\\\<front\\\\>($|\|)/'), array('|', '.*', '\1'. preg_quote(variable_get('site_frontpage', 'node'), '/') .'\2'), preg_quote($block_visibility->pages, '/')) .')$/';
      $page_match = !($block_visibility->visibility xor preg_match($regexp, $path));
    }
    else {
      $page_match = drupal_eval($block_visibility->pages);
    }
  }
  else {
    $page_match = TRUE;
  }

  if ($page_match) {
    return $block;
  }
}

/**
 * Return all block content types available.
 *
 * @ingroup hook_invocations
 *
 * Modules wanting to make special adjustments the way that panels handles their blocks
 * can implement an extension to the hook_block() family, where the function name is
 * of the form "$module . '_panels_block_info'".
 */
function panels_admin_content_types_block() {
  $types = array();
  foreach (module_list() as $module) {
    $module_blocks = module_invoke($module, 'block', 'list');
    if ($module_blocks) {
      foreach ($module_blocks as $delta => $block) {
        // strip_tags used because it goes through check_plain and that
        // just looks bad.
        $info = array(
          'title' => strip_tags($block['info']),
        );

        // Ask around for further information by invoking the hook_block() extension.
        $function = $module . '_panels_block_info';
        if (!function_exists($function)) {
          $function = 'panels_default_block_info';
        }
        $function($module, $delta, $info);

        // this check means modules can remove their blocks; particularly useful
        // if they offer the block some other way (like we do for views)
        if ($info) {
          $types["$module-$delta"] = $info;
        }
      }
    }
  }
  return $types;
}

/**
 * Returns the form for a new block.
 */
function panels_admin_add_block($id, $parents, $conf = array()) {
  list($conf['module'], $conf['delta']) = explode('-', $id, 2);
  return panels_admin_edit_block($id, $parents, $conf);
}

/**
 * Returns an edit form for a block.
 */
function panels_admin_edit_block($id, $parents, $conf) {
  $form['module'] = array(
    '#type' => 'value',
    '#value' => $conf['module'],
  );
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $conf['delta'],
  );

  if (user_access('administer advanced pane settings')) {
    $form['block_visibility'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use block visibility settings (see block config)'),
      '#default_value' => !empty($conf['block_visibility']),
      '#description' => t('If checked, the block visibility settings for this block will apply to this block.'),
    );
    // Module-specific block configurations.
    if ($settings = module_invoke($conf['module'], 'block', 'configure', $conf['delta'])) {
      // Specifically modify a couple of core block forms.
      if ($conf['module'] == 'block') {
        unset($settings['submit']);
        $settings['info']['#type'] = 'value';
        $settings['info']['#value'] = $settings['info']['#default_value'];
      }
      panels_admin_fix_block_tree($settings);
      $form['block_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Block settings'),
        '#description' => t('Settings in this section are global and are for all blocks of this type, anywhere in the system.'),
        '#tree' => FALSE,
      );


      $form['block_settings'] += $settings;
    }
  }

  return $form;
}

function panels_admin_submit_block(&$form_values) {
  if (!empty($form_values['block_settings'])) {
    module_invoke($form_values['module'], 'block', 'save', $form_values['delta'], $form_values['block_settings']);
  }
}

/**
 * Because form api cannot collapse just part of a tree, and the block settings
 * assume no tree, we have to collapse the tree ourselves.
 */
function panels_admin_fix_block_tree(&$form, $key = NULL) {
  if ($key) {
    if (!empty($form['#parents'])) {
      $form['#parents'] = array_merge(array('configuration', 'block_settings'), $form['#parents']);
    }
    else if (empty($form['#tree'])) {
      $form['#parents'] = array('configuration', 'block_settings', $key);
    }
  }

  if (isset($form['#type']) && $form['#type'] == 'textarea' && !empty($form['#rows']) && $form['#rows'] > 10) {
    $form['#rows'] = 10;
  }

  foreach (element_children($form) as $key) {
    panels_admin_fix_block_tree($form[$key], $key);
  }
}

/**
 * Returns the administrative title for a type.
 */
function panels_admin_title_block($subtype, $conf) {
  $block = module_invoke($conf['module'], 'block', 'list');
  if (empty($block) || empty($block[$conf['delta']])) {
    return t('Deleted/missing block @module-@delta', array('@module' => $conf['module'], '@delta' => $conf['delta']));
  }

  $title = filter_xss_admin($block[$conf['delta']]['info']);
  return $title;
}

function panels_default_block_info($module, $delta, &$info) {
  $core_modules = array('aggregator', 'block', 'blog', 'blogapi', 'book', 'color', 'comment', 'contact', 'drupal', 'filter', 'forum', 'help', 'legacy', 'locale', 'menu', 'node', 'path', 'ping', 'poll', 'profile', 'search', 'statistics', 'taxonomy', 'throttle', 'tracker', 'upload', 'user', 'watchdog', 'system');

  if (in_array($module, $core_modules)) {
    $info['icon'] = 'icon_core_block.png';
    $info['category'] = array(t('Core blocks'), -5);
  }
  else {
    $info['icon'] = 'icon_contrib_block.png';
    $info['category'] = t('Contributed blocks');
  }
}

// These are all on behalf of modules that don't implement panels but we that
// we care about.
function menu_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_block_menu.png';
  $info['category'] = array(t('Menus'), -2);
}

function forum_panels_block_info($module, $delta, &$info) {
  $info['category'] = t('Core blocks');
  switch ($delta) {
    case '0':
      $info['icon'] = 'icon_core_activeforumtopics.png';
      break;

    case '1':
      $info['icon'] = 'icon_core_newforumtopics.png';
      break;

    default:
      // safety net
      panels_default_block_info($module, $delta, $info);
  }
}

function profile_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_authorinformation.png';
  $info['category'] = t('Core blocks');
}

function book_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_booknavigation.png';
  $info['category'] = t('Core blocks');
}

function blog_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_recentblogposts.png';
  $info['category'] = t('Core blocks');
}

function poll_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_recentpoll.png';
  $info['category'] = t('Core blocks');
}

function comment_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_recentcomments.png';
  $info['category'] = t('Core blocks');
}

function search_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_searchform.png';
  $info['category'] = t('Core blocks');
}

function node_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_syndicate.png';
  $info['category'] = t('Core blocks');
}

function aggregator_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_syndicate.png';
  $info['category'] = array(t('Syndicated Feeds'), -3);
}

function block_panels_block_info($module, $delta, &$info) {
  $info['icon'] = 'icon_core_block_empty.png';
  $info['category'] = array(t('Custom'), -10);
}

function user_panels_block_info($module, $delta, &$info) {
  $info['category'] = t('Core blocks');
  switch ($delta) {
    case '0':
      $info['icon'] = 'icon_core_userlogin.png';
      break;

    case '1':
      $info['icon'] = 'icon_core_navigation.png';
      $info['category'] = array(t('Menus'), -2);
      break;

    case '2':
      $info['icon'] = 'icon_core_whosnew.png';
      break;

    case '2':
      $info['icon'] = 'icon_core_whosonline.png';
      break;

    default:
      // safety net
      panels_default_block_info($module, $delta, $info);
  }
}

