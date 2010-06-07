<?php

class panels_renderer_ipe extends panels_renderer_standard {

  function render() {
    //drupal_add_css('path/to/css');
    jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
    return parent::render();
  }

  /**
   * Override & call the parent, then add a class to be caught by the js for dnd
   * @param $pane
   */
  function render_pane_content($pane) {
    $content = parent::render_pane_content($pane);
    if (!empty($content->css_class)) {
      $content->css_class .= ' panel-pane-ipe';
    }
    else {
      $content->css_class = 'panel-pane-ipe';
    }

    $content->content = theme('panels_ipe_dnd_additions', $content->content, $pane);
    return $content;
  }
}
