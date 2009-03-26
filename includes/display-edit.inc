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
function _panels_edit($display, $destination, $content_types, $title = FALSE) {
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
    $cache->display_title = $title;
    panels_cache_set('display', $did, $cache);
  }

  ctools_include('form');
  $form_state = array(
    'display' => &$display,
    're_render' => FALSE,
    'no_redirect' => TRUE,
  );

  $output = ctools_build_form('panels_edit_display_form', $form_state);
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
  @todo -- help file
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

  $links = panels_edit_display_get_links($display);

  $form['hide']['display-settings'] = array(
    '#value' => $links,
  );

  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();

  drupal_add_js(panels_get_path('js/panels-base.js'));
  drupal_add_js(panels_get_path('js/display_editor.js'));
  drupal_add_css(panels_get_path('css/panels_dnd.css'));
  drupal_add_css(panels_get_path('css/panels_admin.css'));

  return $form;
}

/**
 * Get the links for a panel display.
 *
 * This is abstracted out for easy ajax replacement.
 */
function panels_edit_display_get_links(&$display) {
  $links = array();
  $links[] = array(
    'title' => t('General settings'),
    'href' => 'panels/ajax/display-settings/' . $display->did,
    'attributes' => array('class' => 'ctools-use-modal'),
  );

  $panel_settings = $display->panel_settings;
  $style = panels_get_style((!empty($panel_settings['style'])) ? $panel_settings['style'] : 'default');

  $links[] = array(
    'title' => t('Display style: @style', array('@style' => $style['title'])),
    'href' => 'panels/ajax/style-type/display/' . $display->did,
    'attributes' => array('class' => 'ctools-use-modal'),
  );
  if (panels_plugin_get_function('style', $style, 'settings form')) {
    $links[] = array(
      'title' => ' -- ' . t('Style settings'),
      'href' => 'panels/ajax/style-settings/display/' . $display->did,
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }

  if (user_access('use panels caching features')) {
    $method = isset($display->cache['method']) ? $display->cache['method'] : 0;
    $info = panels_get_cache($method);
    $cache_method = isset($info['title']) ? $info['title'] : t('No caching');
    $links[] = array(
      'title' => t('Cache method: @method', array('@method' => $cache_method)),
      'href' => 'panels/ajax/cache-method/' . $display->did . '/display',
      'attributes' => array('class' => 'ctools-use-modal'),
    );
    if (panels_plugin_get_function('cache', $info, 'settings form')) {
      $links[] = array(
        'title' => ' -- ' . t('Cache settings'),
        'href' => 'panels/ajax/cache-settings/' . $display->did . '/display',
        'attributes' => array('class' => 'ctools-use-modal'),
      );
    }
  }

  return theme('ctools_dropdown', t('Display settings'), $links, FALSE, 'panels-display-links');
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

    $panel_buttons = panels_edit_panel_get_links($display, $panel_id);

    $content[$panel_id] = theme('panels_panel_dnd', $content[$panel_id], $panel_id, $title, $panel_buttons);
  }

  $output .= drupal_render($form);
  panels_load_include('display-render');
  $output .= theme('panels_dnd', panels_render_layout_admin($layout, $content, $display));
  $output .= $save_buttons;
  return $output;
}

/**
 * Get the links for a panel region.
 *
 * This is abstracted out for easy ajax replacement.
 */
function panels_edit_panel_get_links($display, $panel_id) {
  $links = array();
  $links[] = array(
    'title' => t('Add content'),
    'href' => "panels/ajax/add-pane/$display->did/$panel_id",
    'attributes' => array(
      'class' => 'ctools-use-modal',
    ),
  );

  $panel_settings = $display->panel_settings;
  $style = panels_get_style((!empty($panel_settings[$panel_id]['style'])) ? $panel_settings[$panel_id]['style'] : '-1');
  $style_title = isset($style['title']) ? $style['title'] : t('Default');

  $links[] = array(
    'title' => t('Region style: @style', array('@style' => $style_title)),
    'href' => 'panels/ajax/style-type/panel/' . $display->did . '/' . $panel_id,
    'attributes' => array('class' => 'ctools-use-modal'),
  );
  if (panels_plugin_get_function('style', $style, 'settings form')) {
    $links[] = array(
      'title' => ' -- ' . t('Style settings'),
      'href' => 'panels/ajax/style-settings/panel/' . $display->did . '/' . $panel_id,
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }

  return theme('ctools_dropdown', theme('image', panels_get_path("images/icon-addcontent.png")), $links, TRUE, 'pane-add-link panels-region-links-' . $panel_id);
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
  $links = array();

  if (!empty($pane->shown)) {
    $links[] = array(
      'title' => t('Disable this pane'),
      'href' => "panels/ajax/hide/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-ajax'),
    );
  }
  else {
    $links[] = array(
      'title' => t('Enable this pane'),
      'href' => "panels/ajax/show/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-ajax'),
    );
  }

  $links[] = array(
    'title' => t('Settings'),
    'href' => "panels/ajax/configure/$display->did/$pane->pid",
    'attributes' => array('class' => 'ctools-use-modal'),
  );

  $style = panels_get_style((!empty($pane->style['style'])) ? $pane->style['style'] : 'default');

  $links[] = array(
    'title' => t('Style: @style', array('@style' => $style['title'])),
    'href' => 'panels/ajax/style-type/pane/' . $display->did . '/' . $pane->pid,
    'attributes' => array('class' => 'ctools-use-modal'),
  );

  if (panels_plugin_get_function('style', $style, 'pane settings form')) {
    $links[] = array(
      'title' => ' -- ' . t('Style settings'),
      'href' => 'panels/ajax/style-settings/pane/' . $display->did . '/' . $pane->pid,
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }

  if (user_access('administer advanced pane settings')) {
    $links[] = array(
      'title' => t('CSS properties'),
      'href' => "panels/ajax/pane-css/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }

  if (user_access('administer pane access')) {
    $links[] = array(
      'title' => t('Visibility settings'),
      'href' => "panels/ajax/access-settings/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-modal'),
    );

    $contexts = $display->context;
    // Make sure we have the logged in user context
    if (!isset($contexts['logged-in-user'])) {
      $contexts['logged-in-user'] = ctools_access_get_loggedin_context();
    }

    if (!empty($pane->access['plugins'])) {
      foreach ($pane->access['plugins'] as $id => $test) {
        $plugin = ctools_get_access_plugin($test['name']);
        $access_title  = isset($plugin['title']) ? $plugin['title'] : t('Broken/missing access plugin %plugin', array('%plugin' => $test['name']));
        $access_description = ctools_access_summary($plugin, $contexts, $test);

        $links[] = array(
          'title' => ' -- ' . t('@title: !description', array('@title' => $access_title, '!description' => $access_description)),
          'href' => "panels/ajax/access-test/$display->did/$pane->pid/$id",
          'attributes' => array('class' => 'ctools-use-modal'),
        );
      }
    }
    $links[] = array(
      'title' => ' -- ' . t('Add new visibility rule'),
      'href' => "panels/ajax/access-add/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }

  if (panels_get_caches() && user_access('use panels caching features')) {
    $method = isset($pane->cache['method']) ? $pane->cache['method'] : 0;
    $info = panels_get_cache($method);
    $cache_method = isset($info['title']) ? $info['title'] : t('No caching');
    $links[] = array(
      'title' => t('Cache method: @method', array('@method' => $cache_method)),
      'href' => "panels/ajax/cache-method/$display->did/$pane->pid",
      'attributes' => array('class' => 'ctools-use-modal'),
    );
    if (panels_plugin_get_function('cache', $info, 'settings form')) {
      $links[] = array(
        'title' => ' -- ' . t('Cache settings'),
        'href' => "panels/ajax/cache-settings/$display->did/$pane->pid",
        'attributes' => array('class' => 'ctools-use-modal'),
      );
    }
  }

  $links[] = array(
    'title' => t('Remove'),
    'href' => '#',
    'attributes' => array(
      'class' => 'pane-delete',
      'id' => "pane-delete-panel-pane-$pane->pid",
    ),
  );

  $buttons = theme('ctools_dropdown', theme('image', panels_get_path("images/icon-configure.png")), $links, TRUE);
  // Render administrative buttons for the pane.
/*
  // On the left we have the 'caching' link.
  if (panels_get_caches() && user_access('use panels caching features')) {
    $buttons .= ctools_modal_image_button(
      panels_get_path('images/icon-cache.png'),
      "panels/ajax/cache-method/$display->did/$pane->pid",
      t('Set caching options for "@pane"',
      array('@pane' => $title)), "pane-cache-link"
    );
  }

  // Next to it the show or hide pane link
  if (!empty($pane->shown)) {
    $buttons .= ctools_ajax_image_button(
      panels_get_path('images/icon-hidepane.png'),
      "panels/ajax/hide/$display->did/$pane->pid",
      t('Hide "@pane"', array('@pane' => $title)),
      "pane-toggle-shown-link"
    );
  }
  else {
    $buttons .= ctools_ajax_image_button(
      panels_get_path('images/icon-showpane.png'),
      "panels/ajax/show/$display->did/$pane->pid",
      t('Show "@pane"', array('@pane' => $title)),
      "pane-toggle-shown-link"
    );
  }

  // And the configure settings link
  $buttons .= ctools_modal_image_button(
    panels_get_path('images/icon-configure.png'),
    "panels/ajax/configure/$display->did/$pane->pid",
    t('Configure settings for "@pane"',
    array('@pane' => $title)), "pane-configure-link"
  );

  // The delete button doesn't use ctools_ajax_image_button because it doesn't
  // actually perform an ajax call.
  $alt = t('Delete pane "@pane"', array('@pane' => $title));

  $buttons .= l(theme('image', panels_get_path("images/icon-delete.png")), '', array('html' => TRUE, 'attributes' => array('class' => 'pane-delete', 'id' => "pane-delete-panel-pane-$pane->pid", 'title' => $alt, 'alt' => $alt)));
*/

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
 * Entry point for AJAX: 'Display settings' modal form, from which the user
 * can control settings for the display.
 *
 * @param int $did
 *  The display id of the $display object currently being edited.
 */
function panels_ajax_display_settings($did = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  $form_state = array(
    'display' => &$cache->display,
    'display_title' => !empty($cache->display_title),
    'title' => t('Display settings'),
    'ajax' => TRUE,
  );

  $output = ctools_modal_form_wrapper('panels_edit_display_settings_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);

    $output = array();
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
}

/**
 * Form for display settings.
 */
function panels_edit_display_settings_form(&$form_state) {
  $form = array();
  $display = &$form_state['display'];

  $layout = panels_get_layout($display->layout);
  $form_state['layout'] = $layout;

  if ($form_state['display_title']) {
    $form['display_title'] = array (
      '#tree' => TRUE,
    );

    $form['display_title']['hide_title'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide title'),
      '#default_value' => $display->hide_title,
      '#description' => t('If checked, this panel will have no visible title.'),
    );

    $form['display_title']['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $display->title,
      '#title' => t('Title'),
      '#description' => t('The title of this panel. @TODO: List possible keyword substitutions here.'),
      '#process' => array('ctools_process_dependency'),
      '#dependency' => array('edit-display-title-hide-title' => array(0)),
    );

    if (isset($title) && is_string($title)) {
      $form['display_title']['title']['#description'] .= " " . t("If you leave this field blank, then the default title, '@title', will be used instead.", array('@title' => $title));
    }
  }

  // TODO doc the ability to do this as part of the API
  if (!empty($layout['settings form']) && function_exists($layout['settings form'])) {
    $form['layout_settings'] = $layout['settings form']($display, $layout, $display->layout_settings);
  }
  $form['layout_settings']['#tree'] = TRUE;

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#return_here' => TRUE,
  );

  return $form;
}

/**
 * Validate the layout settings form.
 */
function panels_edit_display_settings_form_validate($form, &$form_state) {
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings validate')) {
    $function($form_state['values']['layout_settings'], $form['layout_settings'], $form_state['display'], $form_state['layout'], $form_state['display']->layout_settings);
  }
}

/**
 * Store changes from the layout settings form.
 */
function panels_edit_display_settings_form_submit($form, &$form_state) {
  $display = &$form_state['display'];
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings submit')) {
    $function($form_state['values']['layout_settings'], $display, $form_state['layout'], $display->layout_settings);
  }

  // Since not all layouts have layout settings, check here in case of notices.
  if (isset($form_state['values']['layout_settings'])) {
    $display->layout_settings = $form_state['values']['layout_settings'];
  }

  if (isset($form_state['values']['display_title']['title'])) {
    $display->title = $form_state['values']['display_title']['title'];
    $display->hide_title = $form_state['values']['display_title']['hide_title'];
  }
}

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
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((is_numeric($did) || $did == 'new') && $cache = panels_cache_get('display', $did)) {
    $display = $cache->display;
    $layout = panels_get_layout($display->layout);
    $layout_panels = panels_get_panels($layout, $display);

    if ($layout && array_key_exists($panel_id, $layout_panels)) {
      ctools_modal_render(
        t('Add content to !s', array('!s' => $layout_panels[$panel_id])),
        panels_add_content($cache, $panel_id)
      );
    }
  }
  ctools_modal_render(t('Error'), t('Invalid input'));
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

        // @todo Icon finding code for this needs to be abstracted.
        $icon = '';

        if (isset($subtype_info['icon'])) {
          $icon = $subtype_info['icon'];
          if (isset($subtype_info['path']) && file_exists($subtype_info['path'] . '/' . $icon)) {
            $path = $subtype_info['path'];
          }
          else if (isset($content_type['path']) && file_exists($content_type['path'] . '/' . $icon)) {
            $path = $content_type['path'];
          }
          else if (file_exists(panels_get_path("plugins/content_types/$type_name") . '/' . $icon)) {
            $path = panels_get_path("plugins/content_types/$type_name");
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
        $url = "panels/ajax/add-pane-config/$did/$panel_id/$type_name/$subtype_name";
        $output .= ctools_ajax_image_button($path . '/' . $icon, $url, $description, 'panels-modal-add-config');
        $output .= '<div>' . ctools_ajax_text_button($title, $url, $description, 'panels-modal-add-config') . '</div>';
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
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  $pane = panels_new_pane($type, $subtype);
  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
  
  $content_type = panels_get_content_type($type);
  
  if (isset($content_type['defaults'])) {
    $defaults = $content_type['defaults'];
  }
  else if (isset($subtype['defaults'])) {
    $defaults = $subtype['defaults'];
  }
  if (isset($defaults)) {
    if (is_string($defaults) && is_array($defaults)) {
      if ($return = $defaults($pane)) {
        $pane->configuration = $return;
      }
    }
    else if (is_array($defaults)) {
      $pane->configuration = $defaults;
    }
  }

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure !subtype_title', array('!subtype_title' => $subtype['title'])),
    'type' => panels_get_content_type($pane->type),
    'subtype' => $subtype,
    'op' => 'add',
  );

  $output = ctools_modal_form_wrapper('panels_content_config_form', $form_state);
  if (empty($output)) {
    // Get a real pid for this pane.
    $pane->pid = "new-" . $cache->display->next_new_pid();
    // Put the pane into the display where it belongs
    $pane->panel = $panel_id;
    $cache->display->content[$pane->pid] = $pane;
    $cache->display->panels[$panel_id][] = $pane->pid;

    panels_cache_set('display', $did, $cache);

    $output = array();
    $output[] = ctools_ajax_command_append("#panel-pane-$panel_id", panels_show_pane($cache->display, $pane, TRUE));
    $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
}

/**
 * AJAX entry point for to configure a pane that has just been added.
 */
function panels_ajax_configure_pane($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];

  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure !subtype_title', array('!subtype_title' => $subtype['title'])),
    'type' => panels_get_content_type($pane->type),
    'subtype' => $subtype,
    'op' => 'edit',
  );

  $output = ctools_modal_form_wrapper('panels_content_config_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);
    $output = array();
    $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
    $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
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
  panels_ct_pane_validate_form($form_state['pane']->type, $form['configuration'], $form_state, $form_state['op']);
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
  ctools_include('ajax');
  ctools_include('modal'); // panels_show_pane relies on this.
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_ajax_render_error(t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_ajax_render_error(t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];

  $pane->shown = ($op == 'show');
  panels_cache_set('display', $did, $cache);

  $output = array();
  $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
  $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");

  ctools_ajax_render($output);
}

/**
 * Entry point for AJAX modal: configure pane cache method
 */
function panels_ajax_cache_method($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }
  // This lets us choose whether we're doing the display's cache or
  // a pane's.
  if ($pid == 'display') {
    $conf = &$cache->display->cache;
    $title = t('Cache method for this display');
  }
  else if (!empty($cache->display->content[$pid])) {
    $pane = &$cache->display->content[$pid];
    $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
    $conf = &$pane->cache;
    $title = t('Cache method for !subtype_title', array('!subtype_title' => $subtype['title']));
  }
  else {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $form_state = array(
    'display' => &$cache->display,
    'conf' => &$conf,
    'title' => $title,
    'ajax' => TRUE,
  );

  $output = ctools_modal_form_wrapper('panels_edit_cache_method_form', $form_state);
  if (empty($output)) {
    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    $info = panels_get_cache($form_state['method']);
    $function = panels_plugin_get_function('cache', $info, 'settings form');
    if (!$function) {
      $conf['method'] = $form_state['method'];
      $conf['settings'] = array();
      panels_cache_set('display', $did, $cache);

      $output = array();
      $output[] = ctools_modal_command_dismiss();

      if ($pid != 'display') {
        $output[] = ctools_ajax_command_replace("#panel-pane-$pid", panels_show_pane($cache->display, $pane, TRUE));
        $output[] = ctools_ajax_command_changed("#panel-pane-$pid", "div.grabber span.text");
      }
      else {
        $links = panels_edit_display_get_links($cache->display);
        $output[] = ctools_ajax_command_replace('.panels-display-links', $links);
      }
    }
    else {
      $cache->method = $form_state['method'];
      panels_cache_set('display', $did, $cache);
      // send them to next form.
      return panels_ajax_cache_settings($cache, $pid);
    }
  }

  ctools_ajax_render($output);
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
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if (is_object($did)) {
    // This is safe: Objects cannot be passed via URL.
    $cache = $did;
    $did = $cache->display->did;
  }
  else if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  // This lets us choose whether we're doing the display's cache or
  // a pane's.
  if ($pid == 'display') {
    $conf = &$cache->display->cache;
    $title = t('Cache settings for this display');
  }
  else if (!empty($cache->display->content[$pid])) {
    $pane = &$cache->display->content[$pid];
    $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);

    $conf = &$pane->cache;
    $title = t('Cache settings for !subtype_title', array('!subtype_title' => $subtype['title']));
  }
  else {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  if (isset($cache->method) && (empty($conf['method']) || $conf['method'] != $cache->method)) {
    $conf['method'] = $cache->method;
    $info = panels_get_cache($conf['method']);
    $conf['settings'] = isset($info['defaults']) ? $info['defaults'] : array();
  }

  $form_state = array(
    'display' => &$cache->display,
    'pid' => $pid,
    'conf' => &$conf,
    'ajax' => TRUE,
    'title' => $title,
    'url' => url("panels/ajax/cache-settings/$did/$pid", array('absolute' => TRUE)),
  );

  $output = ctools_modal_form_wrapper('panels_edit_cache_settings_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);

    $output = array();
    $output[] = ctools_modal_command_dismiss();

    if ($pid != 'display') {
      $output[] = ctools_ajax_command_replace("#panel-pane-$pid", panels_show_pane($cache->display, $pane, TRUE));
      $output[] = ctools_ajax_command_changed("#panel-pane-$pid", "div.grabber span.text");
    }
    else {
      $links = panels_edit_display_get_links($cache->display);
      $output[] = ctools_ajax_command_replace('.panels-display-links', $links);
    }
  }
  ctools_ajax_render($output);
}

/**
 * Cache settings form
 */
function panels_edit_cache_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];
  $pid = $form_state['pid'];
  $info = panels_get_cache($conf['method']);

  $form['#action'] = $form_state['url'];

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($info['description']),
  );

  $function = panels_plugin_get_function('cache', $conf['method'], 'settings form');

  $form['settings'] = $function($conf['settings'], $display, $pid);
  $form['settings']['#tree'] = TRUE;

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
 * Entry point for AJAX modal: configure pane cache method
 */
function panels_ajax_style_type($type = NULL, $did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  // This lets us choose whether we're doing the display's cache or
  // a pane's.
  switch ($type) {
    case 'display':
      $style = isset($cache->display->panel_settings['style']) ? $cache->display->panel_settings['style'] : 'default';
      $title = t('Default style for this display');
      break;

    case 'panel':
      $style = isset($cache->display->panel_settings[$pid]['style']) ? $cache->display->panel_settings[$pid]['style'] : '-1'; // -1 signifies to use the default setting.
      $layout = panels_get_layout($cache->display->layout);
      $layout_panels = panels_get_panels($layout, $cache->display);
      $title = t('Panel style for region "!region"', array('!region' => $layout_panels[$pid]));
      break;

    case 'pane':
      $pane = &$cache->display->content[$pid];
      $style = isset($pane->style['style']) ? $pane->style['style'] : 'default';
      $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
      $title = t('Pane style for "!pane"', array('!pane' => $subtype['title']));
      break;

    default:
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $form_state = array(
    'display' => &$cache->display,
    'style' => $style,
    'title' => $title,
    'ajax' => TRUE,
    'type' => $type,
  );

  $output = ctools_modal_form_wrapper('panels_edit_style_type_form', $form_state);
  if (empty($output)) {
    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    $style = panels_get_style($form_state['style']);
    $function = panels_plugin_get_function('style', $style, ($type == 'pane') ? 'pane settings form' : 'settings form');
    if (!$function) {
      // If there's no settings form, just change the style and exit.
      switch($type) {
        case 'display':
          $cache->display->panel_settings['style'] = $form_state['style'];
          if (isset($cache->display->panel_settings['style_settings']['default'])) {
            unset($cache->display->panel_settings['style_settings']['default']);
          }
          break;

        case 'panel':
          $cache->display->panel_settings[$pid]['style'] = $form_state['style'];
          if (isset($cache->display->panel_settings['style_settings'][$pid])) {
            unset($cache->display->panel_settings['style_settings'][$pid]);
          }
          break;

        case 'pane':
          $pane->style['style'] = $form_state['style'];
          if (isset($pane->style['settings'])) {
            unset($pane->style['settings']);
          }

          break;
      }
      panels_cache_set('display', $did, $cache);

      $output = array();
      $output[] = ctools_modal_command_dismiss();

      if ($type == 'pane') {
        $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
        $output[] = ctools_ajax_command_changed("#panel-pane-$pid", "div.grabber span.text");
      }
      else if ($type == 'panel') {
        $links = panels_edit_panel_get_links($cache->display, $pid);
        $output[] = ctools_ajax_command_replace('.panels-region-links-' . $pid, $links);
      }
      else {
        $links = panels_edit_display_get_links($cache->display);
        $output[] = ctools_ajax_command_replace('.panels-display-links', $links);
      }
    }
    else {
      if ($form_state['style'] != $form_state['old_style']) {
        $cache->style = $form_state['style'];
        panels_cache_set('display', $did, $cache);
      }

      // send them to next form.
      return panels_ajax_style_settings($type, $cache, $pid);
    }
  }

  ctools_ajax_render($output);
}

/**
 * Choose cache method form
 */
function panels_edit_style_type_form(&$form_state) {
  $display = &$form_state['display'];
  $style = $form_state['style'];
  $type = $form_state['type'];

  $styles = panels_get_styles();

  $function = ($type == 'pane' ? 'render pane' : 'render panel');
  $options = array();
  if ($type == 'panel') {
    $options[-1] = t('Use display default style');
  }

  foreach ($styles as $id => $info) {
    if (empty($info['hidden']) && (!empty($info[$function]) || $id == 'default')) {
      $options[$id] = check_plain($info['title']);
    }
  }

  asort($options);

  $form['style'] = array(
    '#prefix' => '<div class="no-float">',
    '#suffix' => '</div>',
    '#type' => 'radios',
    '#title' => t('Style'),
    '#options' => $options,
    '#default_value' => $style,
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
function panels_edit_style_type_form_submit($form, &$form_state) {
  $form_state['old_style'] = $form_state['style'];
  $form_state['style'] = $form_state['values']['style'];
}

/**
 * Handle the cache settings form
 */
function panels_ajax_style_settings($type, $did, $pid = '') {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if (is_object($did)) {
    // This is safe: Objects cannot be passed via URL.
    $cache = $did;
    $did = $cache->display->did;
  }
  else if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (isset($cache->style)) {
    $style = panels_get_style($cache->style);
    $defaults = isset($style['defaults']) ? $style['defaults'] : array();
    // Get the &$conf variable based upon whose style we're editing.
    switch ($type) {
      case 'display':
        $cache->display->panel_settings['style'] = $cache->style;
        $cache->display->panel_settings['style_settings']['default'] = $defaults;
        $conf = &$cache->display->panel_settings['style_settings']['default'];
        break;

      case 'panel':
        $cache->display->panel_settings[$pid]['style'] = $cache->style;
        $cache->display->panel_settings['style_settings'][$pid] = $defaults;
        $conf = &$cache->display->panel_settings['style_settings'][$pid];
        break;

      case 'pane':
        $pane = &$cache->display->content[$pid];
        $pane->style['style'] = $cache->style;
        $pane->style['settings'] = $defaults;
        $conf = &$pane->style['settings'];
        break;
    }
  }
  else {
    switch ($type) {
      case 'display':
        $style = panels_get_style($cache->display->panel_settings['style']);
        break;

      case 'panel':
        $style = panels_get_style($cache->display->panel_settings[$pid]['style']);
        break;

      case 'pane':
        $pane = &$cache->display->content[$pid];
        $style = panels_get_style($pane->style['style']);
        break;
    }
  }

  // Set up our $conf reference.
  // We should only be able to get here after a style has already been set up with
  // defaults so I think it is safe to not check.
  switch ($type) {
    case 'display':
      $conf = &$cache->display->panel_settings['style_settings']['default'];
      $title = t('Style settings for @style (display)', array('@style' => $style['title']));
      break;

    case 'panel':
      $conf = &$cache->display->panel_settings['style_settings'][$pid];
      $layout = panels_get_layout($cache->display->layout);
      $layout_panels = panels_get_panels($layout, $cache->display);
      $title = t('Style settings for style @style (Region "!region")', array('@style' => $style['title'], '!region' => $layout_panels[$pid]));
      break;

    case 'pane':
      $pane = &$cache->display->content[$pid];
      $conf = &$pane->style['settings'];
      $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
      $title = t('Style settings for style @style (Pane "!pane")', array('@style' => $style['title'], '!pane' => $subtype['title']));
      break;
  }

  $form_state = array(
    'display' => &$cache->display,
    'type' => $type,
    'pid' => $pid,
    'conf' => &$conf,
    'style' => $style,
    'ajax' => TRUE,
    'title' => $title,
    'url' => url("panels/ajax/style-settings/$type/$did/$pid", array('absolute' => TRUE)),
  );

  $output = ctools_modal_form_wrapper('panels_edit_style_settings_form', $form_state);
  if (empty($output)) {
    if (isset($cache->style)) {
      unset($cache->style);
    }

    // $conf was a reference so it should just modify.
    panels_cache_set('display', $did, $cache);

    $output = array();
    $output[] = ctools_modal_command_dismiss();

    if ($type == 'pane') {
      $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
      $output[] = ctools_ajax_command_changed("#panel-pane-$pid", "div.grabber span.text");
    }
    else if ($type == 'panel') {
      $links = panels_edit_panel_get_links($cache->display, $pid);
      $output[] = ctools_ajax_command_replace('.panels-region-links-' . $pid, $links);
    }
    else {
      $links = panels_edit_display_get_links($cache->display);
      $output[] = ctools_ajax_command_replace('.panels-display-links', $links);
    }
  }
  ctools_ajax_render($output);
}

/**
 * Cache settings form
 */
function panels_edit_style_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];
  $pid = $form_state['pid'];
  $style = $form_state['style'];
  $type = $form_state['type'];

  $form['#action'] = $form_state['url'];

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($style['description']),
  );

  $function = panels_plugin_get_function('cache', $style, ($type == 'pane') ? 'pane settings form' : 'settings form');

  $form['settings'] = $function($conf, $display, $pid, $type);
  $form['settings']['#tree'] = TRUE;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate cache settings.
 */
function panels_edit_style_settings_form_validate($form, &$form_state) {
  $name = $form_state['type'] == 'pane' ? 'pane settings form validate' : 'settings form validate';
  if ($function = panels_plugin_get_function('style', $form_state['style'], $name)) {
    $function($form, $form_state['values']['settings']);
  }
}

/**
 * Allows panel styles to validate their style settings.
 */
function panels_edit_style_settings_form_submit($form, &$form_state) {
  $name = $form_state['type'] == 'pane' ? 'pane settings form submit' : 'settings form submit';
  if ($function = panels_plugin_get_function('style', $form_state['style'], $name)) {
    $function($form_state['values']['settings']);
  }

  $form_state['conf'] = $form_state['values']['settings'];
}

/**
 * AJAX entry point to configure CSS for a pane.
 */
function panels_ajax_configure_pane_css($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];
  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure CSS on !subtype_title', array('!subtype_title' => $subtype['title'])),
  );

  $output = ctools_modal_form_wrapper('panels_content_configure_pane_css_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);
    $output = array();
//    $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
    $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
}


/**
 * Configure CSS on a pane form.
 */
function panels_content_configure_pane_css_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $form['css_id'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($pane->css['css_id']) ? $pane->css['css_id'] : '',
    '#title' => t('CSS ID'),
    '#description' => t('CSS ID to apply to this pane. This may be blank.'),
  );
  $form['css_class'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($pane->css['css_class']) ? $pane->css['css_class'] : '',
    '#title' => t('CSS class'),
    '#description' => t('CSS class to apply to this pane. This may be blank.'),
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * FAPI submission function for the CSS configure form.
 *
 * All this does is set up $pane properly. The caller is responsible for
 * actually storing this somewhere.
 */
function panels_content_configure_pane_css_form_submit($form, &$form_state) {
  $pane = &$form_state['pane'];
  $display = $form_state['display'];

  $pane->css['css_id'] = $form_state['values']['css_id'];
  $pane->css['css_class'] = $form_state['values']['css_class'];
}

/**
 * AJAX entry point to configure access settings for a pane.
 */
function panels_ajax_configure_access_settings($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];
  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Access settings on !subtype_title', array('!subtype_title' => $subtype['title'])),
  );

  $output = ctools_modal_form_wrapper('panels_content_configure_access_settings_form', $form_state);
  if (empty($output)) {
    panels_cache_set('display', $did, $cache);
    $output = array();
    $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
    $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
}


/**
 * Form to control basic visibility settings.
 */
function panels_content_configure_access_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $form['logic'] = array(
    '#type' => 'radios',
    '#options' => array(
      'and' => t('All criteria must pass.'),
      'or' => t('Only one criteria must pass.'),
    ),
    '#default_value' => isset($pane->access['logic']) ? $pane->access['logic'] : 'and',
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * FAPI submission function for the edit access settings form.
 *
 * All this does is set up $pane properly. The caller is responsible for
 * actually storing this somewhere.
 */
function panels_content_configure_access_settings_form_submit($form, &$form_state) {
  $pane = &$form_state['pane'];
  $display = $form_state['display'];

  $pane->access['logic'] = $form_state['values']['logic'];
}

/**
 * AJAX entry point for to add a visiblity rule.
 */
function panels_ajax_add_access_test($did = NULL, $pid = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];
  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Add visibility rule for !subtype_title', array('!subtype_title' => $subtype['title'])),
  );

  $output = ctools_modal_form_wrapper('panels_content_add_access_test_form', $form_state);
  if (empty($output)) {
    // Set up the plugin in cache
    $plugin = ctools_get_access_plugin($form_state['values']['type']);
    $cache->new_plugin = ctools_access_new_test($plugin);
    panels_cache_set('display', $did, $cache);

    // go to the next step.
    return panels_ajax_configure_access_test($cache, $pid, 'add');
  }

  ctools_ajax_render($output);
}

/**
 * Form to add a visibility rule.
 */
function panels_content_add_access_test_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $plugins = ctools_get_relevant_access_plugins($display->context);
  $options = array();
  foreach ($plugins as $id => $plugin) {
    $options[$id] = $plugin['title'];
  }

  asort($options);

  $form['type'] = array(
    // This ensures that the form item is added to the URL.
    '#type' => 'radios',
    '#options' => $options,
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * AJAX entry point for to configure vsibility rule
 */
function panels_ajax_configure_access_test($did = NULL, $pid = NULL, $id = NULL) {
  panels_load_include('plugins');
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');

  if (is_object($did)) {
    // This is safe: Objects cannot be passed via URL.
    $cache = $did;
    $did = $cache->display->did;
  }
  if ((!is_numeric($did) && $did != 'new') || !($cache = panels_cache_get('display', $did))) {
    ctools_modal_render(t('Error'), t('Invalid display id.'));
  }

  if (empty($cache->display->content[$pid])) {
    ctools_modal_render(t('Error'), t('Invalid pane id.'));
  }

  $pane = &$cache->display->content[$pid];
  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);

  // Set this up here because $id gets changed later.
  $url = "panels/ajax/access-test/$did/$pid/$id";

  // If we're adding a new one, get the stored data from cache and
  // add it. It's stored as a cache so that if this is closed
  // we don't accidentally add an unconfigured plugin.
  if ($id == 'add') {
    $pane->access['plugins'][] = $cache->new_plugin;
    $id = max(array_keys($pane->access['plugins']));
  }
  else if (empty($pane->access['plugins'][$id])) {
    ctools_modal_render(t('Error'), t('Invalid test id.'));
  }

  $form_state = array(
    'display' => &$cache->display,
    'pane' => &$pane,
    'ajax' => TRUE,
    'title' => t('Configure visibility rule for !subtype_title', array('!subtype_title' => $subtype['title'])),
    'test' => &$pane->access['plugins'][$id],
    'plugin' => ctools_get_access_plugin($pane->access['plugins'][$id]['name']),
    'url' => url($url, array('absolute' => TRUE)),
  );

  $output = ctools_modal_form_wrapper('panels_content_configure_access_test_form', $form_state);
  if (empty($output)) {
    // Unset the new plugin
    if (isset($cache->new_plugin)) {
      unset($cache->new_plugin);
    }

    if (!empty($form_state['remove'])) {
      unset($pane->access['plugins'][$id]);
    }

    panels_cache_set('display', $did, $cache);
    $output = array();
    $output[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", panels_show_pane($cache->display, $pane, TRUE));
    $output[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
    $output[] = ctools_modal_command_dismiss();
  }

  ctools_ajax_render($output);
}

/**
 * Form to configure a visibility rule.
 */
function panels_content_configure_access_test_form(&$form_state) {
  $display = &$form_state['display'];
  $test = &$form_state['test'];
  $plugin = &$form_state['plugin'];

  $form['#action'] = $form_state['url'];

  $contexts = $display->context;
  if (!isset($contexts['logged-in-user'])) {
    $contexts['logged-in-user'] = ctools_access_get_loggedin_context();
  }

  if (isset($plugin['required context'])) {
    $form['context'] = ctools_context_selector($contexts, $plugin['required context'], $test['context']);
  }

  $form['settings'] = array('#tree' => TRUE);
  if ($function = ctools_plugin_get_function($plugin, 'settings form')) {
    $function($form, $form_state, $test['settings']);
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['remove'] = array(
    '#type' => 'submit',
    '#value' => t('Remove'),
    '#remove' => TRUE,
  );

  return $form;
}

/**
 * Validate handler for visibility rule settings
 */
function panels_content_configure_access_test_form_validate(&$form, &$form_state) {
  if (!empty($form_state['clicked_button']['#remove'])) {
    return;
  }

  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form validate')) {
    $function($form, $form_state);
  }
}

/**
 * Submit handler for visibility rule settings
 */
function panels_content_configure_access_test_form_submit(&$form, &$form_state) {
  if (!empty($form_state['clicked_button']['#remove'])) {
    $form_state['remove'] = TRUE;
    return;
  }

  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form submit')) {
    $function($form, $form_state);
  }

  $form_state['test']['settings'] = $form_state['values']['settings'];
  if (isset($form_state['values']['context'])) {
    $form_state['test']['context'] = $form_state['values']['context'];
  }
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
