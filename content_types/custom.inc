<?php
/**
 * Callback function to supply a list of content types.
 */
function panels_custom_panels_content_types() {
  $items['custom'] = array(
    'callback' => 'panels_content_custom',
    'admin' => 'panels_admin_custom',
  );
  return $items;
}

/**
 * Output function for the 'custom' content type. Outputs a custom
 * based on the module and delta supplied in the configuration.
 */
function panels_content_custom($conf) {
  $title = filter_xss_admin($conf['title']);
  $body = check_markup($conf['body'], $conf['format'], FALSE);
  return theme('panels_content_custom', $title, $body);
}

function theme_panels_content_custom($title, $body) {
  $output = '<div class="panel-custom">';
  if ($title) {
    $output .= '<h2 class="title">' . $title . '</h2>';
  }
  $output .= $body;
  $output .= '</div>';
EOT;
  return $output;
}
/** 
 * Callback to perform administrative functions on the content block
 */
function panels_admin_custom($op, &$arg, $arg2 = NULL) {
  switch ($op) {
    case 'list':
      $conf = $arg;
      return '<strong>Custom</strong>: ' . filter_xss_admin($conf['title']);
    case 'add button':
      $form['title'] = array(
        '#title' => t('Enter an optional title for custom content you define'),
        '#type' => 'textfield',
        '#maxlength' => 512,
        '#weight'  => -10,
      );
      $form['submit'] = array(
        '#type' => 'button',
        '#value' => t('Add custom'),
      );

      $form['#prefix'] = '<div class="container-inline">';
      $form['#suffix'] = '</div>';
      return $form;
    case 'add':
      if ($_POST['op'] != t('Add custom')) {
        return;
      }
      return $arg;
    case 'edit':
      $conf = &$arg;
      $form['title'] = array(
        '#type' => 'textfield',
        '#default_value' => $conf['title'],
        '#title' => t('Title'),
        '#description' => t('Title'),
        '#size' => 15,
      );
      $form['body'] = array(
        '#title' => t('Body'),
        '#type' => 'textarea',
        '#default_value' => $conf['body'],
        '#rows' => 10,
        '#cols' => 20,
      );
      $arg2[] = 'format';
      $form['format'] = filter_form($conf['format'], 1, $arg2);

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
