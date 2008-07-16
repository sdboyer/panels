<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_user_profile_panels_content_types() {
  $items['user_profile'] = array(
    'title' => t('User profile'),
    'content_types' => 'panels_admin_content_types_user_profile',
    'single' => TRUE,
    'render callback' => 'panels_content_user_profile',
    'title callback' => 'panels_admin_title_user_profile',
  );
  return $items;
}

/**
 * Output function for the 'node' content type. Outputs a node
 * based on the module and delta supplied in the configuration.
 */
function panels_content_user_profile($conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'term-list';

  if ($account === FALSE || ($account->access == 0 && !user_access('administer users'))) {
    return drupal_not_found();
  }
  // Retrieve and merge all profile fields:
  $fields = array();
  foreach (module_list() as $module) {
    if ($data = module_invoke($module, 'user', 'view', '', $account)) {
      foreach ($data as $category => $items) {
        foreach ($items as $key => $item) {
          $item['class'] = "$module-". $item['class'];
          $fields[$category][$key] = $item;
        }
      }
    }
  }

  // Let modules change the returned fields - useful for personal privacy
  // controls. Since modules communicate changes by reference, we cannot use
  // module_invoke_all().
  foreach (module_implements('profile_alter') as $module) {
    $function = $module .'_profile_alter';
    $function($account, $fields);
  }

  $block->title = check_plain($account->name);
  $block->content = theme('user_profile', $account, $fields);

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_user_profile() {
  return array(
    'description' => array(
      'title' => t('User profile'),
      'icon' => 'icon_user.png',
      'path' => panels_get_path('content_types/user'),
      'description' => t('The profile of a user.'),
      'required context' => new panels_required_context(t('User'), 'user'),
      'category' => array(t('User context'), -9),
    ),
  );
}

/**
 * Display the administrative title for a panel pane in the drag & drop UI
 */
function panels_admin_title_user_profile($conf, $context) {
  return t('"@s" user profile', array('@s' => $context->identifier));
}

