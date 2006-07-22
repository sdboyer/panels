<?php
/**
 * Callback function to supply a list of content types.
 */
function panels_block_panels_content_types() {
  $items['block'] = array(
    'callback' => 'panels_content_block',
    'admin' => 'panels_admin_block',
  );
  return $items;
}

/**
 * Output function for the 'block' content type. Outputs a block
 * based on the module and delta supplied in the configuration.
 */
function panels_content_block($conf) {
  $block = (object) module_invoke($conf['module'], 'block', 'view', $conf['delta']);
  $block->module = $conf['module'];
  $block->delta = $conf['delta'];
  if ($conf['override_title']) {
    $block->subject = check_plain($conf['override_title_text']);
  }

  $output = theme('block', $block);
  return $output;
}

/** 
 * Callback to perform administrative functions on the content block
 */
function panels_admin_block($op, &$arg, $arg2 = NULL) {
  switch ($op) {
    case 'list':
      $conf = $arg;
      $block = module_invoke($conf['module'], 'block', 'list');
      $title = $block[$conf['delta']]['info'];
      if ($conf['override_title']) {
        $title .= ' [' . check_plain($conf['override_title_text']) . ']';
      }
      return '<strong>Block</strong>: ' . $title . ' (' . $conf['module'] . '-' . $conf['delta'] . ')';
    case 'add button':
      foreach (module_list() as $module) {
        $module_blocks = module_invoke($module, 'block', 'list');
        if ($module_blocks) {
          $array = array();
          foreach ($module_blocks as $delta => $block) {
            // strip_tags used because it goes through check_plain and that
            // just looks bad.
            $array["$module-$delta"] = strip_tags($block['info']);
          }
          $options[$module] = $array;
        }
      }
      $form['block'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#title' => t('Block'),
      );
      $form['submit'] = array(
        '#type' => 'button',
        '#value' => t('Add block'),
      );

      $form['#prefix'] = '<div class="container-inline">';
      $form['#suffix'] = '</div>';
      return $form;
    case 'add':
      if ($_POST['op'] != t('Add block')) {
        return;
      }
      $form = &$arg;
      $conf = array();
      list($conf['module'], $conf['delta']) = explode('-', $form['block'], 2);
      // take the data given to us and return a fully formed $area object.
      return $conf;
    case 'edit':
      $conf = &$arg;
      $form['module'] = array(
        '#type' => 'hidden',
        '#default_value' => $conf['module'],
      );
      $form['delta'] = array(
        '#type' => 'hidden',
        '#default_value' => $conf['delta'],
      );
      $form['override_title'] = array(
        '#type' => 'checkbox',
        '#default_value' => $conf['override_title'],
        '#title' => t('Override title'),
        '#description' => t('If checked, the block title will be overridden with the override title text.')
      );
      $form['override_title_text'] = array(
        '#type' => 'textfield',
        '#default_value' => $conf['override_title_text'],
        '#title' => t('Override title text'),
        '#size' => 15,
      );

      return $form;
    case 'validate':
      // This one has nothing to validate.
      $form_values = &$arg;
      $form = $arg2;
      return;
    case 'save':
      // For this one, the form values go directly into the config.
      $form = &$arg;
      return $form;
  }
}
