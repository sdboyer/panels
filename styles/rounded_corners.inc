<?php
// $Id$

/**
 * @file styles/rounded_corners.inc
 * Definition of the 'rounded_corners' panel style.
 */

// ---------------------------------------------------------------------------
// Panels hooks.

/**
 * Implementation of hook_panels_style_info().
 */
function panels_rounded_corners_panels_styles() {
  return array(
    'rounded_corners' => array(
      'title' => t('Rounded corners'),
      'description' => t('Presents the panes or panels with a rounded corner box around them'),
      'render panel' => 'panels_rounded_corners_style_render_panel',
      'render pane' => 'panels_rounded_corners_style_render_pane',
      'settings form' => 'panels_rounded_corners_style_settings_form',
    ),
  );
}

// ---------------------------------------------------------------------------
// Panels style plugin callbacks.

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_rounded_corners_style_render_panel($display, $panel_id, $panes, $settings) {
  $output = '';

  // Determine where to put the box. If empty or 'pane' around each pane. If
  // 'panel' then just around the whole panel.
  $where = empty($settings['corner_location']) ? 'pane' : $settings['corner_location'];

  $print_separator = FALSE;
  foreach ($panes as $pane_id => $pane) {
    // Add the separator if we've already displayed a pane.
    if ($print_separator) {
      $output .= '<div class="panel-separator"></div>';
    }
    $text = panels_render_pane($pane, $display->content[$pane_id], $display);
    if ($text) {
      $output .= ($where == 'pane') ? panels_rounded_corners_box($text) : $text;
    }

    // If we displayed a pane, this will become true; if not, it will become
    // false.
    $print_separator = (bool) $text;
  }

  if ($where == 'panel') {
    $output = panels_rounded_corners_box($output);
  }

  panels_add_rounded_corners_css($display, $where);

  return $output;
}

function panels_add_rounded_corners_css($display, $where) {
  static $displays_used = array();
  if (empty($displays_used[$display->name])) {
    panels_rounded_corners_css($display, $where);
    $displays_used[$display->name] = TRUE;
  }
}

/**
 * Render callback for a single pane.
 */
function theme_panels_rounded_corners_style_render_pane($content, $pane, $display) {
  // Just stick a box around the standard theme_panels_pane.
  $output = panels_rounded_corners_box(theme('panels_pane', $content, $pane, $display));
  panels_add_rounded_corners_css($display, 'pane');
  return $output;
}

/**
 * Settings form callback.
 */
function panels_rounded_corners_style_settings_form($style_settings) {
  $form['corner_location'] = array(
    '#type' => 'select',
    '#title' => t('Box around'),
    '#options' => array(
      'pane' => t('Each pane'),
      'panel' => t('Each panel'),
    ),
    '#default_value' => (isset($style_settings['corner_location'])) ? $style_settings['corner_location'] : 'ul',
    '#description' => t('Choose whether to include the box around each pane (piece of content) or panel (each column or region)'),
  );

  return $form;
}

/**
 * Generates the dynamic CSS.
 *
 * @param $display
 *   A Panels display object.
 * @param $where
 *   String indicating where the rounded corners should be applied, either
 *   'pane' or 'panel'.
 */
function panels_rounded_corners_css($display, $where = 'pane') {
  $idstr = empty($display->css_id) ? '.rounded_corner' : "#$display->css_id";
  $url = panels_get_path('styles/corners', TRUE);

  $css = <<<EOF

$idstr .t-edge, .b-edge, .l-edge, .r-edge, .wrap-corner {
  position: relative;
  /* hasLayout -1 ? For IE only */
  zoom: 1;
}
$idstr .t-edge {
  background: url($url/shadow-t.png) repeat-x 0 top;
  font-size: 1px;
}
$idstr .b-edge {
  background: url($url/shadow-b.png) repeat-x 0 bottom;
  font-size: 1px;
}
$idstr .l-edge {
  background: url($url/shadow-l.png) repeat-y 0 0;
}
$idstr .r-edge {
  background: url($url/shadow-r.png) repeat-y right 0;
}
$idstr .wrap-corner {
  background: #fff !important;
}
$idstr .wrap-corner .t-edge, .wrap-corner .b-edge {
  height: 11px;
}
$idstr .wrap-corner .l, .wrap-corner .r {
  position: absolute;
  top: 0;
  height: 11px;
  width: 11px;
  background-image: url($url/corner-bits.png);
}
$idstr .wrap-corner .l {
  left: 0;
}
$idstr .wrap-corner .r {
  right: 0;
  background-position: -11px 0;
}
$idstr .wrap-corner .b-edge .l {
  background-position: 0 -11px;
}
$idstr .wrap-corner .b-edge .r {
  background-position: -11px -11px;
}
$idstr .wrap-corner .r-edge {
  padding: 5px 24px;
}
$idstr div.admin-links {
  margin-top: -14px;
  margin-left: -12px;
}

EOF;

  if ($where == 'panel') {
    $css .= <<<EOF
$idstr .panel-separator {
  background: url($url/shadow-b.png) repeat-x 0 center;
  font-size: 1px;
  height: 30px;
  }
EOF;
  }

  drupal_set_html_head("<style type=\"text/css\" media=\"all\">\n$css</style>\n");
}

/**
 * Create the HTML for our rounded corner box.
 *
 * @param $text
 *   The content of this rounded corner box.
 *
 * @return
 *   The created HTML.
 */
function panels_rounded_corners_box($content) {
  return <<<EOF
<div class="rounded_corner">
  <div class="wrap-corner">
    <div class="t-edge"><div class="l"></div><div class="r"></div></div>
    <div class="l-edge">
      <div class="r-edge">
        $content
      </div>
    </div>
    <div class="b-edge"><div class="l"></div><div class="r"></div></div>
  </div>
</div>
EOF;
}
