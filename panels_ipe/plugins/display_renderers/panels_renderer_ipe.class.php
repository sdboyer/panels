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

  /**
   * Add an 'empty' pane placeholder above all the normal panes, and
   * @param $region_id
   * @param $panes
   */
  function render_region($region_id, $panes) {
    // Generate this region's 'empty' placeholder pane from the IPE plugin.
    $empty_ph = theme('panels_ipe_placeholder_pane', $region_id, $this->plugins['layout']['panels'][$region_id]);
    // Wrap the placeholder in some guaranteed markup.
    $panes['empty_placeholder'] = '<div class="panels-ipe-placeholder panels-ipe-on">' . $empty_ph . "</div>";
    // Generate this region's add new pane button. FIXME waaaaay too hardcoded
    $panes['add_button'] = theme('panels_ipe_add_pane_button', $region_id, $this->display);

    $output = parent::render_region($region_id, $panes);
    $output = theme('panels_ipe_region_wrapper', $output, $region_id, $this->display);
    $classes = 'panels-ipe-region';
    // Use the canonical list of empty regions to determine whether or not this
    // region should initially be marked empty. It is important that we use the
    // list instead of introspecting on render data so that this behavior is
    // easily controlled externally.
    if (array_search($region_id, $this->prepared['empty regions'])) {
      $classes .= ' panels-ipe-region-empty';
    }
    return "<div id='panels-ipe-regionid-$region_id' class='panels-ipe-region'>" . $output . "</div>";
  }
}
