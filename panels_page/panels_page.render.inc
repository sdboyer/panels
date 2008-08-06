<?php
// $Id$

/**
 * @file
 */


function panels_page_render_page($panel_page, $admin) {
  $args = func_get_args();
  // remove the name.
  array_shift($args);
  // remove the admin flag.
  array_shift($args);

  $output = '';
  if ($admin) {
    // Display an admin form to make it easy to set up arguments.
    panels_load_include('panels_page.admin', 'panels_page/');
    $output .= drupal_get_form('panels_page_admin_view', $panel_page, $args);
  }

  if (empty($panel_page->context)) {
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
  $display->args     = $args;
  $display->context  = $panel_page->context;
  $display->keywords = $panel_page->keywords;
  $display->css_id   = $panel_page->css_id;
  $display->owner    = $panel_page;
  // unique id of this display's owner.
  $display->owner->id = $panel_page->name;
  $display->type = 'panel_page';

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
    drupal_set_html_head("<style type=\"text/css\" media=\"all\">" . filter_xss_admin($panel_page->css) . "</style>\n");
  }
  if ($panel_page->no_blocks) {
    print theme('page', $output, FALSE);
  }
  else {
    return $output;
  }
}