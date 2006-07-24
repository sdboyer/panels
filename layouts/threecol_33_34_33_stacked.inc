<?php
// $Id$

/**
 * implementation of hook_panels_layouts
 */
function panels_threecol_33_34_33_stacked_panels_layouts() {
  $items['threecol_33_34_33_stacked'] = array(
    'module' => 'panels',
    'path' => 'layouts',
    'title' => t('Three column 33/34/33 stacked'),
    'icon' => 'layouts/threecol_33_34_33_stacked.png',
    'theme' => 'panels_threecol_33_34_33_stacked',
    'css' => 'layouts/threecol_33_34_33_stacked.css',
    'content areas' => array('top' => t('Top'), 'left' => t('Left side'), 'middle' => t('Middle column'), 'right' => t('Right side'), 'bottom' => t('Bottom')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_threecol_33_34_33_stacked($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-3col-33-stacked" $idstr>
  <div class="panel-col-top">
    <div>$content[top]</div>
  </div>
  <div class="panel-col-first">
    <div>$content[left]</div>
  </div>

  <div class="panel-col">
    <div>$content[middle]</div>
  </div>

  <div class="panel-col-last">
    <div>$content[right]</div>
  </div>
  <div class="panel-col-bottom">
    <div>$content[bottom]</div>
  </div>
</div>
EOT;
  return $output;
}
