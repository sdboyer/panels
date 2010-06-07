<?php

class panels_renderer_single_pane extends panels_renderer_standard {
  /**
   * The pane id of the pane that will be rendered by a call to the render()
   * method. Numeric int or string (typically if a new-# id has been used).
   * @var mixed
   */
  var $render_pid;

  /**
   * Modified build method (vs. panels_renderer_standard::build()); takes a display and the pid of the pane to render.
   * @param $display
   */
  function build(&$display, $pid) {
    $this->display = &$display;
    $this->render_pid = $pid;
  }

  function render() {
    // If the requested pid does not exist,
    if (empty($this->display->content[$this->render_pid])) {
      return NULL;
    }
    return $this->render_pane($this->display->content[$this->render_pid]);
  }

  function render_single($pid) {
    return $this->render_pane($this->display->content[$pid]);
  }
}