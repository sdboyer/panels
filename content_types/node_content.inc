<?php
// $Id$


/**
 * Callback function to supply a list of content types.
 */
function panels_node_content_panels_content_types() {
  $items['node_content'] = array(
    'title' => t('Node content'),
    'weight' => -10,
    'single' => TRUE,
    'content_types' => 'panels_admin_content_types_node_content',
    'render callback' => 'panels_content_node_content',
    'add callback' => 'panels_admin_edit_node_content',
    'edit callback' => 'panels_admin_edit_node_content',
    'title callback' => 'panels_admin_title_node_content',
  );
  return $items;
}

/**
 * Return all content types available.
 */
function panels_admin_content_types_node_content() {
  return array(
    'content' => array(
      'title' => t('Node content'),
      'icon' => 'icon_node.png',
      'path' => panels_get_path('content_types/node'),
      'description' => t('The content of the referenced node.'),
      'required context' => new panels_required_context(t('Node'), 'node'),
      'category' => array(t('Node context'), -9),
    ),
  );
}

/**
 * Output function for the 'node' content type. Outputs a node
 * based on the module and delta supplied in the configuration.
 */
function panels_content_node_content($subtype, $conf, $panel_args, $context) {
  if (!empty($context) && empty($context->data)) {
    return;
  }

  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'node';
  $block->delta  = $node->nid;

  if (empty($node)) {
    $block->delta   = 'placeholder';
    $block->subject = t('Node title.');
    $block->content = t('Node content goes here.');
  }
  else {
    if (!empty($conf['identifier'])) {
      $node->panel_identifier = $conf['identifier'];
    }

    $block->subject = $node->title;

    unset($node->title);
    $block->content = panels_admin_node_content($node, $conf);
  }

  if (node_access('update', $node)) {
    $block->admin_links['update'] = array(
      'title' => t('Edit node'),
      'alt' => t("Edit this node"),
      'href' => "node/$node->nid/edit",
      'query' => drupal_get_destination(),
    );
  }

  if (!empty($conf['link']) && $node) {
    $block->title_link = "node/$node->nid";
  }

  return $block;
}

function panels_admin_node_content($node, $conf) {
  // Remove the delimiter (if any) that separates the teaser from the body.
  $node->body = str_replace('<!--break-->', '', $node->body);

  // The 'view' hook can be implemented to overwrite the default function
  // to display nodes.
  if (node_hook($node, 'view')) {
    $node = node_invoke($node, 'view', $conf['teaser'], $conf['page']);
  }
  else {
    $node = node_prepare($node, $conf['teaser']);
  }

  if (empty($conf['no_extras'])) {
    // Allow modules to make their own additions to the node.
    node_invoke_nodeapi($node, 'view', $conf['teaser'], $conf['page']);
  }

  if ($conf['links']) {
    $node->links = module_invoke_all('link', 'node', $node, $conf['teaser']);

    foreach (module_implements('link_alter') AS $module) {
      $function = $module .'_link_alter';
      $function($node, $node->links);
    }
  }

  // Set the proper node part, then unset unused $node part so that a bad
  // theme can not open a security hole.
  $content = drupal_render($node->content);
  if ($conf['teaser']) {
    $node->teaser = $content;
    unset($node->body);
  }
  else {
    $node->body = $content;
    unset($node->teaser);
  }

  // Allow modules to modify the fully-built node.
  node_invoke_nodeapi($node, 'alter', $conf['teaser'], $conf['page']);

  return theme('node', $node, $conf['teaser'], $conf['page']);
}

/**
 * Returns an edit form for the custom type.
 */
function panels_admin_edit_node_content($id, $parents, $conf = array()) {
  if ($conf == array()) {
    $conf = array(
      'links' => TRUE,
      'page' => TRUE,
      'no_extras' => TRUE,
    );
  }

  $form['aligner_start'] = array(
    '#value' => '<div class="option-text-aligner">',
  );
  $form['override_title'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['override_title'],
    '#title' => t('Override title'),
    '#id' => 'override-title-checkbox',
  );
  $form['override_title_text'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['override_title_text'],
    '#size' => 35,
    '#id' => 'override-title-textfield',
  );
  $form['aligner_stop'] = array(
    '#value' => '</div><div style="clear: both; padding: 0; margin: 0"></div>',
  );
  $form['link'] = array(
    '#title' => t('Link title to node'),
    '#type' => 'checkbox',
    '#default_value' => $conf['link'],
    '#description' => t('Check here to make the title link to the node.'),
  );
  $form['teaser'] = array(
    '#title' => t('Teaser'),
    '#type' => 'checkbox',
    '#default_value' => $conf['teaser'],
    '#description' => t('Check here to show only the node teaser.'),
  );
  $form['page'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['page'],
    '#title' => t('Node page'),
    '#description' => t('Check here if the node is being displayed on a page by itself.'),
  );
  $form['links'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['links'],
    '#title' => t('Display links'),
    '#description' => t('Check here to display the links with the post.'),
  );

  $form['no_extras'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['no_extras'],
    '#title' => t('No extras'),
    '#description' => t('Check here to disable additions that modules might make to the node, such as file attachments and CCK fields; this should just display the basic teaser or body.'),
  );

  $form['identifier'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['identifier'],
    '#title' => t('Identifier'),
    '#description' => t('Whatever is placed here will appear in $node->panel_identifier to make it easier to theme a node or part of a node as necessary.'),
  );

  return $form;
}

function panels_admin_title_node_content($conf, $context) {
  return t('"@s" content', array('@s' => $context->identifier));
}

