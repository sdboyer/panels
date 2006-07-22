<?php
/**
 * implementation of hook_panels_layouts
 */
function panels_threecol_33_34_33_panels_layouts() {
  $items['threecol_33_34_33'] = array(
    'module' => 'panels',
    'path' => 'layouts',
    'title' => t('Three column 33/34/33'),
    'icon' => 'layouts/threecol_33_34_33.png',
    'theme' => 'panels_threecol_33_34_33',
    'css' => 'layouts/threecol_33_34_33.css',
    'content areas' => array('left' => t('Left side'), 'middle' => t('Middle column'), 'right' => t('Right side')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_threecol_33_34_33($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-3col-33" $idstr>
  <div class="panel-col-first">
    $content[left]
  </div>

  <div class="panel-col">
    $content[middle]
  </div>

  <div class="panel-col-last">
    $content[right]
  </div>
</div>
EOT;
  return $output;
}
