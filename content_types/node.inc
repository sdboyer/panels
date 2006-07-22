<?php
/**
 * Callback function to supply a list of content types.
 */
function panels_node_panels_content_types() {
  $items['node'] = array(
    'callback' => 'panels_content_node',
    'admin' => 'panels_admin_node',
  );
  return $items;
}

/**
 * Output function for the 'node' content type. Outputs a node
 * based on the module and delta supplied in the configuration.
 */
function panels_content_node($conf) {
  $node = node_load($conf['nid']);
  $output = node_view($node, $conf['teaser'], FALSE, $conf['links']);
  return $output;
}

/** 
 * Callback to perform administrative functions on the content block
 */
function panels_admin_node($op, &$arg, $arg2 = NULL) {
  switch ($op) {
    case 'list':
      $conf = $arg;
      $node = node_load($conf['nid']);
      return '<strong>Node</strong>: ' . check_plain($node->title);
    case 'add button':
      $form['nid'] = array(
        '#title' => t('Enter the title or NID of a post'),
        '#type' => 'textfield',
        '#maxlength' => 512,
        '#autocomplete_path' => 'panels/node/autocomplete',
        '#weight'  => -10,
      );
      $form['submit'] = array(
        '#type' => 'button',
        '#value' => t('Add post'),
      );

      $form['#prefix'] = '<div class="container-inline">';
      $form['#suffix'] = '</div>';
      return $form;
    case 'add':
      if ($_POST['op'] != t('Add post')) {
        return;
      }
      $form = &$arg;
      if (is_numeric($form['nid'])) {
        $conf = array();
        $conf['nid'] = $form['nid'];
      }
      else {
        $conf = db_fetch_array(db_query(db_rewrite_sql("SELECT n.nid FROM {node} n WHERE LOWER(title) = LOWER('%s')"), $form['nid']));
        if (!$conf['nid']) {
          drupal_set_message(t('Unable to find "%s"', array('%s' => check_plain($form['nid']))));
          return;
        }
      }
      // default to just teaser
      $conf['teaser'] = TRUE;
      return $conf;
    case 'edit':
      $conf = &$arg;
      $form['nid'] = array(
        '#type' => 'hidden',
        '#default_value' => $conf['nid'],
      );
      $form['teaser'] = array(
        '#title' => t('Teaser'),
        '#type' => 'checkbox',
        '#default_value' => $conf['teaser'],
        '#description' => t('Check here to show only the node teaser'),
      );
      $form['links'] = array(
        '#type' => 'checkbox',
        '#default_value' => $conf['links'],
        '#title' => t('Display links'),
        '#description' => t('Check here to display the links with the post.')
      );

      return $form;
    case 'validate':
      // This one has nothing to validate.
      $form = &$arg;
      return;
    case 'save':
      // For this one, the form values go directly into the config.
      $form = &$arg;
      return $form;
  }
}
