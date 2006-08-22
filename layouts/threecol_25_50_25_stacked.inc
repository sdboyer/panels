<?php
// $Id$

/**
 * implementation of hook_panels_layouts
 */
function panels_threecol_25_50_25_stacked_panels_layouts() {
  $items['threecol_25_50_25_stacked'] = array(
    'module' => 'panels',
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
<br class="panel-clearer" />
EOT;
  return $output;
}
