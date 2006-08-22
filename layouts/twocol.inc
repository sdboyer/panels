<?php
// $Id$
/**
 * implementation of hook_panels_layouts
 */
function panels_twocol_panels_layouts() {
  $items['twocol'] = array(
    'module' => 'panels',
    'title' => t('Two column'),
    'icon' => 'layouts/twocol.png',
    'theme' => 'panels_twocol',
    'css' => 'layouts/twocol.css',
    'content areas' => array('left' => t('Left side'), 'right' => t('Right side')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_twocol($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-2col" $idstr>
  <div class="panel-col-first">
    <div>$content[left]</div>
  </div>

  <div class="panel-col-last">
    <div>$content[right]</div>
  </div>
</div>
<br class="panel-clearer" />
EOT;
  return $output;
}
