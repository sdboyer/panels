<?php
// $Id

/**
 * @file
 * Contains Views plugin definitions for the panel pane display.
 */

/**
 * Implementation of hook_views_plugins
 */
function panels_views_views_plugins() {
  return array(
    'display' => array(
      'panel_pane' => array(
        'title' => t('Panel pane'),
        'help' => t('Is available as pane content for a panel display.'),
        'handler' => 'panels_plugin_display_panel_pane',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'use pager' => TRUE,
        'use more' => TRUE,
        'accept attachments' => TRUE,
        'help topic' => 'display-pane',
      ),
    ),
  );
}
