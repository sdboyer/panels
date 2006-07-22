<?php
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
    $content[top]
  </div>
  <div class="panel-col-first">
    $content[left]
  </div>

  <div class="panel-col-last">
    $content[right]
  </div>
  <div class="panel-col-bottom">
    $content[bottom]
  </div>
</div>
<br clear="both" />
EOT;
  return $output;
}
