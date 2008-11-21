<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_comment_form_panels_content_types() {
  if (module_exists('comment')) {
    $items['node_comment_form'] = array(
      'title' => t('Node comment form'),
      'single' => TRUE,
      'content_types' => 'panels_admin_content_types_node_comment_form',
      'render callback' => 'panels_content_node_comment_form',
      'add callback' => 'panels_admin_edit_node_comment_form',
      'edit callback' => 'panels_admin_edit_node_comment_form',
      'title callback' => 'panels_admin_title_node_comment_form',
    );
    return $items;
  }
}

function panels_content_node_comment_form($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'comments';
  $block->delta  = $node->nid;

  $block->subject = t('Add comment');

  if (empty($node)) {
    $block->content = t('Comment form here.');
  }
  else {
    if (user_access('post comments') && node_comment_mode($node->nid) == COMMENT_NODE_READ_WRITE) {
      $block->content = drupal_get_form('panels_comment_form', array('nid' => $node->nid));
    }
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_comment_form() {
  return array(
    'comment_form' => array(
      'title' => t('Comment form'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('A form to add a new comment.'),
      'required context' => new panels_required_context(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

function panels_admin_title_node_comment_form($subtype, $conf, $context) {
  return t('"@s" comment form', array('@s' => $context->identifier));
}

function panels_comment_form(&$form_state, $edit) {
  $form = comment_form($form_state, $edit);
  // force the form to come back to here.
  $url = parse_url($_GET['q']);
  $path = $url['path'];
  $form['#action'] = url($path, array('fragment' => 'new'));
  $form['#redirect'] = array($path, NULL, 'new');
  $form['#validate'][] = 'comment_form_validate';
  $form['#submit'][] = 'comment_form_submit';
  return $form;
}

