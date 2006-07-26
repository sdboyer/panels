<?php
/**
 * implementation of hook_panels_layouts
 */
function panels_threecol_25_50_25_panels_layouts() {
  $items['threecol_25_50_25'] = array(
    'module' => 'panels',
    'title' => t('Three column 25/50/25'),
    'icon' => 'layouts/threecol_25_50_25.png',
    'theme' => 'panels_threecol_25_50_25',
    'css' => 'layouts/threecol_25_50_25.css',
    'content areas' => array('left' => t('Left side'), 'middle' => t('Middle column'), 'right' => t('Right side')),
  );

  return $items;
}

/**
 * This function uses heredoc notation to make it easier to convert
 * to a template.
 */
function theme_panels_threecol_25_50_25($id, $content) {
  if ($id) {
    $idstr = " id='$id'";
  }

  $output = <<<EOT
<div class="panel-3col" $idstr>
  <div class="panel-col-first">
    <div>$content[left]</div>
  </div>

  <div class="panel-col">
    <div>$content[middle]</div>
  </div>

  <div class="panel-col-last">
    <div>$content[right]</div>
  </div>
</div>
EOT;
  return $output;
}
