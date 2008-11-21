<?php
// $Id$

/**
 * @file
 *
 * Form and ajax handling for edit layout settings
 */
/**
 * Handle calling and processing of the form for editing display layout settings.
 *
 * Helper function for panels_edit_layout_settings().
 *
 * @see panels_edit_layout_settings() for details on the various behaviors of this function.
 */
function _panels_edit_layout_settings($display, $finish, $destination, $title) {
  $form_state = array(
    'display' => &$display,
    'finish' => $finish,
    'destination' => $destination,
    'display_title' => $title,
    're_render' => FALSE,
    'no_redirect' => TRUE,
  );

  $output = drupal_build_form('panels_edit_layout_settings_form', $form_state);
  // no output == submit
  if (!$output) {
    panels_save_display($display);
    panels_cache_clear('style_settings', $display->did);
    drupal_set_message(t("Your layout settings have been saved."));

    if (!empty($form_state['clicked_button']['#return_here'])) {
      drupal_goto($_GET['q']);
    }
    if ($destination) {
      return drupal_goto($destination);
    }
    return $form_state['display'];
  }

  return $output;
}

/**
 * Form definition for the display layout settings editor.
 *
 * @ingroup forms
 * @see panels_edit_layout_settings_form_validate()
 * @see panels_edit_layout_settings_form_submit()
 */
function panels_edit_layout_settings_form(&$form_state) {
  $form = array();
  $display = &$form_state['display'];

  // Style settings are edited via a modal and cached, this will retrieve
  // them from cache if posting or insert them into cache if the form is
  // being filled out fresh.
  if (!empty($_POST) && is_array($cache = panels_cache_get('style_settings', $display->did))) {
    $display->panel_settings['style_settings'] = $cache;
  }
  else {
    if (!isset($display->panel_settings['style_settings'])) {
      $display->panel_settings['style_settings'] = array();
    }
    panels_cache_set('style_settings', $display->did, $display->panel_settings['style_settings']);
  }


  $layout = panels_get_layout($display->layout);

  // TODO doc the ability to do this as part of the API
  if (!empty($layout['settings form']) && function_exists($layout['settings form'])) {
    $form['layout_settings'] = $layout['settings form']($display, $layout, $display->layout_settings);
  }
  $form['layout_settings']['#tree'] = TRUE;

  if ($form_state['display_title']) {
    $form['display_title'] = array (
      '#type' => 'fieldset',
      '#title' => t('Panel title'),
      '#tree' => TRUE,
    );

    $form['display_title']['title'] = array(
      '#type' => 'textfield',
      '#size' => 35,
      '#default_value' => $display->title,
      '#title' => t('Title'),
      '#description' => t('The title of this panel. Your theme will render this text as the main page title users view this display, unless this text is overridden elsewhere.'),
    );

    $form['display_title']['hide_title'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide title'),
      '#default_value' => $display->hide_title,
      '#description' => t('Check this box to hide the main page title for this panel.'),
    );

    if (isset($title) && is_string($title)) {
      $form['display_title']['title']['#description'] .= " " . t("If you leave this field blank, then the default title, '@title', will be used instead.", array('@title' => $title));
    }
  }

  $panel_settings = $display->panel_settings;
  $style = panels_get_style((!empty($panel_settings['style'])) ? $panel_settings['style'] : 'default');

  // Let the user choose between panel styles that are available for any
  // panels implementation or specifically to this one.
  $options = array();
  foreach (panels_get_styles() as $name => $properties) {
    if (empty($properties['hidden']) && (!empty($properties['render panel']))) {
      $options[$name] = $properties['title'];
    }
  }

  $form['panel_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Panel settings'),
    '#tree' => TRUE,
  );

  $form['panel_settings']['start_box'] = array(
    '#value' => '<div class="form-item clear-block"><label>' . t('Default panel style') . ':</label>',
  );

  // component #1 of the URL for hte panel style settings button.
  $form['panel_settings']['edit_style_destination'] = array(
    '#attributes' => array('class' => 'panels-style-settings-url'),
    '#type' => 'hidden',
    '#value' => url('panels/ajax/style-settings/' . $display->did . '/default',  array('absolute' => true)),
  );

  // Component #2 of the URL -- the value will be selected by the user.
  $form['panel_settings']['style'] = array(
    '#prefix' => '<div class="panels-style-settings-box">',
    '#suffix' => '</div>',
    '#type' => 'select',
    '#options' => $options,
    '#attributes' => array('class' => 'panels-style-settings-url'),
    '#default_value' => $style['name'],
  );

  // Is this form being posted? If so, check cache.
  $style_settings = $panel_settings['style_settings'];

  $form['panel_settings']['style_settings'] = array(
    '#type' => 'value',
    '#value' => $style_settings,
  );

  $form['panel_settings']['edit_style'] = array(
    '#type' => 'submit',
    '#attributes' => array('class' => 'panels-ajax-link'),
    '#id' => 'panels-style-settings',
    '#value' => t('Edit style settings'),
  );

  $form['panel_settings']['end_box'] = array(
    '#value' => '</div>',
  );

  $form['panel_settings']['individual'] = array(
    '#type' => 'checkbox',
    '#title' => t('Per panel settings'),
    '#id' => 'panel-settings-individual',
    '#description' => t('If this is checked, each region in the display can have its own style.'),
    '#default_value' => !empty($panel_settings['individual']),
  );

  $layout_options = array_merge(array('-1' => t('Use the default panel style')), $options);
  $layout_panels  = panels_get_panels($layout, $display);

  $checkboxes = array();
  foreach ($layout_panels as $id => $name) {
    $form['panel_settings']['panel'][$id]['start_box'] = array(
      '#value' => '<div class="form-item clear-block"><label>' . $name . ':</label>',
    );

    // component #1 of the URL for hte panel style settings button.
    $form['panel_settings']['panel'][$id]['edit_style_destination'] = array(
      '#attributes' => array('class' => "panels-style-settings-$id-url"),
      '#type' => 'hidden',
      '#value' => url('panels/ajax/style-settings/' . $display->did . '/' . $id,  array('absolute' => true)),
    );

    if (!isset($display->panel_settings['panel'][$id]['style'])) {
      $display->panel_settings['panel'][$id]['style'] = -1;
    }

    // Component #2 of the URL -- the value will be selected by the user.
    $form['panel_settings']['panel'][$id]['style'] = array(
      '#prefix' => '<div class="panels-style-settings-box">',
      '#suffix' => '</div>',
      '#type' => 'select',
      '#options' => $layout_options,
      '#id' => 'panel-settings-style-' . $id,
      '#attributes' => array('class' => "panels-style-settings-$id-url"),
      '#default_value' => $display->panel_settings['panel'][$id]['style'],
    );
    $checkboxes[] = '#panel-settings-style-' . $id;

    $form['panel_settings']['panel'][$id]['edit_style'] = array(
      '#type' => 'submit',
      '#id' => 'panels-style-settings-' . $id,
      '#attributes' => array('class' => 'panels-ajax-link'),
      '#value' => t('Edit style settings'),
    );
    $checkboxes[] = '#panels-style-settings-' . $id;

    $form['panel_settings']['panel'][$id]['end_box'] = array(
      '#value' => '</div>',
    );
  }

  $form_state['layout'] = $layout;

  // Always show a Save button even if they sent in a Next or something similar
  // button.
  if ($form_state['finish'] !== t('Save')) {
    $form['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#return_here' => TRUE,
    );
  }

  if (empty($form_state['no buttons'])) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => $form_state['finish'],
    );
  }

  panels_modal_js_includes();
  // @todo -- why dnd? That's wrong.
  drupal_add_css(panels_get_path('css/panels_dnd.css'));
  $settings['panels']['checkboxes']['#panel-settings-individual'] = $checkboxes;

  drupal_add_js($settings, 'setting');
  return $form;
}

/**
 * Validate the layout settings form.
 */
function panels_edit_layout_settings_form_validate($form, &$form_state) {
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings validate')) {
    $function($form_state['values']['layout_settings'], $form['layout_settings'], $form_state['display'], $form_state['layout'], $form_state['display']->layout_settings);
  }
}

/**
 * Store changes from the layout settings form.
 */
function panels_edit_layout_settings_form_submit($form, &$form_state) {
  $display = &$form_state['display'];
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings submit')) {
    $function($form_state['values']['layout_settings'], $display, $form_state['layout'], $display->layout_settings);
  }

  if (isset($form_state['values']['display_title']['title'])) {
    $display->title = $form_state['values']['display_title']['title'];
    $display->hide_title = $form_state['values']['display_title']['hide_title'];
  }

  // Remove buttons so they don't get written into the database:
  if (isset($form_state['values']['panel_settings']['edit_style'])) {
    unset($form_state['values']['panel_settings']['edit_style']);
  }
  if (isset($form_state['values']['panel_settings']['edit_style_destination'])) {
    unset($form_state['values']['panel_settings']['edit_style_destination']);
  }

  foreach ($form_state['values']['panel_settings']['panel'] as $id => $info) {
    if (isset($form_state['values']['panel_settings']['panel'][$id]['edit_style'])) {
      unset($form_state['values']['panel_settings']['panel'][$id]['edit_style']);
    }
    if (isset($form_state['values']['panel_settings']['panel'][$id]['edit_style_destination'])) {
      unset($form_state['values']['panel_settings']['panel'][$id]['edit_style_destination']);
    }
  }

  // Since not all layouts have layout settings, check here in case of notices.
  if (isset($form_state['values']['layout_settings'])) {
    $display->layout_settings = $form_state['values']['layout_settings'];
  }
  $display->panel_settings = $form_state['values']['panel_settings'];

}

// ------------------------------------------------------------------
// Panels settings + ajax for modal popup

/**
 * AJAX incoming to deal with the style settings modal
 *
 * @ingroup panels_ajax
 */
function panels_ajax_style_settings($did = NULL, $panel = NULL, $name = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');

  if ($name == '0') {
    panels_ajax_render(t('There are no style settings to edit.'), t('Edit default style settings'));
  }

  $style = panels_get_style($name);
  $style_settings = panels_cache_get('style_settings', $did);
  if (!isset($style_settings)) {
    panels_ajax_render();
  }

  if (!isset($style_settings[$panel])) {
    $style_settings[$panel] = array();
  }

  $form_state = array(
    'did' => $did,
    'style' => $style,
    'style_settings' => &$style_settings[$panel],
    'ajax' => TRUE,
    'title' =>  t('Edit style settings for @style', array('@style' => $style['title'])),
  );

  $output = panels_ajax_form_wrapper('panels_style_settings_form', $form_state);
  if (!$output) { // no output means submit
    panels_cache_set('style_settings', $did, $style_settings);
    $output = 'dismiss';
  }

  panels_ajax_render($output);
}

/**
 * Form for the style settings modal.
 *
 * @ingroup panels_ajax
 */
function panels_style_settings_form(&$form_state) {
  $did = $form_state['did'];
  $style = &$form_state['style'];
  $style_settings = &$form_state['style_settings'];

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($style['description']),
  );

  if (isset($style['settings form']) && function_exists($style['settings form'])) {
    $form['style_settings'] = $style['settings form']($style_settings);
    $form['style_settings']['#tree'] = TRUE;
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
  }
  else {
    $form['markup'] = array(
      '#value' => t('This style does not have any settings.'),
    );
  }

  return $form;
}

/**
 * Allows panel styles to validate their style settings.
 */
function panels_style_settings_form_validate($form, &$form_state) {
  $style = $form_state['style'];

  if ($function = panels_plugin_get_function('style', $style, 'settings form validate')) {
    $function($form, $form_state['values']['style_settings']);
  }
}

/**
 * Allows panel styles to make changes to their style settings during submit.
 */
function panels_style_settings_form_submit($form, &$form_state) {
  $style = $form_state['style'];

  if ($function = panels_plugin_get_function('style', $style, 'settings form submit')) {
    $function($form_state['values']['style_settings']);
  }

  // Transfer input values onto the style settings.
  $form_state['style_settings'] = $form_state['values']['style_settings'];
}
