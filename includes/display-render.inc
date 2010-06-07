<?php
// $Id$

/**
 * @file
 *
 * Contains Panels display rendering functions.
 */

/**
 * Render the administrative layout of a display.
 *
 * This is used for the edit version, so that layouts can have different
 * modes, such as the flexible layout designer mode.
 */
function panels_render_layout_admin($layout, $content, $display) {
  // @todo This should be abstracted.
  if (!empty($layout['css'])) {
    if (file_exists(path_to_theme() . '/' . $layout['css'])) {
      drupal_add_css(path_to_theme() . '/' . $layout['css']);
    }
    else {
      drupal_add_css($layout['path'] . '/' . $layout['css']);
    }
  }

  if (isset($layout['admin css'])) {
    drupal_add_css($layout['path'] . '/' . $layout['admin css']);
  }

  $theme = isset($layout['admin theme']) ? $layout['admin theme'] : $layout['theme'];
  return theme($theme, isset($display->css_id) ? $display->css_id : '', $content, $display->layout_settings, $display);
}

/**
 * Render a single pane, identifying its context, and put it into
 * the $panes array.
 */
function panels_render_pane_content(&$display, &$pane, $keywords) {
  $content = panels_get_pane_content($display, $pane, $keywords, $display->args, $display->context, $display->incoming_content);

  // Pass long the css_id that is usually available.
  if (!empty($pane->css['css_id'])) {
    $content->css_id = $pane->css['css_id'];
  }

  // Pass long the css_class that is usually available.
  if (!empty($pane->css['css_class'])) {
    $content->css_class = $pane->css['css_class'];
  }

  return $content;
}

/**
 * Render a pane using the appropriate style.
 *
 * $content
 *   The already rendered content via panels_render_pane_content()
 * $pane
 *   The $pane information from the display
 * $display
 *   The display.
 *
 * TODO refactor the render pipeline so that style plugins need not call this
 * in panel region styling
 */
function panels_render_pane($content, $pane, &$display) {
  if ($display->hide_title == PANELS_TITLE_PANE && !empty($display->title_pane) && $display->title_pane == $pane->pid) {

    // If the user selected to override the title with nothing, and selected
    // this as the title pane, assume the user actually wanted the original
    // title to bubble up to the top but not actually be used on the pane.
    if (empty($content->title) && !empty($content->original_title)) {
      $display->stored_pane_title = $content->original_title;
    }
    else {
      $display->stored_pane_title = !empty($content->title) ? $content->title : '';
    }
  }

  if (!empty($pane->style['style'])) {
    $style = panels_get_style($pane->style['style']);

    if (isset($style) && isset($style['render pane'])) {
      $output = theme($style['render pane'], $content, $pane, $display);

      // This could be null if no theme function existed.
      if (isset($output)) {
        return $output;
      }
    }
  }

  if (!empty($content->content)) {
    // fallback
    return theme('panels_pane', $content, $pane, $display);
  }
}

/**
 * Given a display and the id of a panel, get the style in which to render
 * that panel.
 */
function panels_get_panel_style_and_settings($panel_settings, $panel) {
  if (empty($panel_settings)) {
    return array(panels_get_style('default'), array());
  }

  if (empty($panel_settings[$panel]['style']) || $panel_settings[$panel]['style'] == -1) {
    if (empty($panel_settings['style'])) {
      return array(panels_get_style('default'), array());
    }

    $style = panels_get_style($panel_settings['style']);
    $style_settings = isset($panel_settings['style_settings']['default']) ? $panel_settings['style_settings']['default'] : array();
  }
  else {
    $style = panels_get_style($panel_settings[$panel]['style']);
    $style_settings = isset($panel_settings['style_settings'][$panel]) ? $panel_settings['style_settings'][$panel] : array();
  }

  return array($style, $style_settings);
}

/**
 * Render a panel, by storing the content of each pane in an appropriate array
 * and then passing through to the theme function that will render the panel
 * in the configured panel style.
 *
 * @param $display
 *   A display object.
 * @param $panel
 *   The ID of the panel being rendered
 * @param $panes
 *   An array of panes that are assigned to the panel that's being rendered.
 *
 * @return
 *   The rendered HTML for a panel.
 * @render
 */
function panels_render_panel($display, $panel, $panes) {
  list($style, $style_settings) = panels_get_panel_style_and_settings($display->panel_settings, $panel);

  // Retrieve the pid (can be a panel page id, a mini panel id, etc.), this
  // might be used (or even necessary) for some panel display styles.
  // TODO: Got to fix this to use panel page name instead of pid, since pid is
  // no longer guaranteed. This needs an API to be able to set the final id.
  $owner_id = 0;
  if (isset($display->owner) && is_object($display->owner) && isset($display->owner->id)) {
    $owner_id = $display->owner->id;
  }

  return theme($style['render panel'], $display, $owner_id, $panes, $style_settings, $panel);
}

/**
 * Get the title from a display.
 *
 * The display must have already been rendered, or the setting to set the display's title
 * from a pane's title will not have worked.
 *
 * @param $display
 *   The display to get the title from.
 *
 * @return
 *   The title to use. If NULL, this means to let any default title that may be in use
 *   pass through. i.e, do not actually set the title.
 */
function panels_display_get_title($display) {
  switch ($display->hide_title) {
    case PANELS_TITLE_NONE:
      return '';

    case PANELS_TITLE_PANE:
      return isset($display->stored_pane_title) ? $display->stored_pane_title : '';

    case PANELS_TITLE_FIXED:
    case FALSE; // For old exported panels that are not in the database.
      if (!empty($display->title)) {
        return filter_xss_admin(ctools_context_keyword_substitute($display->title, array(), $display->context));
      }
      return NULL;
  }
}
