<?php
// $Id$

$plugin = array(
  'schema' => 'panels_style',
  'access' => 'administer panels styles',

  'menu' => array(
    'menu prefix' => 'admin/build/panels',
    'menu item' => 'stylizer',
    'menu title' => 'Styles',
    'menu description' => 'Add, edit or delete stylizer styles.',
  ),

  'title singular' => t('style'),
  'title singular proper' => t('Style'),
  'title plural' => t('styles'),
  'title plural proper' => t('Styles'),

  'handler' => array(
    'class' => 'panels_stylizer_ui',
    'parent' => 'ctools_export_ui',
  ),
);

