<?php
// $Id$

/**
 * implementation of hook_panels_layouts
 */
function panels_twocol_stacked_panels_layouts() {
  $items['twocol_stacked'] = array(
    'module' => 'panels',
    'path' => 'layouts',
    'title' => t('Two column stacked'),
    'icon' => 'layouts/twocol_stacked.png',
    'theme' => 'panels_twocol_stacked',
    'css' => 'layouts/twocol_stacked.css',
    'content areas' => array('top' => t('Top'), 'left' => t('Left side'), 'right' => t('Right side'), 'bottom' => t('Bottom')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_twocol_stacked($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-2col-stacked" $idstr>
  <div class="panel-col-top">
    <div>$content[top]</div>
  </div>
  <div class="panel-col-first">
    <div>$content[left]</div>
  </div>

  <div class="panel-col-last">
    <div>$content[right]</div>
  </div>
  <div class="panel-col-bottom">
    <div>$content[bottom]</div>
  </div>
</div>
<br clear="both" />
EOT;
  return $output;
}
