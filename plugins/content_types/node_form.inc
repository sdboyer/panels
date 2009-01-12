<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_form_panels_content_types() {
  $items['node_form'] = array(
    'title' => t('Node form'),
    'content_types' => 'panels_admin_content_types_node_form',
    'render callback' => 'panels_content_node_form',
    'add callback' => 'panels_admin_edit_node_form',
    'edit callback' => 'panels_admin_edit_node_form',
    'title callback' => 'panels_admin_title_node_form',
  );
  return $items;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_form() {
  $content = array(
    'publishing' => array(
      'title' => t('Node form publishing options'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Publishing options on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    ),
    'author' => array(
      'title' => t('Node form author information'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Author information on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    ),
    'input_format' => array(
      'title' => t('Node form input format'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Input format for the body field on a node.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    ),
    'log' => array(
      'title' => t('Node form log message'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Log message for the node.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    ),
    'buttons' => array(
      'title' => t('Node form submit buttons'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Submit buttons for the node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    ),
  );
  if (module_exists('comment')) {
    $content['comment'] = array(
      'title' => t('Node form comment settings'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Comment settings on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  if (module_exists('menu')) {
    $content['menu'] = array(
      'title' => t('Node form menu settings'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('menu settings on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  if (module_exists('path')) {
    $content['url_path'] = array(
      'title' => t('URL Path settings'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Publishing options on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  if (module_exists('upload')) {
    $content['attachments'] = array(
      'title' => t('Node form file attachments'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('File attachments on the Node form.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  if (module_exists('taxonomy')) {
    $content['taxonomy'] = array(
      'title' => t('Node form categories'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Taxonomy categories for the node.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  if (module_exists('book')) {
    $content['book'] = array(
      'title' => t('Node form book options'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('Book options for the node.'),
      'required context' => new ctools_context_required(t('Form'), 'node_form'),
      'category' => array(t('Form'), -9),
    );
  }
  return $content;
}

function panels_content_node_form($subtype, $conf, $panel_args, &$context) {
  $block = new stdClass();
  $block->module = 'node-form';

  switch ($conf['id']) {
    case 'publishing':
      $block->subject = t('Publishing options');
      $block->delta = 'publishing-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && $context->form['options']['#type'] == 'fieldset') {
          // remove the fieldset
          unset($context->form['options']['#type']);
          $block->content = drupal_render($context->form['options']);
        }
      }
      else {
        $block->content = t('Publishing options.');
      }
      break;

    case 'comment':
      $block->subject = t('Comment options');
      $block->delta = 'comment-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['comment_settings']['#access'])) {
          // remove the fieldset
          unset($context->form['comment_settings']['#type']);
          $block->content = drupal_render($context->form['comment_settings']);
        }
      }
      else {
        $block->content = t('Comment options.');
      }
      break;

    case 'author':
      $block->subject = $block->content = t('Authoring information');
      $block->delta = 'author-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['author']['#access'])) {
          // remove the fieldset
          unset($context->form['author']['#type']);
          $context->form['author']['name']['#size'] /= 2;
          $context->form['author']['date']['#size'] /= 2;
          $block->content = drupal_render($context->form['author']);
        }
      }
      else {
        $block->content = t('Authoring information.');
      }
      break;

    case 'menu':
      $block->subject = t('Menu options');
      $block->delta = 'menu-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['menu']['#access'])) {
          // remove the fieldset
          unset($context->form['menu']['#type']);
          $context->form['menu']['link_title']['#size'] /= 2;
          $block->content = drupal_render($context->form['menu']);
        }
      }
      else {
        $block->content = t('Menu options.');
      }
      break;

    case 'url_path':
      $block->subject = t('URL path options');
      $block->delta = 'url-path-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['path']['#access'])) {
          // remove the fieldset
          unset($context->form['path']['#type']);
          $context->form['path']['path']['#size'] /= 2;
          $block->content = drupal_render($context->form['path']);
        }
      }
      else {
        $block->content = t('URL Path options.');
      }
      break;

    case 'attachments':
      $block->subject = t('Attach files');
      $block->delta = 'url-path-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['attachments']['#access'])) {
          // remove the fieldset
          unset($context->form['attachments']['#type']);
          $block->content = drupal_render($context->form['attachments']);
        }
      }
      else {
        $block->content = t('Attach files.');
      }
      break;

    case 'taxonomy':
      $block->subject = t('Categories');
      $block->delta = 'url-path-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['taxonomy'])) {
          // remove the fieldset
          unset($context->form['taxonomy']['#type']);
          $block->content = drupal_render($context->form['taxonomy']);
        }
      }
      else {
        $block->content = t('Categories.');
      }
      break;

    case 'book':
      $block->subject = t('Book options');
      $block->delta = 'book-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id)) {
          $block->content = '';
          if ($context->form['parent']['#type'] != 'value') {
            $block->content .= drupal_render($context->form['parent']);
          }
          if ($context->form['weight']['#type'] != 'value') {
            $block->content .= drupal_render($context->form['weight']);
          }
        }
      }
      else {
        $block->content = t('Book options.');
      }
      break;

    case 'buttons':
      $block->subject = t('Buttons');
      $block->delta = 'buttons';

      if (isset($context->form)) {
        $block->content = drupal_render($context->form['buttons']);
      }
      else {
        $block->content = t('Node form buttons.');
      }
      break;

    case 'input_format':
      $block->subject = t('Input format');
      $block->delta = 'format-options';

      if (isset($context->form)) {
        if (!empty($context->form->form_id) && !empty($context->form['body_filter']['format'])) {
          // remove the fieldset
          unset($context->form['body_filter']['format']['#type']);
          $block->content = drupal_render($context->form['body_filter']['format']);
        }
      }
      else {
        $block->content = t('Input format.');
      }
      break;
  }

  return $block;
}

/**
 * Returns an edit form for the custom type.
 */
function panels_admin_edit_node_form($id, $parents, $conf = array()) {
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $id,
  );

  return $form;
}

function panels_admin_title_node_form($subtype, $conf, $context) {
  $choices = panels_admin_content_types_node_form();
  return t('"@s" @type', array('@s' => $context->identifier, '@type' => drupal_strtolower($choices[$conf['id']]['title'])));
}

