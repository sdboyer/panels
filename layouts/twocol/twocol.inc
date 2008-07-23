<?php
// $Id$

/**
 * Implementation of hook_panels_layouts().
 */
function panels_twocol_panels_layouts() {
  $items['twocol'] = array(
    'title' => t('Two column'),
    'icon' => 'twocol.png',
    'theme' => 'panels_twocol',
    'css' => 'twocol.css',
    'panels' => array(
      'left' => t('Left side'),
      'right' => t('Right side')
    ),
  );

  return $items;
}