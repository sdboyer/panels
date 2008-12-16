<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_user_picture_panels_content_types() {
  $items['user_picture'] = array(
    'title' => t('User picture'),
    'content_types' => 'panels_admin_content_types_user_picture',
    'single' => TRUE,
    'render callback' => 'panels_content_user_picture',
    'title callback' => 'panels_admin_title_user_picture',
  );
  return $items;
}

function panels_content_user_picture($subtype, $conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'term-list';

  if ($account === FALSE || ($account->access == 0 && !user_access('administer users'))) {
    return drupal_not_found();
  }

  $block->title = check_plain($account->name);
  $block->content = theme('user_picture', $account);

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_user_picture() {
  return array(
    'description' => array(
      'title' => t('User picture'),
      'icon' => 'icon_user.png',
      'path' => panels_get_path('content_types/user'),
      'description' => t('The picture of a user.'),
      'required context' => new ctools_context_required(t('User'), 'user'),
      'category' => array(t('User context'), -9),
    ),
  );
}

/**
 * Display the administrative title for a panel pane in the drag & drop UI
 */
function panels_admin_title_user_picture($subtype, $conf, $context) {
  return t('"@s" user picture', array('@s' => $context->identifier));
}

