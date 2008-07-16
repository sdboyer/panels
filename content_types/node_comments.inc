<?php
// $Id$


/**
 * Plugin declaration function - returns a plugin definition array that
 * describes the content type.
 */
function panels_node_comments_panels_content_types() {
  if (module_exists('comment')) {
    $items['node_comments'] = array(
      'title' => t('Node comments'),
      'content_types' => 'panels_admin_content_types_node_comments',
      'single' => TRUE,
      'render callback' => 'panels_content_node_comments',
      'add callback' => 'panels_admin_edit_node_comments',
      'edit callback' => 'panels_admin_edit_node_comments',
      'title callback' => 'panels_admin_title_node_comments',
    );
  }
  return $items;
}

/**
 * Output function for the 'node comments' content type. Outputs the comments
 * that have been made on a given nid, as provided by the $context.
 */
function panels_content_node_comments($conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'comments';
  $block->delta  = $node->nid;

  $block->subject = t('Comments');
  if (empty($node)) {
    $block->content = t('Node comments go here.');
  }
  else {
    $block->content = panels_comment_render($node, $conf);
    // Update the history table, stating that this user viewed this node.
    node_tag_new($node->nid);
  }

  return $block;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_comments() {
  return array(
    'comments' => array(
      'title' => t('Node comments'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('The comments of the referenced node.'),
      'required context' => new panels_required_context(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

function panels_admin_edit_node_comments($id, $parents, $conf = array()) {
  if (empty($conf)) {
    $conf = array(
      'mode' => _comment_get_display_setting('mode'),
      'order' => _comment_get_display_setting('sort'),
      'comments_per_page' => _comment_get_display_setting('comments_per_page'),
    );
  }

  $form['mode'] = array(
    '#type' => 'select',
    '#title' => t('Mode'),
    '#default_value' => $conf['mode'],
    '#options' => _comment_get_modes(),
    '#weight' => 1,
  );
  $form['order'] = array(
    '#type' => 'select',
    '#title' => t('Sort'),
    '#default_value' => $conf['order'],
    '#options' => _comment_get_orders(),
    '#weight' => 2,
  );
  foreach (_comment_per_page() as $i) {
    $options[$i] = t('!a comments per page', array('!a' => $i));
  }
  $form['comments_per_page'] = array('#type' => 'select',
    '#title' => t('Pager'),
    '#default_value' => $conf['comments_per_page'],
    '#options' => $options,
    '#weight' => 3,
  );
  return $form;
}

function panels_admin_title_node_comments($conf, $context) {
  return t('"@s" comments', array('@s' => $context->identifier));
}

/**
 * This function is a somewhat stripped down version of comment_render
 * that removes a bunch of cruft that we both don't need, and makes it
 * difficult to modify this.
 */
function panels_comment_render($node, $conf) {
  if (!user_access('access comments')) {
    return;
  }

  $mode = $conf['mode'];
  $order = $conf['order'];
  $comments_per_page = $conf['comments_per_page'];

  // Multiple comment view
  $query_count = 'SELECT COUNT(*) FROM {comments} WHERE nid = %d';
  $query = 'SELECT c.cid as cid, c.pid, c.nid, c.subject, c.comment, c.format, c.timestamp, c.name, c.mail, c.homepage, u.uid, u.name AS registered_name, u.picture, u.data, c.score, c.users, c.thread, c.status FROM {comments} c INNER JOIN {users} u ON c.uid = u.uid WHERE c.nid = %d';

  $query_args = array($node->nid);
  if (!user_access('administer comments')) {
    $query       .= ' AND c.status = %d';
    $query_count .= ' AND status = %d';
    $query_args[] = COMMENT_PUBLISHED;
  }

  if ($order == COMMENT_ORDER_NEWEST_FIRST) {
    if ($mode == COMMENT_MODE_FLAT_COLLAPSED || $mode == COMMENT_MODE_FLAT_EXPANDED) {
      $query .= ' ORDER BY c.timestamp DESC';
    }
    else {
      $query .= ' ORDER BY c.thread DESC';
    }
  }
  else if ($order == COMMENT_ORDER_OLDEST_FIRST) {
    if ($mode == COMMENT_MODE_FLAT_COLLAPSED || $mode == COMMENT_MODE_FLAT_EXPANDED) {
      $query .= ' ORDER BY c.timestamp';
    }
    else {
      $query .= ' ORDER BY SUBSTRING(c.thread, 1, (LENGTH(c.thread) - 1))';
    }
  }

  // Start a form, for use with comment control.
  $result = pager_query($query, $comments_per_page, 0, $query_count, $query_args);

  $divs = 0;
  $last_depth = 0;
  drupal_add_css(drupal_get_path('module', 'comment') .'/comment.css');
  while ($comment = db_fetch_object($result)) {
    $comment = drupal_unpack($comment);
    $comment->name  = $comment->uid ? $comment->registered_name : $comment->name;
    $comment->depth = count(explode('.', $comment->thread)) - 1;

    if ($mode == COMMENT_MODE_THREADED_COLLAPSED || $mode == COMMENT_MODE_THREADED_EXPANDED) {
      if ($comment->depth > $last_depth) {
        $divs++;
        $output .= '<div class="indented">';
        $last_depth++;
      }
      else {
        while ($comment->depth < $last_depth) {
          $divs--;
          $output .= '</div>';
          $last_depth--;
        }
      }
    }

    if ($mode == COMMENT_MODE_FLAT_COLLAPSED) {
      $output .= theme('comment_flat_collapsed', $comment);
    }
    else if ($mode == COMMENT_MODE_FLAT_EXPANDED) {
      $output .= theme('comment_flat_expanded', $comment);
    }
    else if ($mode == COMMENT_MODE_THREADED_COLLAPSED) {
      $output .= theme('comment_thread_collapsed', $comment);
    }
    else if ($mode == COMMENT_MODE_THREADED_EXPANDED) {
      $output .= theme('comment_thread_expanded', $comment);
    }
  }
  for ($i = 0; $i < $divs; $i++) {
    $output .= '</div>';
  }
  $output .= theme('pager', NULL, $comments_per_page, 0);

  return $output;
}

