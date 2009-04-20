<?php
// $Id$
/**
 * @file callbacks.inc
 * Minor menu callbacks for Panels helpers.
 */

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
  $output = '<p>' . t('Panels is the core engine for a number of submodules, including panels-pages, panels-nodes, and mini-panels.  Panels allows the website adminstrator (or sometimes the end-user) to manipulate the layout of individual pages, sidebars, and content pieces, as well as easily dictate what content is displayed in the layout. ') . '</p>';
  $output .= '<p>' . t('Most Drupal users are familiar with the block to region layout mechanism in which you can assign a block to any region defined in your theme.  Panels takes this concept a massive step forward.  Through the panels interface you can start by creating a layout with any number of columns, headers, and footer, and control the width of those areas. ') . '</p>';
  $output .= '<p>' . t('After creating your layout, you can assign pieces of content to those areas in an easy drag and drop interface.  Content is not limited to blocks, but can be nodes, views, blocks, or other types of content that extend themselves to panels. ') . '</p>';
  $output .= '<p><b>' . t('Panels-nodes') . '</b>' . t(', is useful for creating layouts that only occupy the content area of your pages.  Frequently, it is desirable to add an area to a node layout, such as a pull quote for a newspaper or a photo block, that you don\'t necessarily want on every node.  Panels Nodes lets you control the layout of a single node at a time and place content such as blog posts, images, blogs in and around the post. ') . '</p>';
  $output .= '<p><b>' . t('Mini-panels') . '</b>' . t(', is a layout mechanism for blocks.  It won\'t take long using panels before you get to a point when you want a panel inside of a panel.  Or a panel that can be used as a block.  That is exactly what mini-panels does. You can create a small panel here with various pieces of content and then put it inside of a panels-page or panels-node. ') . '</p>';
  $output .= '<p><b>' . t('Panels-pages') . '</b>'  . t(' are now handled by the delegator.module which can handle both panels as pages as well as other rendering engines. Please activate the delegator module which is part of the CTools package, and then visit <a href="!link">administer >> build >> pages</a>', array('!link' => url('admin/build/pages'))) . '</p>';

  module_load_include('inc', 'system', 'system.admin');
  $output .= system_admin_menu_block_page();

  return $output;
}

function panels_admin_panel_page() {
  $output = '<p><b>' . t('Panels-pages') . '</b>'  . t(' are now handled by the delegator.module which can handle both panels as pages as well as other rendering engines. Please activate the delegator module which is part of the CTools package, and then visit <a href="!link">administer >> build >> pages</a>', array('!link' => url('admin/build/pages'))) . '</p>';

  return $output;
}