<?php
// $Id$

/**
 * @file 
 * Definition of the 'naked' panel style.
 */

// ---------------------------------------------------------------------------
// Panels hooks.

/**
 * Implementation of hook_panels_style_info().
 */
function panels_naked_panels_styles() {
  return array(
    'title' => t('No markup at all'),
    'description' => t('Display the pane with no markup, not even a title.'),
    'render pane' => 'panels_naked_style_render_pane',
  );
}

// ---------------------------------------------------------------------------
// Panels style plugin callbacks.

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_naked_style_render_pane($content, $pane, $display) {
  return $content->content;
}

