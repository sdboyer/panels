<?php
// $Id$


/**
 * @file arguments/uid.inc
 *
 * Plugin to provide an argument handler for a user id
 */
function panels_uid_panels_arguments() {
  $args['uid'] = array(
    'title' => t("User ID"),
    // keyword to use for %substitution
    'keyword' => 'user',
    'description' => t('Creates a user object from the argument.'),
    'context' => 'panels_uid_context',
    'settings form' => 'panels_uid_settings_form',
    'settings form submit' => 'panels_uid_settings_form_submit',
    'displays' => 'panels_uid_displays',
    'choose display' => 'panels_uid_choose_display',
  );
  return $args;
}

/**
 * Discover if this argument gives us the user we crave.
 */
function panels_uid_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return panels_context_create_empty('user');
  }

  if (!is_numeric($arg)) {
    return FALSE;
  }

  $user = user_load(array('uid' => $arg));
  if (!$user) {
    return FALSE;
  }

  return panels_context_create('user', $user);
}

/**
 * Settings form for the argument
 */
function panels_uid_settings_form($conf) {
  // Doing different displays based upon role is hard because roles are not
  // 1:1 like type/vocabulary are for node and term.
}

/**
 * There appears to be a bit of a bug with the way we're handling forms; it causes
 * 'checkboxes' to get invalid values added to them when empty. This takes care
 * of that.
 */
function panels_uid_settings_form_submit(&$values) {
}

/**
 * What additional displays does this argument provide?
 */
function panels_uid_displays($conf, $id) {
  return array();
}

/**
 * Based upon the settings and the context, choose which display to use.
 */
function panels_uid_choose_display($conf, $context) {
}
