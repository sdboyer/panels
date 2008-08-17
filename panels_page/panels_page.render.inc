<?php
// $Id$

/**
 * @file panels_page.render.inc
 *  Functions utilized during the panels_page render process. On any given page
 *  request, this file is lazy-loaded by panels_page itself only once it's been
 *  absolutely verified that we'll need it.
 */

function panels_page_prepare_panels_render(&$loader_data, $panel_page) {
  _panels_page_check_switchers($panel_page);

    // Store the instructions back in our loading handler.
  $loader_data['panel page']      = $panel_page;
  $loader_data['page callback']   = 'panels_page_render_page';
  // Pass by ref to reduce static variable size & ensure php4/5 consistency.
  $loader_data['page arguments']  = array(&$loader_data['panel page'], FALSE);
  $loader_data['title callback']  = 'panels_page_get_title';
  $loader_data['title arguments'] = array(&$loader_data['panel page']);
}

function _panels_page_check_switchers(&$panel_page) {
  $switchers = panels_get_switchers();
  if ($switchers) {
    $candidates = $list = array();
    $result = db_query("SELECT name FROM {panels_page} WHERE switcher_name = '%s'", $panel_page->name);
    // get a list of all pages that might switch for this one
    while ($candidate = db_fetch_object($result)) {
      $list[] = $candidate->name;
    }
    // Add default panels that apply to the list as well
    foreach (panels_page_default_panels() as $page) {
      if ($page->switcher_name == $panel_page->name) {
        $candidates[$page->name] = $page;
      }
    }

    if ($list) {
      $candidates += (count($list) > 1) ? panels_page_load_all($list) : array(panels_page_load($list[0]));
    }

    if ($candidates) {
      // separate them based on switcher type
      $pages = array();
      foreach ($candidates as $candidate) {
        $pages[$candidate->switcher_type][$candidate->name] = $candidate;
      }

      // go through switcher types and switch the first one that matches.
      foreach ($pages as $plugin => $candidates) {
        if ($page = panels_switcher_switch($plugin, 'panel_page', $panel_page->name, $candidates)) {
          $panel_page = $page;
          break;
        }
      }
    }
  }
}

function panels_page_render_page($panel_page, $admin) {
  $args = func_get_args();
  // remove the name.
  array_shift($args);
  // remove the admin flag.
  array_shift($args);

  // TODO separate this out prior to this point using the logic handler
  $output = '';
  if ($admin) {
    // Display an admin form to make it easy to set up arguments.
    panels_load_include('panels_page.admin', 'panels_page/');
    $output .= drupal_get_form('panels_page_admin_view', $panel_page, $args);
  }

  if (empty($panel_page->context)) { // FIXME die
    $panel_page->context = array();
  }

  if ($panel_page->arguments) {
    if (!panels_argument_load_contexts($panel_page->arguments, $panel_page->context, $args)) {
      if ($admin) {
        return $output . t('Page reports "Page not found" due to argument restrictions');
      }
      else {
        return drupal_not_found();
      }
    }
  }

  $panel_page->context = panels_context_load_contexts($panel_page, FALSE, $panel_page->context);
  $panel_page->keywords = panels_context_get_keywords($panel_page->context);

  // Figure out which display to use.
  $display_id = panels_argument_get_display($panel_page->arguments, $panel_page->context);

  $panel_page->display = $display = panels_page_fetch_display($panel_page, $display_id);

  // Figure out if these contexts include a form; will be NULL if there
  // isn't one, or the context if there is.
  $form = panels_context_get_form($panel_page->context);

  // This is the first point at which it is safe to add items to the display
  // as the argument handling, above, may choose which display is actually
  // used.
  $display->args      = $args;
  $display->context   = $panel_page->context;
  $display->keywords  = $panel_page->keywords;
  $display->css_id    = $panel_page->css_id;
  $display->owner     = $panel_page;
  // unique id of this display's owner.
  $display->owner->id = $panel_page->name;
  $display->type      = 'panel_page';

  // Set this as 'current' so that other stuff can utilize it.
  panels_page_get_current($panel_page);

  if ($form) {
    $form->form['#theme'] = 'panels_page_render_form';
    $form->form['#display'] = $display;
    $output .= drupal_render_form($form->form_id, $form->form);
  }
  else {
    $output .= panels_render_display($display);
  }

  if ($output == NULL) {
    return drupal_not_found();
  }

  if ($panel_page->css) {
    panels_load_include('panels_page.css_filter', 'panels_page/');
    $css = panels_page_filter_css(panels_page_disassemble_css($panel_page->css));
    // If the custom css didn't survive filtering, don't bother adding it.
    if (!empty($css)) {
      drupal_set_html_head("<style type=\"text/css\" media=\"all\">" . panels_page_compress_css($css) . "</style>\n");
    }
  }

  if ($panel_page->no_blocks) {
    print theme('page', $output, FALSE);
  }
  else {
    return $output;
  }
}

