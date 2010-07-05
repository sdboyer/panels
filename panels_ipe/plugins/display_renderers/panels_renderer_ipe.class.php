<?php
// $Id$

/**
 * Renderer class for all In-Place Editor (IPE) behavior.
 */
class panels_renderer_ipe extends panels_renderer_editor {
  // The IPE operates in normal render mode, not admin mode.
  var $admin = FALSE;

  function render() {
    $output = parent::render();
    return "<div id='panels-ipe-display-{$this->display->cache_key}' class='panels-ipe-display-container'>$output</div>";
  }

  function add_meta() {
    ctools_include('display-edit', 'panels');
    ctools_include('content');

    $this->display->cache_key = $this->display->did;
    panels_ipe_get_cache_key($this->display->cache_key);
    $cache = new stdClass();
    $cache->display = $this->display;

    // NO good reason for this to need to be set way out here!?
    $cache->content_types = ctools_content_get_available_types();
    panels_edit_cache_set($cache);

    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();

    ctools_add_css('panels_dnd', 'panels');
    ctools_add_css('panels_admin', 'panels');
    ctools_add_js('panels_ipe', 'panels_ipe');
    ctools_add_css('panels_ipe', 'panels_ipe');

    $settings = array(
      'formPath' => url($this->get_url('save-form')),
    );
    drupal_add_js(array('PanelsIPECacheKeys' => array($this->display->cache_key)), 'setting');
    drupal_add_js(array('PanelsIPESettings' => array($this->display->cache_key => $settings)), 'setting');

    jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
    parent::add_meta();
  }

  /**
   * Override & call the parent, then pass output through to the dnd wrapper
   * theme function.
   *
   * @param $pane
   */
  function render_pane($pane) {
    $output = parent::render_pane($pane);
    if (empty($output)) {
      return;
    }

    // Add an inner layer wrapper to the pane content before placing it into
    // draggable portlet
    $output = "<div class='panels-ipe-portlet-content'>$output</div>";

    // Hand it off to the plugin/theme for placing draggers/buttons
    $output = theme('panels_ipe_pane_wrapper', $output, $pane, $this->display, $this);
    return "<div id='panels-ipe-paneid-{$pane->pid}' class='panels-ipe-portlet-wrapper panels-ipe-portlet-marker'>" . $output . "</div>";
  }

  /**
   * Add an 'empty' pane placeholder above all the normal panes.
   *
   * @param $region_id
   * @param $panes
   */
  function render_region($region_id, $panes) {
    // Generate this region's 'empty' placeholder pane from the IPE plugin.
    $empty_ph = theme('panels_ipe_placeholder_pane', $region_id, $this->plugins['layout']['panels'][$region_id]);

    // Wrap the placeholder in some guaranteed markup.
    $panes['empty_placeholder'] = '<div class="panels-ipe-placeholder panels-ipe-on panels-ipe-portlet-marker panels-ipe-portlet-static">' . $empty_ph . "</div>";

    // Generate this region's add new pane button. FIXME waaaaay too hardcoded
    $panes['add_button'] = theme('panels_ipe_add_pane_button', $region_id, $this->display, $this);

    $output = parent::render_region($region_id, $panes);
    $output = theme('panels_ipe_region_wrapper', $output, $region_id, $this->display);
    $classes = 'panels-ipe-region';

    return "<div id='panels-ipe-regionid-$region_id' class='panels-ipe-region'>" . $output . "</div>";
  }

  /**
   * AJAX entry point to create the controller form for an IPE.
   */
  function ajax_save_form() {
    ctools_include('form');

    $form_state = array(
      'display' => &$this->display,
      'content_types' => $this->cache->content_types,
      'rerender' => FALSE,
      'no_redirect' => TRUE,
      // Panels needs this to make sure that the layout gets callbacks
      'layout' => $this->plugins['layout'],
    );

    $output = ctools_build_form('panels_ipe_edit_control_form', $form_state);
    if ($output) {
      $this->commands[] = array(
        'command' => 'initIPE',
        'key' => $this->display->cache_key,
        'data' => $output,
      );
      return;
    }

    // no output == submit
    if (!empty($form_state['clicked_button']['#save-display'])) {
      // saved
      panels_save_display($this->display);
    }
    else {
      // canceled
      panels_cache_clear($this->display->cache_key);
    }

    $this->commands[] = array(
      'command' => 'endIPE',
      'key' => $this->display->cache_key,
      'data' => $output,
    );
  }

  /**
   * Create a command array to redraw a pane.
   */
  function command_update_pane($pid) {
    if (is_object($pid)) {
      $pane = $pid;
    }
    else {
      $pane = $this->display->content[$pid];
    }

    $this->commands[] = ctools_ajax_command_replace("#panels-ipe-paneid-$pane->pid", $this->render_pane($pane));
//    $this->commands[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
  }

  /**
   * Create a command array to add a new pane.
   */
  function command_add_pane($pid) {
    if (is_object($pid)) {
      $pane = $pid;
    }
    else {
      $pane = $this->display->content[$pid];
    }

    $this->commands[] = ctools_ajax_command_append("#panels-ipe-regionid-$pane->panel div.panels-ipe-sort-container", $this->render_pane($pane));
//    $this->commands[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
  }
}

/**
 * FAPI callback to create the Save/Cancel form for the IPE.
 */
function panels_ipe_edit_control_form(&$form_state) {
  $display = &$form_state['display'];
  $display->cache_key = isset($display->cache_key) ? $display->cache_key : $display->did;

  // Annoyingly, theme doesn't have access to form_state so we have to do this.
  $form['#display'] = $display;

  $layout = panels_get_layout($display->layout);
  $layout_panels = panels_get_regions($layout, $display);

  $form['panel'] = array('#tree' => TRUE);
  $form['panel']['pane'] = array('#tree' => TRUE);

  foreach ($layout_panels as $panel_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($display->panels[$panel_id])) {
      $display->panels[$panel_id] = array();
    }

    $form['panel']['pane'][$panel_id] = array(
      // Use 'hidden' instead of 'value' so the js can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', (array) $display->panels[$panel_id]),
    );
  }

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#id' => 'panels-ipe-save',
    '#submit' => array('panels_edit_display_form_submit'),
    '#save-display' => TRUE,
  );
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );
  return $form;
}
