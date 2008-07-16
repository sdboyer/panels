<?php
// $Id$


/**
 * @file twocol_bricks.inc
 *
 * Implementation for the two column bricked layout
 */

/**
 * implementation of hook_panels_layouts
 */
function panels_twocol_bricks_panels_layouts() {
  $items['twocol_bricks'] = array(
    'module' => 'panels',
    'title' => t('Two column bricks'),
    'icon' => 'layouts/twocol_bricks.png',
    'theme' => 'panels_twocol_bricks',
    'theme arguments' => array('id', 'content'), 
    'css' => 'layouts/twocol_bricks.css',
    'panels' => array(
      'top' => t('Top'),
      'left_above' => t('Left above'),
      'right_above' => t('Right above'),
      'middle' => t('Middle'),
      'left_below' => t('Left below'),
      'right_below' => t('Right below'),
      'bottom' => t('Bottom'),
    ),
  );
  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_twocol_bricks($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-display panel-2col-bricks clear-block" $idstr>
  <div class="panel-panel panel-col-top">
    <div class="inside">$content[top]</div>
  </div>
  <div class="center-wrapper">
    <div class="panel-panel panel-col-first">
      <div class="inside">$content[left_above]</div>
    </div>

    <div class="panel-panel panel-col-last">
      <div class="inside">$content[right_above]</div>
    </div>
  </div>
  <div class="panel-panel panel-col-middle">
    <div class="inside">$content[middle]</div>
  </div>
  <div class="center-wrapper">
    <div class="panel-panel panel-col-first">
      <div class="inside">$content[left_below]</div>
    </div>

    <div class="panel-panel panel-col-last">
      <div class="inside">$content[right_below]</div>
    </div>
  </div>
  <div class="panel-panel panel-col-bottom">
    <div class="inside">$content[bottom]</div>
  </div>
</div>
EOT;
  return $output;
}

