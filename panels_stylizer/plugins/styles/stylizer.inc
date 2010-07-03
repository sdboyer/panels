<?php
// $Id$

/**
 * @file
 * Definition of the 'stylizer' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('Stylizer'),
  'description' => t('Allows choice of a stylizer style'),

  'render pane' => 'panels_stylizer_stylizer_style_render_pane',
  'pane settings form' => 'panels_stylizer_stylizer_style_settings_form',

  'render region' => 'panels_stylizer_stylizer_style_render_region',
  'settings form' => 'panels_stylizer_stylizer_style_settings_form',

  // Set up an AJAX callback for the style
  'ajax' => array(
    'custom' => 'panels_stylizer_pane_add_style',
  ),
//  'settings validate' => 'panels_stylizer_stylizer_style_settings_validate',
);

/**
 * Region render theme.
 */
function theme_panels_stylizer_stylizer_style_render_region($display, $panel_id, $panes, $style_settings) {
  $output = '';

  foreach ($panes as $pane_id => $pane_output) {
    $output .= $pane_output;
  }

  if ($style_settings['style'] == '$') {
    $settings = $style_settings['settings'];
  }
  else {
    $style = panels_stylizer_load($style_settings['style']);
    if ($style) {
      $settings = $style->settings;
    }
  }

  if (!empty($settings)) {
    ctools_include('stylizer');
    $plugin = ctools_get_style_base($settings['style_base']);
    ctools_stylizer_add_css($plugin, $settings);

    return theme($plugin['theme'], $settings, ctools_stylizer_get_css_class($plugin, $settings), $output);
  }
  else {
    // if the style is gone, just display the output.
    return $output;
  }
}

/**
 * Pane render theme
 */
function theme_panels_stylizer_stylizer_style_render_pane($content, $pane, $display) {
  $style_settings = $pane->style['settings'];

  if ($style_settings['style'] == '$') {
    $settings = $style_settings['settings'];
  }
  else {
    $style = panels_stylizer_load($style_settings['style']);
    if ($style) {
      $settings = $style->settings;
    }
  }

  if ($settings) {
    ctools_include('stylizer');
    $plugin = ctools_get_style_base($settings['style_base']);

    if (empty($content->css_class)) {
      $content->css_class = '';
    }
    $content->css_class .= ctools_stylizer_get_css_class($plugin, $settings);
    ctools_stylizer_add_css($plugin, $settings);

    if (isset($plugin['theme'])) {
      return theme($plugin['theme'], $settings, $content, $pane, $display);
    }
  }

  // if the style is gone or has no theme of its own, just display the output.
  return theme('panels_pane', $content, $pane, $display);
}

/**
 * Settings form callback.
 */
function panels_stylizer_stylizer_style_settings_form($style_settings, $display, $pid, $type, $renderer) {
  if (!empty($style_settings['style']) && $style_settings['style'] == '$') {
    panels_stylizer_pane_add_style($renderer, array(), $style_settings, $type, $pid, 'settings');
  }

  ctools_include('stylizer');
  $plugins = ctools_get_style_bases();
  $styles = panels_stylizer_load_all();

  $options = array('' => t('- None selected -'));

  foreach ($styles as $name => $style) {
    if (empty($plugins[$style->settings['style_base']])) {
      continue;
    }

    $plugin = $plugins[$style->settings['style_base']];
    if ($plugin['type'] == $type) {
      $options[$name] = $style->admin_title;
    }
  }

  $form['style'] = array(
    '#type' => 'select',
    '#title' => t('Select a preconfigured style'),
    '#options' => $options,
    '#default_value' => (isset($style_settings['style'])) ? $style_settings['style'] : '',
  );

  // I really hate to do this, but the style system does not actually send all
  // of the data necessary to do this through. It's one of the older plugins and
  // its form treatment goes all the way back to 4.7 and hasn't changed much.

  $path = $renderer->get_url('style', 'custom', $type, $pid);
//  $path = 'admin/build/panels/style/pane-style/' . arg(3) . '/' . arg(4) . '/' . arg(5);
  $form['new'] = array(
    '#value' => '<div>' . l(t('Or create a custom style...'), $path, array('attributes' => array('class' => 'ctools-use-ajax'))) . '</div>',
  );

  return $form;
}


/**
 * Allow on-the-fly creation of styles in panes.
 */
function panels_stylizer_pane_add_style(&$renderer, $plugin, &$conf, $type, $pid, $step = NULL) {
  if (!user_access('administer panels styles')) {
    return;
  }

  ctools_include('stylizer');
  $js = FALSE;

  $path = $renderer->get_url('style', 'custom', $type, $pid, '%step');

  $info = array(
    'module' => 'panels',
    'type' => $type,
    'path' => $path,
    'modal' => t('Create custom style'),
    'owner form' => 'panels_stylizer_edit_pane_style_form',
    'owner form validate' => 'panels_stylizer_edit_pane_style_form_validate',
    'owner form submit' => 'panels_stylizer_edit_pane_style_form_submit',
    'owner settings' => array('preconfigured' => FALSE, 'name' => '', 'admin_title' => '', 'admin_description' => ''),
    'cache' => &$renderer->cache,
    'conf' => &$conf,
    'pid' => $pid,
  );

  if (!empty($conf['settings'])) {
    $info['settings'] = $conf['settings'];
  }

  $output = ctools_stylizer_edit_style($info, TRUE, $step);
  if (!empty($info['complete'])) {
    if (isset($cache->style)) {
      unset($cache->style);
    }

    if (!empty($info['owner settings']['preconfigured'])) {
      $style = panels_stylizer_new();
      $style->name = $info['settings']['name'];
      $style->admin_title = $info['owner settings']['admin_title'];
      $style->admin_description = $info['owner settings']['admin_description'];
      $style->settings = $info['settings'];
      panels_stylizer_save($style);
      $conf['style'] = $info['settings']['name'];
      if (isset($conf['settings'])) {
        unset($conf['settings']);
      }
    }
    else {
      $conf['style'] = '$';
      $conf['settings'] = $info['settings'];
    }

    // Be sure to unset the temporary if the style was just changed.
    if (isset($renderer->cache->style)) {
      unset($renderer->cache->style);
    }
    // $conf was a reference so it should just modify.
    panels_edit_cache_set($renderer->cache);

    $renderer->commands[] = ctools_modal_command_dismiss();

    if ($type == 'pane') {
      $renderer->command_update_pane($pid);
    }
    else if ($type == 'region') {
      $renderer->command_update_region_links($pid);
    }
    else {
      $renderer->command_update_display_links();
    }
  }
  else {
    $renderer->commands = $output;
  }
}
