<?php
// $Id$


/**
 * implementation of hook_panels_layouts
 */
function panels_onecol_panels_layouts() {
  $items['onecol'] = array(
    'module' => 'panels',
    'title' => t('Single column'),
    'icon' => 'layouts/onecol.png',
    'theme' => 'panels_onecol',
    'theme arguments' => array('id', 'content'), 
    'css' => 'layouts/onecol.css',
    'panels' => array('middle' => t('Middle column')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_onecol($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-display panel-1col clear-block" $idstr>
  <div class="panel-panel panel-col">
    <div>$content[middle]</div>
  </div>
</div>
EOT;
  return $output;
}

