<?php

class panels_renderer_ipe extends panels_renderer_standard {
  function render() {
    //drupal_add_css('path/to/css');
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
    $output = theme('panels_ipe_pane_wrapper', $output, $pane, $this->display);
    return "<div id='' class='panels-ipe-draggable'>" . $output . "</div>";
  }

  function render_region($region_name, $panes) {
    $output = parent::render_region($region_name, $panes);
    return $output;
  }
}
