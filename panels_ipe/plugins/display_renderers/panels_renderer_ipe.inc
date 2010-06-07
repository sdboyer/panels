<?php

class panels_renderer_ipe extends panels_renderer_standard {
  function render() {
    panels_load_include('display-edit');
    $info = page_manager_get_current_page();
    $this->display->cache_key = $this->display->did;
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
    jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
    return parent::render();
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
    $output = theme('panels_ipe_pane_wrapper', $output, $pane, $this->display);
    return "<div id='panels-ipe-paneid-{$pane->pid}' class='panels-ipe-pane'>" . $output . "</div>";
  }

  function render_region($region_name, $panes) {
    $output = parent::render_region($region_name, $panes);
    $output = theme('panels_ipe_region_wrapper', $output, $region_name, $this->display);
    return "<div id='panels-ipe-regionid-$region_name' class='panels-ipe-region'>" . $output . "</div>";
  }
}
