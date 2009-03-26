<?php
// $Id$

/**
 * @file
 * Contains the content type plugin for a mini panel. While this does not
 * need to be broken out into a .inc file, it's convenient that we do so
 * that we don't load code unneccessarily. Plus it demonstrates plugins
 * in modules other than Panels itself.
 * 
 */

/**
 * Specially named hook. for .inc file. This looks a little silly due to the
 * redundancy, but that's really just because the content type shares a
 * name with the module.
 */
function panels_mini_panels_mini_panels_content_types() {
  $items['panels_mini'] = array(
    'title' => t('Mini panels'),
    'content_types' => 'panels_mini_content_types',
    'render callback' => 'panels_mini_content',
    'add callback' => 'panels_mini_add_mini_panel',
    'edit callback' => 'panels_mini_edit_mini_panel',
    'title callback' => 'panels_mini_title_mini_panel',
  );
  return $items;
}

/**
 * Return each available mini panel available as a subtype.
 */
function panels_mini_content_types() {
  $types = array();
  ctools_include('context');
  foreach (panels_mini_load_all() as $mini) {
    if (!empty($mini->disabled)) {
      continue;
    }

    $types[$mini->name] = array(
      'title' => filter_xss_admin($mini->title),
      // For now mini panels will just use the contrib block icon.
      'icon' => 'icon_contrib_block.png',
      'path' => panels_get_path("plugins/content_types/block"),
      'description' => filter_xss_admin($mini->title),
      'category' => array(!empty($mini->category) ? filter_xss_admin($mini->category) : t('Mini panel'), -8),
    );
    if (!empty($mini->requiredcontexts)) {
      $types[$mini->name]['required context'] = array();
      foreach ($mini->requiredcontexts as $context) {
        $info = ctools_get_context($context['name']);
        // TODO: allow an optional setting
        $types[$mini->name]['required context'][] = new ctools_context_required($context['identifier'], $info['context name']);
      }
    }
  }
  return $types;
}


/**
 * Render a mini panel called from a panels display.
 */
function panels_mini_content($subtype, $conf, $panel_args, &$contexts) {
  $mini = panels_mini_load($conf['name']);
  if (!$mini) {
    return FALSE;
  }

  // Load up any contexts we might be using.
  $context = ctools_context_match_required_contexts($mini->requiredcontexts, $contexts);
  $mini->context = $mini->display->context = ctools_context_load_contexts($mini, FALSE, $context);

  if (empty($mini) || !empty($mini->disabled)) {
    return;
  }

  $mini->display->args = $panel_args;
  $mini->display->css_id = panels_mini_get_id($conf['name']);
  $mini->display->owner = $mini;
  // unique ID of this mini.
  $mini->display->owner->id = $mini->name;

  $block = new stdClass();
  $block->module  = 'panels_mini';
  $block->delta   = $conf['name'];
  $block->subject = filter_xss_admin($mini->title);
  $block->content = panels_render_display($mini->display);
  return $block;
}

/**
 * Form to add a mini panel to a panel.
 */
function panels_mini_add_mini_panel($id, $parents, $conf = array()) {
  $conf['name'] = $id;
  return panels_mini_edit_mini_panel($id, $parents, $conf);
}

/**
 * Returns an edit form for the mini panel.
 *
 * There isn't much here as most of this is set up at mini panel creation time.
 */
function panels_mini_edit_mini_panel($id, $parents, $conf) {
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $conf['name'],
  );

  return $form;
}

/**
 * Provide the administrative title of a mini panel.
 */
function panels_mini_title_mini_panel($subtype, $conf) {
  $mini = panels_mini_load($conf['name']);
  if (!$mini) {
    return t('Deleted/missing mini panel @name', array('@name' => $conf['name']));
  }

  $title = filter_xss_admin($mini->title);
  if (empty($title)) {
    $title = t('Untitled mini panel');
  }
  return $title;
}

