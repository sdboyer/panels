<?php
// $Id$


/**
 * Plugin to provide an relationship handler for node from user
 */
function panels_user_from_node_panels_relationships() {
  $args['user_from_node'] = array(
    'title' => t("User from node"),
    'keyword' => 'user',
    'description' => t('Adds a user from node author context'),
    'required context' => new panels_required_context(t('Node'), 'node'),
    'context' => 'panels_user_from_node_context',
  );
  return $args;
}

/**
 * Return a new context based on an existing context
 */
function panels_user_from_node_context($context = NULL, $conf) {
  // If unset it wants a generic, unfilled context, which is just NULL
  if (empty($context->data)) {
    return panels_context_create_empty('user', NULL);
  }
  if (isset($context->data->uid)) {
    // Load the user that is the author of the node
    $uid = $context->data->uid;
    $account = user_load(array('uid' => $uid));

    // Send it to panels
    return panels_context_create('user', $account);
  }
  else {
    return panels_context_create_empty('user', NULL);
  }
}

