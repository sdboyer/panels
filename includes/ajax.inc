<?php
// $Id$

/**
 * @file
 *
 * Miscellaneous AJAX helper routines
 */

/**
 * Includes required JavaScript libraries.
 */
function panels_modal_js_includes($modals = array()) {
  $ajax = array('panels' => array(
    'closeText' => t('Close Window'),
    'closeImage' => theme('image', panels_get_path('images/icon-delete.png'), t('Close window'), t('Close window')),
    'throbber' => theme('image', panels_get_path('images/throbber.gif'), t('Loading...'), t('Loading')),
  ));

  if ($modals) {
    $ajax['panels']['modals'] = $modals;
  }

  drupal_add_js(panels_get_path('js/panels-base.js'));
  drupal_add_js($ajax, 'setting');
  drupal_add_js(panels_get_path('js/lib/dimensions.js'));
  drupal_add_js(panels_get_path('js/lib/mc.js'));
  drupal_add_js('misc/jquery.form.js');
  drupal_add_js('misc/autocomplete.js');
  drupal_add_js('misc/collapse.js');
  drupal_add_js(panels_get_path('js/modal_forms.js'));
  drupal_add_js(panels_get_path('js/checkboxes.js'));
}

/**
 * Based upon Views' views_ajax_form_wrapper, respecialed to Panels' needs.
 */
function panels_ajax_form_wrapper($form_id, &$form_state) {
  // This won't override settings already in.
  $form_state += array(
    're_render' => FALSE,
    'no_redirect' => !empty($form_state['ajax']),
  );

  $output = drupal_build_form($form_id, $form_state);
  if (!empty($form_state['ajax']) && empty($form_state['executed'])) {
    // If the form didn't execute and we're using ajax, build up a
    // json command object to render.
    $object = new stdClass();
    $object->output = '';
    $object->type = 'display';
    if ($messages = theme('status_messages')) {
      $object->output = '<div class="views-messages">' . $messages . '</div>';
    }
    $object->output .= $output;

    $object->title = empty($form_state['title']) ? '' : $form_state['title'];
    if (!empty($form_state['help_topic'])) {
      $module = !empty($form_state['help_module']) ? $form_state['help_module'] : 'panels';
      $object->title = theme('advanced_help_topic', $module, $form_state['help_topic']) . $object->title;
    }
    $object->url = empty($form_state['url']) ? url($_GET['q'], array('absolute' => TRUE)) : $form_state['url'];
    $object->js = empty($form_state['js settings']) ? NULL : $form_state['js settings'];

    $output = $object;
  }

  // These forms have the title built in, so set the title here:
  if (empty($form_state['ajax']) && !empty($form_state['title'])) {
    drupal_set_title($form_state['title']);
  }

  return $output;
}

/**
 * @todo -- move this somewhere more useful. ajax.inc
 */
function panels_ajax_image_button($image, $dest, $alt, $class = '') {
  return panels_ajax_text_button(theme('image', panels_get_path("images/$image")), $dest, $alt, $class);
}

/**
 * Note: 'html' => true so be sure any text is vetted! Chances are these kinds of buttons will
 * not use user input so this is a very minor concern.
 * ajax.inc
 */
function panels_ajax_text_button($text, $dest, $alt, $class = '') {
  return l($text, $dest, array('html' => TRUE, 'attributes' => array('class' => "panels-ajax-link $class", 'title' => $alt, 'alt' => $alt)));
}


/**
 * Simple render function to make sure output is what we want.
 * @ingroup panels_ajax
 */
function panels_ajax_render($output = NULL, $title = NULL, $url = NULL) {
  if (!is_object($output)) {
    $temp = new stdClass();
    $temp->output = $output;
    switch ($output) {
      case 'dismiss':
        $temp->type = $output;
        break;

      default:
        $temp->type = 'display';
    }
    $temp->title = $title;
    $temp->url   = $url;
    $output      = $temp;
  }
  if (!$output->output || !$output->type) {
    $output->output = t('The input was invalid');
    $output->type   = 'display';
    $output->title  = t('Error');
  }
  drupal_json($output);
  exit;
}
