<?php
// $Id$

/**
 * @file panels_page.render.inc
 *  Functions utilized during the panels_page render process. On any given page
 *  request, this file is lazy-loaded by panels_page itself only once it's been
 *  absolutely verified that we'll need it.
 */

function panels_page_prepare_render(&$loader_data, &$panel_page) {
  _panels_page_prepare_render($panel_page);

  // Store the instructions back in our load handler array.
  $loader_data['page callback']   = 'panels_page_render_page_' . $panel_page->form ? 'form' : 'normal';
  $loader_data['title callback']  = 'panels_page_get_title';
  // Pass by ref to ENSURE php4/5 consistency and keep down static cache size.
  $loader_data['panel page']      =& $panel_page;
  $loader_data['page arguments']  = array(&$loader_data['panel page'], FALSE);
  $loader_data['title arguments'] = array(&$loader_data['panel page']);
}

function _panels_page_prepare_render(&$panel_page) {
  _panels_page_check_switchers($panel_page);
  $panel_page->context = panels_context_load_contexts($panel_page, FALSE, $panel_page->context);
  $panel_page->keywords = panels_context_get_keywords($panel_page->context);
  // Figure out which display to use.
  $display_id = panels_argument_get_display($panel_page->arguments, $panel_page->context);
  $panel_page->display = panels_page_fetch_display($panel_page, $display_id);
  $panel_page->form = panels_context_get_form($panel_page->context);
  panels_page_set_current($panel_page);
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

function _panels_page_prepare_display($panel_page, $args = array()) {
  $panel_page->display->args      = $args;
  $panel_page->display->context   = $panel_page->context;
  $panel_page->display->keywords  = $panel_page->keywords;
  $panel_page->display->css_id    = $panel_page->css_id;
  $panel_page->display->owner     = $panel_page; // TODO ref/value php4/5 compat problem?
  $panel_page->display->owner->id = $panel_page->name;
  $panel_page->display->type      = 'panel_page';
}

function panels_page_render_page_normal($panel_page, $args) {
  // TODO separate this out prior to this point using the logic handler
/*  $output = '';
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
*/
  _panels_page_prepare_display($panel_page, $args);
  $output = panels_render_display($display);
  // FIXME I believe this method no longer works.
  if ($panel_page->no_blocks) {
    print theme('page', $output, FALSE);
  }
  else {
    return $output;
  }
}

function panels_page_render_page_form($panel_page, $args) {
  $form = array_shift($args);
  _panels_page_render_display($panel_page, $args);
  $form = panels_context_get_form($panel_page->context);
  $form->form['#theme'] = 'panels_page_render_form';
  $form->form['#display'] = $panel_page->display;
  $output = drupal_render_form($form->form_id, $form->form);
}

function panels_page_custom_css($panel_page) {
  if ($panel_page->css) {
    panels_load_include('panels_page.css_filter', 'panels_page/');
    $css = panels_page_filter_css(panels_page_disassemble_css($panel_page->css));
    // If the custom css didn't survive filtering, don't bother adding it.
    if (!empty($css)) {
      drupal_set_html_head("<style type=\"text/css\" media=\"all\">" . panels_page_compress_css($css) . "</style>\n");
    }
  }
}


