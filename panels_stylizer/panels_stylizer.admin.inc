<?php
// $Id$

/**
 * @file
 * Contains admin pages for stylizer integration for Panels.
 */


/**
 * Add owner information to a style form.
 *
 * This allows Panels to add administrative info to a style for saving
 * it in the database.
 */
function panels_stylizer_edit_style_form(&$form, &$form_state) {
  $settings = $form_state['owner info']['owner settings'];

  $form['panels']['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('The name of this style. This will appear in the administrative interface to easily identify it.'),
    '#default_value' => $settings['admin_title'],
  );

  $form['panels']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('The machine readable name of this page. It must be unique, and it must contain only alphanumeric characters and underscores. Once created, you will not be able to change this value!'),
    '#default_value' => $settings['name'],
  );

  if ($form_state['form_type'] != 'add') {
    $form['panels']['name']['#disabled'] = TRUE;
    $form['panels']['name']['#value'] = $settings['name'];
  }

  $form['panels']['admin_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Administrative description'),
    '#description' => t('A description of what this style is, does or is for, for administrative use.'),
    '#default_value' => $settings['admin_description'],
  );
}

function panels_stylizer_edit_style_form_validate(&$form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == t('Next') || $form_state['clicked_button']['#value'] == t('Finish')) {
    if (empty($form_state['values']['admin_title'])) {
      form_error($form['panels']['admin_title'], t('You must choose an administrative title.'));
    }

    // If this is new, make sure the name is unique:
    if ($form_state['form_type'] == 'add') {
      if (empty($form_state['values']['name'])) {
        form_error($form['panels']['name'], t('You must choose a machine name.'));
      }

      $test = panels_stylizer_load($form_state['values']['name']);
      if ($test) {
        form_error($form['panels']['name'], t('That name is used by another style: @page', array('@page' => $test->admin_title)));
      }

      // Ensure name fits the rules:
      if (preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
        form_error($form['panels']['name'], t('Name must be alphanumeric or underscores only.'));
      }
    }
  }
}

function panels_stylizer_edit_style_form_submit(&$form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == t('Next') || $form_state['clicked_button']['#value'] == t('Finish')) {
    $form_state['owner info']['owner settings']['admin_title'] = $form_state['values']['admin_title'];
    $form_state['owner info']['owner settings']['admin_description'] = $form_state['values']['admin_description'];

    // Clean up preview files before we set the name
    ctools_stylizer_cleanup_style($form_state['plugin'], $form_state['settings']);

    $form_state['settings']['name'] = $form_state['values']['name'];
    $form_state['name'] = $form_state['values']['name'];
  }
}

/**
 * Allow on-the-fly creation of styles in panes.
 */
function panels_stylizer_pane_add_style($type, $cache, $pid = '', $step = NULL) {
  // This must be reached through the Panels style edit dialog, so reject
  // it if the cache is not populated:
  $info = panels_edit_style_get_style($type, $cache, $pid);
  $style = $info[0];
  $conf = &$info[1];

  ctools_include('stylizer');
  $js = FALSE;

  $cache_key = $cache->display->cache_key;
  $path = "admin/build/panels/style/pane-style/$type/$cache_key/$pid/%step";

  $info = array(
    'module' => 'panels',
    'type' => $type == 'panel' ? 'region' : 'pane',
    'path' => $path,
    'modal' => t('Create custom style'),
    'owner form' => 'panels_stylizer_edit_pane_style_form',
    'owner form validate' => 'panels_stylizer_edit_pane_style_form_validate',
    'owner form submit' => 'panels_stylizer_edit_pane_style_form_submit',
    'owner settings' => array('preconfigured' => FALSE, 'name' => '', 'admin_title' => '', 'admin_description' => ''),
    'cache' => &$cache,
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

    // $conf was a reference so it should just modify.
    panels_edit_cache_set($cache);

    $output = array();
    $output[] = ctools_modal_command_dismiss();

    if ($type == 'pane') {
      $pane = &$cache->display->content[$pid];
      $output[] = ctools_ajax_command_replace("#panel-pane-$pid", panels_show_pane($cache->display, $pane, TRUE));
      $output[] = ctools_ajax_command_changed("#panel-pane-$pid", "div.grabber span.text");
    }
    else if ($type == 'panel') {
      $links = panels_edit_panel_get_links($cache->display, $pid);
      $output[] = ctools_ajax_command_replace('.panels-region-links-' . $pid, $links);
    }
    else {
      $links = panels_edit_display_get_links($cache->display);
      $output[] = ctools_ajax_command_replace('.panels-display-links', $links);
    }
  }

  ctools_ajax_render($output);
}

/**
 * The form for determining if a pane should create a local style or a
 * preconfigured style.
 */
function panels_stylizer_edit_pane_style_form(&$form, &$form_state) {
  if (!user_access('administer panels styles')) {
    return;
  }

  panels_stylizer_edit_style_form($form, $form_state);
  $settings = $form_state['owner info']['owner settings'];

  // Add the checkbox, set the weight early
  $form['panels']['preconfigured'] = array(
    '#type' => 'checkbox',
    '#title' => t('Make this style available to other regions or panes'),
    '#default_value' => $settings['name'],
    '#weight' => -1,
  );

  // Make the other form items dependent upon it.
  ctools_include('dependent');
  $depends = array(
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-preconfigured' => array(1)),
  );

  $form['panels']['name'] += $depends;
  $form['panels']['admin_title'] += $depends;
  $form['panels']['admin_description'] += $depends;

  // Add a revert button to get rid of an existing style.
  if ($form_state['form_type'] != 'add') {
    $form['buttons']['revert'] = array(
      '#type' => 'submit',
      '#value' => t('Choose a preconfigured style'),
    );
  }
}

/**
 * Validate to see if we need to check the preconfigured values.
 */
function panels_stylizer_edit_pane_style_form_validate(&$form, &$form_state) {
  if (!user_access('administer panels styles')) {
    return;
  }

  // Only validate if preconfigured is checked.
  if ($form_state['values']['preconfigured']) {
    panels_stylizer_edit_style_form_validate($form, $form_state);
  }
}

/**
 * Store the preconfigured values.
 */
function panels_stylizer_edit_pane_style_form_submit(&$form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == t('Choose a preconfigured style')) {
    // If they chose to revert, get rid of these settings and go back to the
    // normal style settings page.
    $cache = &$form_state['owner info']['cache'];
    $conf = &$form_state['owner info']['conf'];

    $conf['style'] = '';
    if (isset($conf['settings'])) {
      unset($conf['settings']);
    }

    // $conf was a reference so it should just modify.
    panels_edit_cache_set($cache);

    $type = $form_state['owner info']['type'] == 'region' ? 'panel' : 'pane';
    $pid = $form_state['owner info']['pid'];

    ctools_include('display-edit', 'panels');
    panels_ajax_style_settings($type, $cache, $pid);
  }

  if (!user_access('administer panels styles')) {
    return;
  }

  // Only validate if preconfigured is checked.
  if ($form_state['values']['preconfigured']) {
    panels_stylizer_edit_style_form_submit($form, $form_state);
    $form_state['owner info']['owner settings']['preconfigured'] = $form_state['values']['preconfigured'];
  }
}
