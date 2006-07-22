<?php
// $Id$

/**
 * implementation of hook_panels_layouts
 */
function panels_threecol_25_50_25_stacked_panels_layouts() {
  $items['threecol_25_50_25_stacked'] = array(
    'module' => 'panels',
    'path' => 'layouts',
    'title' => t('Three column 25/50/25 stacked'),
    'icon' => 'layouts/threecol_25_50_25_stacked.png',
    'theme' => 'panels_threecol_25_50_25_stacked',
    'css' => 'layouts/threecol_25_50_25_stacked.css',
    'content areas' => array('top' => t('Top'), 'left' => t('Left side'), 'middle' => t('Middle column'), 'right' => t('Right side'), 'bottom' => t('Bottom')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_threecol_25_50_25_stacked($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-3col-stacked" $idstr>
  <div class="panel-col-top">
    $content[top]
  </div>
  <div class="panel-col-first">
    $content[left]
  </div>

  <div class="panel-col">
    $content[middle]
  </div>

  <div class="panel-col-last">
    $content[right]
  </div>
  <div class="panel-col-bottom">
    $content[bottom]
  </div>
</div>
EOT;
  return $output;
}
