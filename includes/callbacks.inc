<?php
// $Id$
/**
 * @file callbacks.inc
 * Minor menu callbacks for Panels helpers.
 */

/**
 * Helper function for autocompletion of node titles.
 */
function panels_node_autocomplete($string) {
  if ($string != '') { // if there are node_types passed, we'll use those in a MySQL IN query.
    $preg_matches = array();
    $match = preg_match('/\[nid: (\d+)\]/', $string, $preg_matches);
    if (!$match) {
      $match = preg_match('/^nid: (\d+)/', $string, $preg_matches);
    }
    if ($match) {
      $arg = $preg_matches[1];
      $where = "n.nid = %d";
    }
    else {
      $arg = $string;
      $where = "LOWER(title) LIKE LOWER('%%%s%%')";
    }
    $result = db_query_range(db_rewrite_sql("SELECT n.nid, n.title, u.name FROM {node} n INNER JOIN {users} u ON u.uid = n.uid WHERE $where"), $arg, 0, 10);

    $matches = array();
    while ($node = db_fetch_object($result)) {
      $name = empty($node->name) ? variable_get('anonymous', t('Anonymous')) : check_plain($node->name);
      $matches[$node->title . " [nid: $node->nid]"] = '<span class="autocomplete_title">'. check_plain($node->title) .'</span> <span class="autocomplete_user">('. t('by @user', array('@user' => $name)) .')</span>';
    }
    drupal_json($matches);
  }
}

/**
 * A central administrative page for Panels.
 */
function panels_admin_page() {
  //
  // TODO: Provide a cute little panel-base administrative panel
  // that provides things like "Recently edited", "recently added"
  // and other administrative items to make dealing with the complexity
  // of panels a little easier.

  // TODO -- move this to a help file.
  $output = '<p>'. t('Panels is the core engine for a number of submodules, including panels-pages, panels-nodes, and mini-panels.  Panels allows the website adminstrator (or sometimes the end-user) to manipulate the layout of individual pages, sidebars, and content pieces, as well as easily dictate what content is displayed in the layout.') .'</p>';
  $output .= '<p>'. t('Most Drupal users are familiar with the block to region layout mechanism in which you can assign a block to any region defined in your theme.  Panels takes this concept a massive step forward.  Through the panels interface you can start by creating a layout with any number of columns, headers, and footer, and control the width of those areas.') .'</p>';
  $output .= '<p>'. t('After creating your layout, you can assign pieces of content to those areas in an easy drag and drop interface.  Content is not limited to blocks, but can be nodes, views, blocks, or other types of content that extend themselves to panels.') .'</p>';
  $output .= '<p><b>'. t('Panels-pages') .'</b>' .t(', is the the primary panels module, you can use this for creating single full page layouts.  This replaces the standard panel that existed in the earlier versions of panels. If you are upgrading your site from Panels 1, and you cannot find where your panels went, be sure to enable the panel pages module!') .'</p>';
  $output .= '<p><b>'. t('Panels-nodes') .'</b>' .t(', is useful for creating layouts that only occupy the content area of your pages.  Frequently, it is desirable to add an area to a node layout, such as a pull quote for a newspaper or a photo block, that you don\'t necessarily want on every node.  Panels Nodes lets you control the layout of a single node at a time and place content such as blog posts, images, blogs in and around the post.') .'</p>';
  $output .= '<p><b>'. t('Mini-panels') .'</b>' .t(', is a layout mechanism for blocks.  It won\'t take long using panels before you get to a point when you want a panel inside of a panel.  Or a panel that can be used as a block.  That is exactly what mini-panels does. You can create a small panel here with various pieces of content and then put it inside of a panels-page or panels-node.') .'</p>';

  require_once './' . drupal_get_path('module', 'system') . '/system.admin.inc';
  $output .= system_admin_menu_block_page();

  return $output;
}