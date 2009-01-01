<?php
// $Id$

/*
 * @file display-edit.inc
 * Core Panels API include file containing various display-editing functions.
 * This includes all the basic editing forms (content, layout, layout settings)
 * as well as the ajax modal forms associated with them.
 */

/**
 * Handle calling and processing of the form for editing display content.
 *
 * Helper function for panels_edit().
 *
 * @see panels_edit() for details on the various behaviors of this function.
 */
function _panels_edit($display, $destination, $content_types) {
  $did = $display->did;
  if (!$did) {
    $display->did = $did = 'new';
  }

  // Load the display being edited from cache, if possible.
  if (!empty($_POST) && is_object($cache = panels_cache_get('display', $did))) {
    $display = $cache->display;
  }
  else {
    panels_cache_clear('display', $did);
    $cache = new stdClass();
    $cache->display = $display;
    $cache->content_types = $content_types;
    panels_cache_set('display', $did, $cache);
  }

  $form_state = array(
    'display' => &$display,
    're_render' => FALSE,
    'no_redirect' => TRUE,
  );

  $output = drupal_build_form('panels_edit_display_form', $form_state);
  // no output == submit
  if (!$output) {
    if (!empty($form_state['clicked_button']['#save-display'])) {
      drupal_set_message(t('Panel content has been updated.'));
      panels_save_display($display);
    }
    else {
      drupal_set_message(t('Your changes have been discarded.'));
    }

    panels_cache_clear('display', $display->did);
    if ($destination) {
      return drupal_goto($destination);
    }
    return $form_state['display'];
  }

  return $output;
}

/**
 * Form definition for the panels display editor
 *
 * No validation function is necessary, as all 'validation' is handled
 * either in the lead-up to form rendering (through the selection of
 * specified content types) or by the validation functions specific to
 * the ajax modals & content types.
 *
 * @ingroup forms
 * @see panels_edit_display_submit()
 */
function panels_edit_display_form(&$form_state) {
  $display = &$form_state['display'];

  // Annoyingly, theme doesn't have access to form_state so we have to do this.
  $form['#display'] = $display;
/*
  $explanation_text = '<p>';
  $explanation_text .= t('Grab the title bar of any pane to drag-and-drop it into another panel. Click the add pane button (!addicon) in any panel to add more content. Click the configure (!configicon) button on any pane to re-configure that pane. Click the cache (!cacheicon) button to configure caching for that pane specifically. Click the show/hide (!showicon/!hideicon) toggle button to show or hide that pane. Panes hidden in this way will be hidden from <em>everyone</em> until the hidden status is toggled off.',
    array(
      '!addicon' => '<span class="inline-icon-help">' . theme('image', panels_get_path('images/icon-addcontent.png'), t('Add content to this panel'), t('Add content to this panel')) . '</span>',
      '!configicon' => '<span class="inline-icon-help">' . theme('image', panels_get_path('images/icon-configure.png'), t('Configure this pane'), t('Configure this pane')) . '</span>',
      '!cacheicon' => '<span class="inline-icon-help">' . theme('image', panels_get_path('images/icon-cache.png'), t('Control caching'), t('Control caching')) . '</span>',
      '!showicon' => '<span class="inline-icon-help">' . theme('image', panels_get_path('images/icon-showpane.png'), t('Show this pane'), t('Show this pane')) . '</span>',
      '!hideicon' => '<span class="inline-icon-help">' . theme('image', panels_get_path('images/icon-hidepane.png'), t('Hide this pane'), t('Hide this pane')) . '</span>',
    )
  );
  $explanation_text .= '</p>';

  $form['explanation'] = array(
    '#value' => $explanation_text,
  );
*/
  $layout = panels_get_layout($display->layout);
  $layout_panels = panels_get_panels($layout, $display);

  $form['panel'] = array('#tree' => TRUE);
  $form['panel']['pane'] = array('#tree' => TRUE);

  foreach ($layout_panels as $panel_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($display->panels[$panel_id])) {
      $display->panels[$panel_id] = array();
    }

    $form['panel']['pane'][$panel_id] = array(
      // Use 'hidden' instead of 'value' so the js can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', (array) $display->panels[$panel_id]),
    );
  }

  if (empty($form_state['no buttons'])) {
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#id' => 'panels-dnd-save',
      '#submit' => array('panels_edit_display_form_submit'),
      '#save-display' => TRUE,
    );
    $form['buttons']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
    );
  }

/* -- disabled, these buttons aren't useful right now
  $form['hide'] = array(
    '#prefix' => '<span id="panels-js-only">',
    '#suffix' => '</span>',
  );

  $form['hide']['hide-all'] = array(
    '#type' => 'submit',
    '#value' => t('Hide all'),
    '#id' => 'panels-hide-all',
  );

  $form['hide']['show-all'] = array(
    '#type' => 'submit',
    '#value' => t('Show all'),
    '#id' => 'panels-show-all',
  );
*/

  if (user_access('use panels caching features')) {
    $form['hide']['cache-settings-url'] = array(
      '#type' => 'hidden',
      '#value' => url('panels/ajax/cache-method/' . $display->did . '/display',  array('absolute' => true)),
      '#attributes' => array('class' => 'panels-cache-settings-url'),
    );

    $form['hide']['cache-settings'] = array(
      '#type' => 'submit',
      '#value' => t('Cache settings'),
      '#id' => 'panels-cache-settings',
      '#attributes' => array('class' => 'panels-ajax-link'),
    );
  }

  panels_modal_js_includes();
  drupal_add_js(panels_get_path('js/display_editor.js'));
  drupal_add_css(panels_get_path('css/panels_dnd.css'));
  drupal_add_css(panels_get_path('css/panels_admin.css'));

  return $form;
}

/**
 * Theme the edit display form.
 *
 * This has to do a fair bit of work since it's actually rendering the layout
 * as well as ensuring that all of the gadgets go in the right place.
 */
function theme_panels_edit_display_form($form) {
  $output = '';
  $content = array();

  $display = $form['#display'];

  $layout = panels_get_layout($display->layout);
  $layout_panels = panels_get_panels($layout, $display);
  $save_buttons = drupal_render($form['buttons']);

  foreach ($layout_panels as $panel_id => $title) {
    if (empty($content[$panel_id])) {
      $content[$panel_id] = '';
    }

    foreach ((array) $display->panels[$panel_id] as $pid) {
      $pane = $display->content[$pid];
      $left_buttons = NULL;
      $content[$pane->panel] .= panels_show_pane($display, $pane);
    }

    $panel_buttons = panels_ajax_image_button('icon-addcontent.png', "panels/ajax/add-pane/$display->did/$panel_id", t('Add a pane to "@panel"', array('@panel' => $title)), "pane-add-link");
    $content[$panel_id] = theme('panels_panel_dnd', $content[$panel_id], $panel_id, $title, $panel_buttons);
   }

  $output .= drupal_render($form);
  panels_load_include('display-render');
  $output .= theme('panels_dnd', panels_render_layout($layout, $content, '', $display->layout_settings));
  $output .= $save_buttons;
   return $output;
}

/**
 * Handle form submission of the display content editor.
 *
 * This reads the location of the various panes from the form, which will
 * have been modified from the ajax, rearranges them and then saves
 * the display.
 */
function panels_edit_display_form_submit($form, &$form_state) {
  $display = &$form_state['display'];

  $old_content = $display->content;
  $display->content = array();

  foreach ($form_state['values']['panel']['pane'] as $panel_id => $panes) {
    $display->panels[$panel_id] = array();
    if ($panes) {
      $pids = explode(',', $panes);
      // need to filter the array, b/c passing it in a hidden field can generate trash
      foreach (array_filter($pids) as $pid) {
        if ($old_content[$pid]) {
          $display->panels[$panel_id][] = $pid;
          $old_content[$pid]->panel = $panel_id;
          $display->content[$pid] = $old_content[$pid];
        }
      }
    }
  }
}

/**
 * Render a single pane in the edit environment.
 *
 * @param $pane
 *   The pane to render.
 * @param $left_buttons
 *   Buttons that go on the left side of the pane.
 * @param $buttons
 *   Buttons that go on the right side of the pane.
 * @param $skin
 *   If true, provide the outside div. Used to provide an easy way to get
 *   just the innards for ajax replacement
 */
// TODO check and see if $skin is ever FALSE; pane show/hide setting is dependent on it being TRUE. can't imagine it could be...
function panels_show_pane($display, $pane, $skin = TRUE) {
  $content_type = panels_get_content_type($pane->type);

  // This is just used for the title bar of the pane, not the content itself.
  // If we know the content type, use the appropriate title for that type,
  // otherwise, set the title using the content itself.
  $title = !empty($content_type) ? panels_get_pane_title($pane, $display->context) : $block->title;

  $left_buttons = '';
  $buttons = '';

  // Render administrative buttons for the pane.

  // On the left we have the 'caching' link.
  if (panels_get_caches() && user_access('use panels caching features')) {
    $buttons .= panels_ajax_image_button('icon-cache.png', "panels/ajax/cache-method/$display->did/$pane->pid", t('Set caching options for "@pane"', array('@pane' => $title)), "pane-cache-link");
  }

  // Next to it the show or hide pane link
  if (!empty($pane->shown)) {
    $buttons .= panels_ajax_image_button('icon-hidepane.png', "panels/ajax/hide/$display->did/$pane->pid", t('Hide "@pane"', array('@pane' => $title)), "pane-toggle-shown-link panels-no-modal");
  }
  else {
    $buttons .= panels_ajax_image_button('icon-showpane.png', "panels/ajax/show/$display->did/$pane->pid", t('Show "@pane"', array('@pane' => $title)), "pane-toggle-shown-link panels-no-modal");
  }

  // And the configure settings link
  $buttons .= panels_ajax_image_button('icon-configure.png', "panels/ajax/configure/$display->did/$pane->pid", t('Configure settings for "@pane"', array('@pane' => $title)), "pane-configure-link");

  // The delete button doesn't use panels_ajax_image_button because it doesn't
  // actually perform an ajax call.
  $alt = t('Delete pane "@pane"', array('@pane' => $title));
  $buttons .= l(theme('image', panels_get_path("images/icon-delete.png")), '', array('html' => TRUE, 'attributes' => array('class' => 'pane-delete', 'id' => "pane-delete-panel-pane-$pane->pid", 'title' => $alt, 'alt' => $alt)));

  $block = new stdClass();
  if (empty($content_type)) {
    $block->title = '<em>' . t('Missing content type') . '</em>';
    $block->content = t('This pane\'s content type is either missing or has been deleted. This pane will not render.');
  }
  elseif (isset($content_type['editor render callback']) && function_exists($content_type['editor render callback'])) {
    $block = $content_type['editor render callback']($display, $pane);
  }
  else {
    $block = _panels_render_preview_pane_disabled($pane, $display->context);
  }

  $output = theme('panels_pane_dnd', $block, $pane->pid, $title, $left_buttons, $buttons);
  if ($skin) {
    $class = 'panel-pane' . ($pane->shown ? '' : ' hidden-pane');
    $output = '<div class="' . $class . '" id="panel-pane-' . $pane->pid . '">' . $output . '</div>';
  }
  return $output;
}

/**
 * Provide filler content for dynamic pane previews in the editor, as they're just a
 * bad idea to have anyway, and can also cause infinite recursion loops that render the
 * editor inaccessible in some cases.
 *
 */
function _panels_render_preview_pane_disabled($pane, $context) {
  $block = new stdClass();
  $block->title   = panels_get_pane_title($pane, $context);
  $block->content = '<em>' . t("Dynamic content previews have been disabled to improve performance and stability for this editing screen.") . '</em>';
  return $block;
}

/**
 * @defgroup panels_ajax Functions for panels ajax features
 * @{
 */

/**
 * Entry point for AJAX: 'Add Content' modal form, from which the user selects the
 * type of pane to add.
 *
 * @param int $did
 *  The display id of the $display object currently being edited.
 * @param string $panel_id
 *  A string with the name of the panel region to which the selected
 *  pane type will be added.
 */
function panels_ajax_add_pane_choose($did = NULL, $panel_id = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  $output = new stdClass();
  if ((is_numeric($did) || $did == 'new') && $cache = panels_cache_get('display', $did)) {
    $display = $cache->display;
    $layout = panels_get_layout($display->layout);
    $layout_panels = panels_get_panels($layout, $display);

    if ($layout && array_key_exists($panel_id, $layout_panels)) {
      $output->output = panels_add_content($cache, $panel_id);
      $output->type   = 'display';
      $output->title  = t('Add content to !s', array('!s' => $layout_panels[$panel_id]));
    }
  }
  panels_ajax_render($output);
}

function panels_add_content($cache, $panel_id) {
  $did = $cache->display->did;

  if (!isset($cache->content_types)) {
    $cache->content_types = panels_get_available_content_types();
  }

  $weights    = array(t('Contributed blocks') => 0);
  $categories = array();
  $titles     = array();

  foreach ($cache->content_types as $type_name => $subtypes) {
    if (is_array($subtypes)) {
      $content_type = panels_get_content_type($type_name);
      foreach ($subtypes as $subtype_name => $subtype_info) {
        $title = filter_xss_admin($subtype_info['title']);
        $description = isset($subtype_info['description']) ? $subtype_info['description'] : $title;

        $icon = '';

        if (isset($subtype_info['icon'])) {
          $icon = $subtype_info['icon'];
          if (isset($subtype_info['path']) && file_exists($subtype_info['path'] . '/' . $icon)) {
            $path = $subtype_info['path'];
          }
          else if (isset($content_type['path']) && file_exists($content_type['path'] . '/' . $icon)) {
            $path = $content_type['path'];
          }
          else if (file_exists(panels_get_path("content_types/$type_name") . '/' . $icon)) {
            $path = panels_get_path("content_types/$type_name");
          }
        }

        if (empty($icon)) {
          $icon = 'no-icon.png';
          $path = panels_get_path('images');
        }

        if (isset($subtype_info['category'])) {
          if (is_array($subtype_info['category'])) {
            list($category, $weight) = $subtype_info['category'];
            $weights[$category] = $weight;
          }
          else {
            $category = $subtype_info['category'];
            if (!isset($weights['category'])) {
              $weights[$category] = 0;
            }
          }
        }
        else {
          $category = t('Contrib blocks');
        }

        $output = '<div class="content-type-button clear-block">';
        $link_image = theme('image', $path . '/' . $icon, $description, $description);
        $url = "panels/ajax/add-pane-config/$did/$panel_id/$type_name/$subtype_name";
        $output .= panels_ajax_text_button($link_image, $url, $description, 'panels-modal-add-config');
        $output .= '<div>' . panels_ajax_text_button($title, $url, $description, 'panels-modal-add-config') . '</div>';
        $output .= '</div>';
        if (!isset($categories[$category])) {
          $categories[$category] = array();
          $titles[$category] = array();
        }

        $categories[$category][] = $output;
        $titles[$category][] = $title;
      }
    }
  }
  if (!$categories) {
    $output = t('There are no content types you may add to this display.');
  }
  else {
    asort($weights);
    $output = '';

    $columns = 3;
    foreach (range(1, $columns) as $column) {
      $col[$column] = '';
      $size[$column] = 0;
    }

    foreach ($weights as $category => $weight) {
      $which = 1; // default;
      $count = count($titles[$category]) + 3; // add 3 to account for title.
      // Determine which column to use by seeing which column has the most
      // free space. This algorithm favors left.
      foreach (range($columns, 2) as $column) {
        if ($size[$column - 1] - $size[$column] > $count / 2 || ($size[$column - 1] > 0 && $size[$column] == 0)) {
          $which = $column;
          break;
        }
      }

      $col[$which] .= '<div class="panels-section">';
      $col[$which] .= '<h3 class="panels-section-title">' . $category . '</h3>';
      $col[$which] .= '<div class="panels-section-decorator"></div>';
      $col[$which] .= '<div class="panels-section-content">';
      if (is_array($titles[$category])) {
        natcasesort($titles[$category]);
        foreach ($titles[$category] as $id => $title) {
          $col[$which] .= $categories[$category][$id];
        }
      }
      $col[$which] .= '</div>';
      $col[$which] .= '</div>';
      $size[$which] += $count;
    }

    foreach ($col as $column) {
      $output .= '<div class="panels-section-column">' . $column . '</div>';
    }
  }
  return $output;
}

/**
 * AJAX entry point for to configure a pane that has just been added.
 */
function panels_ajax_add_pane_config($did = NULL, $panel_id = NULL, $type = NULL, $subtype = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    panels_ajax_render(t('Invalid display id.'));
  }

  $pane = panels_new_pane($type, $subtype);

  $subtypes = panels_ct_get_types($pane->type);
  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure !subtype_title', array('!subtype_title' => $subtypes[$pane->subtype]['title'])),
    'type' => panels_get_content_type($pane->type),
    'subtype' => $subtypes[$pane->subtype],
    'op' => 'add',
  );

  $output = panels_ajax_form_wrapper('panels_content_config_form', $form_state);
  if (empty($output)) {
    // Get a real pid for this pane.
    $pane->pid = "new-" . $cache->display->next_new_pid();
    // Put the pane into the display where it belongs
    $pane->panel = $panel_id;
    $cache->display->content[$pane->pid] = $pane;
    $cache->display->panels[$panel_id][] = $pane->pid;

    panels_cache_set('display', $did, $cache);

    $output = new stdClass();
    $output->type = 'add';
    $output->region = "#panel-pane-$panel_id";
    $output->id = "#panel-pane-$pane->pid";

    $output->output = panels_show_pane($cache->display, $pane, TRUE);
  }

  panels_ajax_render($output);
}

/**
 * AJAX entry point for to configure a pane that has just been added.
 */
function panels_ajax_configure_pane($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    panels_ajax_render(t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    panels_ajax_render(t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];
  $subtypes = panels_ct_get_types($pane->type);

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure !subtype_title', array('!subtype_title' => $subtypes[$pane->subtype]['title'])),
    'type' => panels_get_content_type($pane->type),
    'subtype' => $subtypes[$pane->subtype],
    'op' => 'edit',
  );

  $output = panels_ajax_form_wrapper('panels_content_config_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);

    $output = new stdClass();
    $output->type = 'replace';
    $output->id = "#panel-pane-$pane->pid";
    $output->output = panels_show_pane($cache->display, $pane, TRUE);
  }

  panels_ajax_render($output);
}


/**
 * Master FAPI definition for all pane add/edit configuration forms.
 *
 * @param object $cache
 *  The $cache object for the panels $display currently being edited.
 * @param object $pane
 *  The $pane object currently being added/edited.
 * @param bool $add
 *  A boolean indicating whether we are adding a new pane ($add === TRUE)
 *  operation in this operation, or editing an existing pane ($add === FALSE).
 *
 * @return array $form
 *  A structured FAPI form definition, having been passed through all the appropriate
 *  content-type specific callbacks.
 */
function panels_content_config_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];
  $type = $form_state['type'];
  $subtype = $form_state['subtype'];
  $op = $form_state['op'];

  $form['configuration'] = panels_ct_conf_form($type, $subtype, $display->context, $pane->configuration);
  if (is_array($form_additions = panels_ct_pane_config_form($pane, $display->context, array('configuration'), $op))) {
    $form['configuration'] += $form_additions;
  }
  $form['configuration']['#tree'] = TRUE;

  $ignore_roles = empty($type['role-based access']);

  if ($visibility_function = panels_plugin_get_function('content_types', $type, 'visibility control')) {
    $ignore_roles = TRUE;
    if (isset($type['roles and visibility']) && $type['roles and visibility'] === TRUE) {
      $ignore_roles = FALSE;
    }
  }

  if (!$ignore_roles) {
    if (user_access('administer pane access')) {
      $form['access'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Access'),
        '#default_value' => $pane->access,
        '#options' => user_roles(TRUE),
        '#description' => t('Only the checked roles will be able to see this pane; if no roles are checked, access will not be restricted.'),
      );
    }
    else {
      $form['access'] = array(
        '#type' => 'value',
        '#value' => isset($pane->access) ? $pane->access : array(),
      );
    }
  }

  if (isset($visibility_function)) {
    $form['visibility'] = $visibility_function($display->context, $pane->subtype, $pane->configuration, $form_state['op'] == 'add');
  }

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => $op == 'add' ? t('Add pane') : t('Save'),
  );

  // Allows content types that define this callback to have full control over the pane config form.
/* @todo -- make this work
  if (isset($cc['form_controller'])) {
    call_user_func_array($cc['form_controller'], array(&$form, &$cache->content_config[$pane->pid], &$cache->display, $add));
  }
*/
  return $form;
}

/**
 * FAPI validator for panels_content_config_form().
 *
 * Call any validation functions defined by the content type.
 */
function panels_content_config_form_validate($form, &$form_state) {
  panels_ct_pane_validate_form($form_state['pane']->type, $form['configuration'], $form_state['values']['configuration'], $form_state['op']);
}

/**
 * FAPI submission function for the edit content form.
 *
 * All this does is set up $pane properly. The caller is responsible for
 * actually storing this somewhere.
 */
function panels_content_config_form_submit($form, &$form_state) {
  $pane = &$form_state['pane'];
  $display = $form_state['display'];

  panels_ct_pane_submit_form($pane->type, $form_state['values']['configuration'], $form_state['op']);

  if (isset($form_state['values']['visibility'])) {
    if ($visibility_submit = panels_plugin_get_function('content_types', $form_state['type'], 'visibility submit')) {
      // Use call_user_func_array() in order to ensure that all these values
      // can only be passed by value.
      $pane->visibility = call_user_func_array($visibility_submit,
        array($form_state['values']['visibility'], $form_state['op'] == 'add', $pane, $display));
    }
    else {
      // If no visibility submit callback is defined, fall back to the
      // default storage behavior. Should be adequate for the vast majority
      // of use cases, so most client modules won't need to define callbacks.
      $$pane->visibility = is_array($form_state['values']['visibility']) ? array_keys(array_filter($form_state['values']['visibility'])) : $form_state['values']['visibility'];
    }
  }

  if (isset($form_state['values']['access'])) {
    $pane->access = array_keys(array_filter($form_state['values']['access']));
  }
  else {
    $pane->access = array();
  }

  $pane->configuration = $form_state['values']['configuration'];
}

/**
 * Entry point for AJAX: toggle pane show/hide status.
 *
 * @param int $did
 *  The display id for the display object currently being edited. Errors out silently if absent.
 * @param int $pid
 *  The pane id for the pane object whose show/hide state we're toggling.
 * @param string $op
 *  The operation - showing or hiding - that this should perform. This could be calculated from
 *  cached values, but given that this is a toggle button and people may click it too fast,
 *  it's better to leave the decision on which $op to use up to the js than to calculate it here.
 */
function panels_ajax_toggle_shown($op, $did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    panels_ajax_render(t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    panels_ajax_render(t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];

  $pane->shown = ($op == 'show');
  panels_cache_set('display', $did, $cache);

  $output = new stdClass();
  $output->type = 'replace';
  $output->id = "#panel-pane-$pane->pid";
  $output->output = panels_show_pane($cache->display, $pane, TRUE);

  panels_ajax_render($output);
}

/**
 * Entry point for AJAX modal: configure pane cache method
 */
function panels_ajax_cache_method($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    panels_ajax_render(t('Invalid display id.'));
  }
  // This lets us choose whether we're doing the display's cache or
  // a pane's.
  if ($pid == 'display') {
    $conf = &$cache->display->cache;
    $title = t('Cache method for this display');
  }
  else if (!empty($cache->display->content[$pid])) {
    $pane = $cache->display->content[$pid];
    $subtypes = panels_ct_get_types($pane->type);
    $conf = &$cache->display->content[$pid]->configuration;
    $title = t('Cache method for !subtype_title', array('!subtype_title' => $subtypes[$pane->subtype]['title']));
  }
  else {
    panels_ajax_render(t('Invalid pane id.'));
  }

  $form_state = array(
    'display' => &$cache->display,
    'conf' => &$conf,
    'title' => $title,
    'ajax' => TRUE,
  );

  $output = panels_ajax_form_wrapper('panels_edit_cache_method_form', $form_state);
  if (empty($output)) {
    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    $function = panels_plugin_get_function('cache', $form_state['method'], 'settings form');
    if (!$function) {
      // This cache method has no settings form, so just save and dismiss.
      $conf['method'] = $form_state['method'];
      $conf['settings'] = array();
      panels_cache_set('display', $did, $cache);
      $output = new stdClass();
      $output->type = 'dismiss';
      $output->output = 'dummy';

      if ($pid != 'display') {
        $output->id = "panel-pane-$pid";
      }
    }
    else {
      $cache->method = $form_state['method'];
      panels_cache_set('display', $did, $cache);
      // send them to next form.
      return panels_ajax_cache_settings($cache, $pid);
    }
  }

  panels_ajax_render($output);
}

/**
 * Choose cache method form
 */
function panels_edit_cache_method_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];

  // Set to 0 to ensure we get a selected radio.
  if (!isset($conf['method'])) {
    $conf['method'] = 0;
  }

  $caches = panels_get_caches();
  if (empty($caches)) {
    $form['markup'] = array('#value' => t('No caching options are available at this time. Please enable a panels caching module in order to use caching options.'));
    return $form;
  }

  $options[0] = t('No caching');
  foreach ($caches as $cache => $info) {
    $options[$cache] = check_plain($info['title']);
  }

  $form['method'] = array(
    '#prefix' => '<div class="no-float">',
    '#suffix' => '</div>',
    '#type' => 'radios',
    '#title' => t('Method'),
    '#options' => $options,
    '#default_value' => $conf['method'],
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}

/**
 * Submit callback for panels_edit_cache_method_form.
 *
 * All this needs to do is return the method.
 */
function panels_edit_cache_method_form_submit($form, &$form_state) {
  $form_state['method'] = $form_state['values']['method'];
}

/**
 * Handle the cache settings form
 */
function panels_ajax_cache_settings($did, $pid) {
  panels_load_include('plugins');
  panels_load_include('ajax');
  ctools_include('context');

  if (is_object($did)) {
    // This is safe: Objects cannot be passed via URL.
    $cache = $did;
    $did = $cache->display->did;
  }
  else if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    panels_ajax_render(t('Invalid display id.'));
  }

  // This lets us choose whether we're doing the display's cache or
  // a pane's.
  if ($pid == 'display') {
    $conf = &$cache->display->cache;
    $title = t('Cache settings for this display');
  }
  else if (!empty($cache->display->content[$pid])) {
    $pane = $cache->display->content[$pid];
    $subtypes = panels_ct_get_types($pane->type);

    $conf = &$cache->display->content[$pid]->configuration;
    $title = t('Cache settings for !subtype_title', array('!subtype_title' => $subtypes[$pane->subtype]['title']));
  }
  else {
    panels_ajax_render(t('Invalid pane id.'));
  }

  $conf['method'] = $cache->method;

  $form_state = array(
    'display' => &$cache->display,
    'pid' => $pid,
    'conf' => &$conf,
    'ajax' => TRUE,
    'title' => $title,
    'url' => url("panels/ajax/cache-settings/$did/$pid", array('absolute' => TRUE)),
  );

  $output = panels_ajax_form_wrapper('panels_edit_cache_settings_form', $form_state);
  if (empty($output)) {
    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    if ($pid == 'display') {
      $cache->display->cache = $conf;
    }
    else {
      $cache->display->content[$pid]->configuration = $conf;
    }

    panels_cache_set('display', $did, $cache);

    $output = new stdClass();
    $output->type = 'dismiss';
    $output->output = 'dummy';
    if ($pid != 'display') {
      $output->id = "panel-pane-$pid";
    }
  }
  panels_ajax_render($output);
}

/**
 * Cache settings form
 */
function panels_edit_cache_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];
  $pid = $form_state['pid'];
  $info = panels_get_cache($conf['method']);

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($info['description']),
  );

  $function = panels_plugin_get_function('cache', $conf['method'], 'settings form');

  $form['settings'] = $function($conf['settings'], $display, $pid);
  $form['settings']['#tree'] = TRUE;

  $form['method'] = array(
    '#type' => 'hidden',
    '#value' => $method,
  );

  $form['display'] = array(
    '#type' => 'value',
    '#value' => $display,
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate cache settings.
 */
function panels_edit_cache_settings_form_validate($form, &$form_state) {
  if ($function = panels_plugin_get_function('cache', $form_state['conf']['method'], 'settings form validate')) {
    $function($form, $form_state['values']['settings']);
  }
}

/**
 * Allows panel styles to validate their style settings.
 */
function panels_edit_cache_settings_form_submit($form, &$form_state) {
  if ($function = panels_plugin_get_function('cache', $form_state['conf']['method'], 'settings form submit')) {
    $function($form_state['values']['settings']);
  }

  $form_state['conf']['settings'] = $form_state['values']['settings'];
}

/**
 * }@ End of 'defgroup panels_ajax'
 */


// ---------------------------------------------------------------------------
// Panels theming functions

// @DND
function theme_panels_dnd($content) {
  $output = '<div class="panels-dnd" id="panels-dnd-main">' . $content . '</div>';
  return $output;
}

// @DND
function theme_panels_panel_dnd($content, $area, $label, $footer) {
  return "<div class='panels-display' id='panel-pane-$area'>$footer<h2 class='label'>$label</h2>$content</div>";
}

// @DND
function theme_panels_pane_dnd($block, $id, $label, $left_buttons = NULL, $buttons = NULL) {
  $output = '';
  if (!$block->title) {
    $block->title = t('No title');
  }
  static $count = 0;
  $output .= '<div class="grabber">';
  if ($buttons) {
    $output .= '<span class="buttons">' . $buttons . '</span>';
  }
  if ($left_buttons) {
    $output .= '<span class="left_buttons">' . $left_buttons . '</span>';
  }
  $output .= '<span class="text">' . $label . '</span></div>';
  $output .= '<div class="panel-pane-collapsible">';
  $output .= theme('panels_pane_collapsible', $block);
  $output .= '</div>';
  return $output;
}

// @DND
function theme_panels_pane_collapsible($block) {
  $output = '';
  $output .= '<h2 class="title">' . $block->title . '</h2>';
  $output .= '<div class="content">' . filter_xss_admin($block->content) . '</div>';
  return $output;
}
