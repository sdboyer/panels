<?php


/**
 * Base renderer class for all In-Place Editor (IPE) behavior.
 *
 */
class panels_renderer_ipe extends panels_renderer_standard {
  function render() {
    $output = parent::render();
    return "<div id='panels-ipe-display-{$this->display->cache_key}' class='panels-ipe-display-container'>$output</div>";
  }

  function add_meta() {
    $this->display->cache_key = $this->display->did;
    panels_ipe_get_cache_key($this->display->cache_key);
    panels_load_include('display-edit');
    $cache = new stdClass();
    $cache->display = $this->display;
    ctools_include('content');
    // NO good reason for this to need to be set way out here!?
    $cache->content_types = ctools_content_get_available_types();
    panels_edit_cache_set($cache);
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();
    drupal_add_css(panels_get_path('css/panels_admin.css'));
    drupal_add_css(panels_get_path('css/panels_dnd.css'));
    drupal_add_css(drupal_get_path('module', 'panels_ipe') . '/panels_ipe.css');
    drupal_add_js(drupal_get_path('module', 'panels_ipe') . '/panels_ipe.js');
    $settings = array(
      'formPath' => '/panels_ipe/ajax/edit/' . $this->display->cache_key,
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
    $output = theme('panels_ipe_pane_wrapper', $output, $pane, $this->display);
    return "<div id='panels-ipe-paneid-{$pane->pid}' class='panels-ipe-portlet-wrapper'>" . $output . "</div>";
  }

  function render_region($region_name, $panes) {
    $output = parent::render_region($region_name, $panes);
    $output = theme('panels_ipe_region_wrapper', $output, $region_name, $this->display);
    return "<div id='panels-ipe-regionid-$region_name' class='panels-ipe-region'>" . $output . "</div>";
  }
}
