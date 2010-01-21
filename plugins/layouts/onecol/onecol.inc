<?php
// $Id$

/**
 * implementation of hook_panels_layouts()
 */
// Plugin definition
$plugin = array(
  'title' => t('Single column'),
  'icon' => 'onecol.png',
  'theme' => 'panels_onecol',
  'css' => 'onecol.css',
  'panels' => array('middle' => t('Middle column')),
);
