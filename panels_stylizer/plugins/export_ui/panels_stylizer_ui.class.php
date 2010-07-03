<?php
// $Id$

class panels_stylizer_ui extends ctools_export_ui {

  function list_form(&$form, &$form_state) {
    ctools_include('stylizer');
    parent::list_form($form, $form_state);

    $all = array('all' => t('- All -'));

    $form['top row']['type'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $all + array(
        'pane' => t('Pane'),
        'region' => t('Region')
      ),
      '#default_value' => 'all',
      '#weight' => -10,
      '#attributes' => array('class' => 'ctools-auto-submit'),
    );

    $plugins = ctools_get_style_bases();
    $form_state['style_plugins'] = $plugins;

    $options = $all;
    foreach ($plugins as $name => $plugin) {
      if ($plugin['module'] == 'panels') {
        $options[$name] = $plugin['title'];
      }
    }

    $form['top row']['base'] = array(
      '#type' => 'select',
      '#title' => t('Base'),
      '#options' => $all + $options,
      '#default_value' => 'all',
      '#weight' => -9,
      '#attributes' => array('class' => 'ctools-auto-submit'),
    );
  }

  function list_sort_options() {
    return array(
      'disabled' => t('Enabled, title'),
      'title' => t('Title'),
      'name' => t('Name'),
      'base' => t('Base'),
      'type' => t('Type'),
      'storage' => t('Storage'),
    );
  }

  function list_filter($form_state, $item) {
    if (empty($form_state['style_plugins'][$item->settings['style_base']])) {
      $this->style_plugin = array(
        'name' => 'broken',
        'title' => t('Missing plugin'),
        'type' => t('Unknown'),
      );
    }
    else {
      $this->style_plugin = $form_state['style_plugins'][$item->settings['style_base']];
    }

    // This isn't really a field, but by setting this we can list it in the
    // filter fields and have the search box pick it up.
    $item->plugin_title = $this->style_plugin['title'];

    if ($form_state['values']['type'] != 'all' && $form_state['values']['type'] != $this->style_plugin['type']) {
      return TRUE;
    }

    if ($form_state['values']['base'] != 'all' && $form_state['values']['base'] != $this->style_plugin['name']) {
      return TRUE;
    }

    return parent::list_filter($form_state, $item);
  }

  function list_search_fields() {
    $fields = parent::list_search_fields();
    $fields[] = 'plugin_title';
    return $fields;
  }

  function list_build_row($item, &$form_state, $operations) {
    // Set up sorting
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$item->name] = empty($item->disabled) . $item->admin_title;
        break;
      case 'title':
        $this->sorts[$item->name] = $item->admin_title;
        break;
      case 'name':
        $this->sorts[$item->name] = $item->name;
        break;
      case 'type':
        $this->sorts[$item->name] = $this->style_plugin['type'] . $item->admin_title;
        break;
      case 'base':
        $this->sorts[$item->name] = $this->style_plugin['title'] . $item->admin_title;
        break;
      case 'storage':
        $this->sorts[$item->name] = $item->type . $item->admin_title;
        break;
    }

    // $this->style_plugin was set up by this::list_filter
    switch ($this->style_plugin['type']) {
      case 'pane':
        $type = t('Pane');
        break;
      case 'region':
        $type = t('Region');
        break;
      default:
        $type = t('Unknown');
    }

    $this->rows[$item->name] = array(
      'data' => array(
        array('data' => $type, 'class' => 'ctools-export-ui-type'),
        array('data' => check_plain($item->name), 'class' => 'ctools-export-ui-name'),
        array('data' => check_plain($item->admin_title), 'class' => 'ctools-export-ui-title'),
        array('data' => check_plain($this->style_plugin['title']), 'class' => 'ctools-export-ui-base'),
        array('data' => check_plain($item->type), 'class' => 'ctools-export-ui-storage'),
        array('data' => theme('links', $operations), 'class' => 'ctools-export-ui-operations'),
      ),
      'title' => check_plain($item->admin_description),
      'class' => !empty($item->disabled) ? 'ctools-export-ui-disabled' : 'ctools-export-ui-enabled',
    );
  }

  function list_table_header() {
    return array(
      array('data' => t('Type'), 'class' => 'ctools-export-ui-type'),
      array('data' => t('Name'), 'class' => 'ctools-export-ui-name'),
      array('data' => t('Title'), 'class' => 'ctools-export-ui-title'),
      array('data' => t('Base'), 'class' => 'ctools-export-ui-base'),
      array('data' => t('Storage'), 'class' => 'ctools-export-ui-storage'),
      array('data' => t('Operations'), 'class' => 'ctools-export-ui-operations'),
    );
  }

  function init($plugin) {
    // Change the item to a tab on the Panels page.
    $plugin['menu']['items']['list callback']['type'] = MENU_LOCAL_TASK;

    $base = $plugin['menu']['items']['add'];

    // Remove the default 'add' menu item.
    unset($plugin['menu']['items']['add']);

    // Create a new menu item for the 'pane' base type.
    $pane = $base;
    $pane['title'] = 'Add pane style';
    $pane['page arguments'][] = 'pane';
    $pane['path'] = 'add-pane';
    $plugin['menu']['items']['add pane'] = $pane;

    // Create a new menu item for the 'region' base type.
    $region = $base;
    $region['title'] = 'Add region style';
    $region['page arguments'][] = 'region';
    $region['path'] = 'add-region';
    $plugin['menu']['items']['add region'] = $region;

    parent::init($plugin);
  }

  function add_page($js, $input) {
    // Get the arg like this because strict mode does not allow us to define
    // the method with different arguments from the parent.
    $args = func_get_args();

    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => ctools_export_crud_new($this->plugin['schema']),
      'op' => 'add',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      // Store these in case additional args are needed.
      'function args' => $args,
      'stylizer type' => $args[2],
    );

    $output = $this->edit_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      $export_key = $this->plugin['export']['key'];
      drupal_goto(str_replace('%ctools_export_ui', $form_state['item']->{$export_key}, $this->plugin['redirect']['add']));
    }

    return $output;

    return $output;
  }

  /**
   * Execute the stylizer edit form.
   *
   * The stylizer forms are significantly complicated by mostly being owned
   * by the stylizer tool, but we have to add a few things.
   */
  function edit_execute_form(&$form_state) {
    ctools_include('stylizer');

    $js = $form_state['ajax'];
    // This is an unfortunately annoying way to get the argument, but it's the 3rd arg passed in.
    $step = isset($form_state['function args'][3]) ? $form_state['function args'][3] : NULL;

    if (!empty($form_state['stylizer type'])) {
      $path = ctools_export_ui_plugin_menu_path($this->plugin, 'add ' . $form_state['stylizer type']);
    }
    else {
      $path = ctools_export_ui_plugin_menu_path($this->plugin, $form_state['op'], $form_state['item']->name);
    }

    $info = array(
      'module' => 'panels',
      'path' => $path . '/%step',
      'owner form' => 'panels_stylizer_edit_style_form',
      'owner form validate' => 'panels_stylizer_edit_style_form_validate',
      'owner form submit' => 'panels_stylizer_edit_style_form_submit',
      'owner settings' => array(
        'name' => $form_state['item']->name,
        'admin_title' => $form_state['item']->admin_title,
        'admin_description' => $form_state['item']->admin_description
      ),
      'settings' => $form_state['item']->settings,
      'form_type' => $form_state['op'],
    );

    if (!empty($form_state['stylizer type'])) {
      $info['type'] = $form_state['stylizer type'];
    }

    $output = ctools_stylizer_edit_style($info, $js, $step);
    if (!empty($info['complete'])) {
      if ($form_state['op'] == 'add') {
        $form_state['item']->name = $info['settings']['name'];
      }
      $form_state['item']->admin_title = $info['owner settings']['admin_title'];
      $form_state['item']->admin_description = $info['owner settings']['admin_description'];
      $form_state['item']->settings = $info['settings'];
      // Don't let name accidentally change:
      $form_state['item']->settings['name'] = $form_state['item']->name;

      $form_state['executed'] = TRUE;
      $this->edit_save_form($form_state);
    }

    return $output;
  }

  /**
   * Import form
   *
   * The import wizard and the stylizer wizard do not mesh well, so we have
   * to do our own thing.
   */
  function import_page($js, $input, $step = 'code') {
    drupal_set_title(str_replace('%title', check_plain($this->plugin['title']), $this->plugin['strings']['title']['import']));
    if ($step == 'begin') {
      $form_info = array(
        'id' => 'ctools_export_ui_import',
        'path' => ctools_export_ui_plugin_base_path($this->plugin) . '/' . $this->plugin['menu']['items']['import']['path'] . '/%step',
        'order' => array(
          'code' => t('Import code'),
        ),
        'forms' => array(
          'code' => array(
            'form id' => 'ctools_export_ui_import_code'
          ),
        ),
      );

      $form_state = array(
        'plugin' => $this->plugin,
        'input' => $input,
        'rerender' => TRUE,
        'no_redirect' => TRUE,
        'object' => &$this,
        'export' => '',
        'overwrite' => FALSE,
        // Store these in case additional args are needed.
        'function args' => func_get_args(),
      );

      ctools_include('wizard');
      $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
      if (!empty($form_state['complete'])) {
        return $this->import_page_edit($js, $input);
      }
      return $output;
    }

    return $this->import_page_edit($js, $input);
  }

  function import_page_edit($js, $input) {
    $form_state = array(
      'plugin' => $this->plugin,
      'object' => &$this,
      'ajax' => $js,
      'item' => ctools_export_crud_new($this->plugin['schema']),
      'op' => 'add',
      'rerender' => TRUE,
      'no_redirect' => TRUE,
      // Store these in case additional args are needed.
      'function args' => func_get_args(),
    );

    $form_state['item'] = $_SESSION['ctools_export_ui_import'][$this->plugin['name']];
    $form_state['export'] = $form_state['item']->export_ui_code;
    $form_state['overwrite'] = $form_state['item']->export_ui_allow_overwrite;
    if (!empty($form_state['item']->export_ui_allow_overwrite)) {
      // if allow overwrite was enabled, set this to 'edit' only if the key already existed.
      $export_key = $this->plugin['export']['key'];

      if (ctools_export_crud_load($this->plugin['schema'], $form_state['item']->{$export_key})) {
        $form_state['op'] = 'edit';
      }
    }

    $output = $this->edit_execute_form($form_state);
    if (!empty($form_state['executed'])) {
      unset($_SESSION['ctools_export_ui_import'][$this->plugin['name']]);
      drupal_goto(str_replace('%ctools_export_ui', $form_state['item']->name, $this->plugin['redirect']['import']));
    }

    return $output;
  }
}
