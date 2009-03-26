<?php
// $Id$

/**
 * @file
 *
 * Contains helper code for plugins and contexts.
 */


/**
 * @defgroup panels_content Panels content and pane helper/info functions
 * @{
 */

/**
 * Determine if a pane is visible.
 *
 * @param $pane
 *   The pane object to test for access.
 * @param $display
 *   The display object containing the pane object to be tested.
 */
function panels_pane_access($pane, $display) {
  ctools_include('context');
  return ctools_access($pane->access, $display->context);
}

/**
 * Get the content from a given content type.
 *
 * @param $type
 *   The content type. May be the name or an already loaded content type object.
 * @param $conf
 *   The configuration for the content type.
 * @param $args
 *   The arguments to the content type.
 * @param $context
 *   The panels_context object.
 * @param $incoming_content
 *   Any incoming content, if this display is a wrapper.
 */
function panels_ct_get_content($type, $subtype, $conf, $args, $context, $incoming_content) {
  if ($function = panels_plugin_get_function('content_types', $type, 'render callback')) {
    $content = $function($subtype, $conf, $args, $context, $incoming_content);
    if (empty($content->title) && !empty($content->subject)) {
      $content->title = $content->subject;
    }

    if (!empty($content->title) && empty($content->subject)) {
      $content->subject = $content->title;
    }

    return $content;
  }
}

/**
 * Get the title from a given content type.
 *
 * @param $type
 *   The content type. May be the name or an already loaded content type object.
 * @param $conf
 *   The configuration for the content type.
 * @param $context
 *   The panels_context object.
 * @param $incoming_content
 *   Any incoming content, if this display is a wrapper.
 */
function panels_ct_get_title($type, $subtype, $conf, $context = NULL, $incoming_content = NULL) {
  if ($function = panels_plugin_get_function('content_types', $type, 'title callback')) {
    return $function($subtype, $conf, $context, $incoming_content);
  }
  return t('Deleted/missing content type @type', array('@type' => $type));
}

/**
 * Add the default FAPI elements to the content type configuration form
 */
function panels_ct_conf_form($content_type, $subtype, $contexts, $conf) {
  ctools_include('context');
  if (!empty($subtype['required context']) && is_array($contexts)) {
    $form['context'] = ctools_context_selector($contexts, $subtype['required context'], isset($conf['context']) ? $conf['context'] : array());
  }

  // Unless we're not allowed to overide the title on this content type, add this
  // gadget to all panes.
  if (empty($content_type['no title override'])) {
    $form['aligner_start'] = array(
      '#value' => '<div class="option-text-aligner">',
    );
    $form['override_title'] = array(
      '#type' => 'checkbox',
      '#default_value' => isset($conf['override_title']) ? $conf['override_title'] : '',
      '#title' => t('Override title'),
      '#id' => 'override-title-checkbox',
    );
    $form['override_title_text'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($conf['override_title_text']) ? $conf['override_title_text'] : '',
      '#size' => 35,
      '#id' => 'override-title-textfield',
    );
    $form['aligner_stop'] = array(
      '#value' => '</div><div style="clear: both; padding: 0; margin: 0"></div>',
    );
    $form['override_title_markup'] = array(
      '#prefix' => '<div class="description">',
      '#suffix' => '</div>',
      '#value' => t('You may use %keywords from contexts, as well as %title to contain the original title.'),
    );
  }

  return $form;
}

/**
 * Call any content type-defined add/edit callbacks so that additions
 * to $form['configuration'] can be made.
 *
 * @see panels_content_config_form()
 *
 * @param object $pane
 *   The $pane object currently being edited.
 * @param $contexts
 *   A list of possible contexts.
 * @param $parents
 *   The #parents to be used on the form, because some form gadgets need to
 *   know where they live.
 * @param string $op
 *  Either 'add' or 'edit' depending on the operation being performed.
 * @param mixed $content_type
 *  This variable should only be passed if the calling function already has access to the
 *  relevant content_type data and wants to save a little on performance. If so, then the
 *  fully-loaded content type plugin declaration array should be passed.
 */
function panels_ct_pane_config_form($pane, $contexts, $parents, $op, $content_type = 'content_types') {
  if ($function = panels_plugin_get_function($content_type, $pane->type, "$op callback")) {
    return $function($pane->subtype, $parents, $pane->configuration);
  }
}

/**
 * Call any add/edit validators defined by the content type.
 *
 * @see panels_content_config_form_validate()
 *
 * @param $type
 *   The content type. May be the name or an already loaded content type object.
 * @param $form
 *   The actual Forms API form that is being validated.
 * @param $form_values
 *   The actual Forms API values being validated.
 * @param string $op
 *  Either 'add' or 'edit' depending on the operation being performed.
 */
function panels_ct_pane_validate_form($content_type, $form, &$form_state, $op) {
  if ($function = panels_plugin_get_function('content_types', $content_type, "$op validate callback")) {
    return $function($form, $form_state);
  }
}

/**
 * Call any add/edit submit handlers defined by the content type.
 *
 * @param string $content_type
 *  A string value containing the name of the content type.
 * @param $form_values
 *  The $form_values['configuration'] sub-array generated by FAPI for the
 *  overall ct add/edit submit handler.
 * @param string $op
 *  Either 'add' or 'edit' depending on the operation being performed.
 */
function panels_ct_pane_submit_form($content_type, &$form_values, $op) {
  if ($function = panels_plugin_get_function('content_types', $content_type, "$op submit callback")) {
    return $function($form_values);
  }
}

/**
 * Get all of the individual subtypes provided by a given content type. This
 * would be all of the blocks for the block type, or all of the views for
 * the view type.
 *
 * @param $type
 *   The content type to load.
 */
function panels_ct_get_subtypes($type) {
  if (is_array($type)) {
    $content_type = $type;
  }
  else {
    $content_type = panels_get_content_type($type);
  }

  // Leave this in to maintain a little backward compatibility. We can take
  // this out later.
  if (isset($content_type['content types'])) {
    $function = $content_type['content types'];
  }
  else {
    $function = $content_type['content_types'];
  }
  
  if (is_array($function)) {
    return (array) $function;
  }

  if (function_exists($function)) {
    return (array) $function();
  }
  return array();
}

/**
 * Given a content type and a subtype id, return the information about that
 * content subtype.
 * 
 * @param <type> $type
 * @param <type> $subtype_id 
 */
function panels_ct_get_subtype($type, $subtype_id) {
  $function = panels_plugin_get_function('content_types', $type, 'content_type');
  if ($function) {
    return $function($subtype_id);
  }
  else {
    $subtypes = panels_ct_get_subtypes($type);
    if (isset($subtypes[$subtype_id])) {
      return $subtypes[$subtype_id];
    }
  }
}

/**
 * Get a list of panels available in the layout.
 */
function panels_get_panels($layout, $display) {
  if (!empty($layout['panels function']) && function_exists($layout['panels function'])) {
    return $layout['panels function']($display, $display->layout_settings);
  }
  if (!empty($layout['panels'])) {
    return $layout['panels'];
  }
  return array();
}

/**
 * Get an array of all available content types that can be fed into the
 * display editor for the add content list.
 *
 * @param $context
 *   If a context is provided, content that requires that context can apepar.
 * @param $has_content
 *   Whether or not the display will have incoming content
 * @param $allowed_types
 *   An array of allowed content types (pane types) keyed by content_type . '-' . sub_type
 * @param $default_types
 *   A default allowed/denied status for content that isn't known about
 */
function panels_get_available_content_types($contexts = NULL, $has_content = FALSE, $allowed_types = NULL, $default_types = NULL) {
  $content_types = panels_get_content_types();
  $available = array();

  ctools_include('context');
  foreach ($content_types as $id => $type) {
    foreach (panels_ct_get_subtypes($type) as $cid => $cinfo) {
      // exclude items that require content if we're saying we don't
      // provide it.
      if (!empty($cinfo['requires content']) && !$has_content) {
        continue;
      }

      // Check to see if the content type can be used in this context.
      if (!empty($cinfo['required context'])) {
        if (!ctools_context_filter($contexts, $cinfo['required context'])) {
          continue;
        }
      }

      // Check to see if the passed-in allowed types allows this content.
      if ($allowed_types) {
        $key = $id . '-' . $cid;
        if (!isset($allowed_types[$key])) {
          $allowed_types[$key] = isset($default_types[$id]) ? $default_types[$id] : $default_types['other'];
        }
        if (!$allowed_types[$key]) {
          continue;
        }
      }

      // If we made it through all the tests, then we can use this content.
      $available[$id][$cid] = $cinfo;
    }
  }
  return $available;
}

/**
 * Get an array of all content types that can be fed into the
 * display editor for the add content list, regardless of
 * availability.
 *
 */
function panels_get_all_content_types() {
  $content_types = panels_get_content_types();
  $available = array();

  foreach ($content_types as $id => $type) {
    foreach (panels_ct_get_subtypes($type) as $cid => $cinfo) {
      // If we made it through all the tests, then we can use this content.
      $available[$id][$cid] = $cinfo;
    }
  }
  return $available;
}

// ------------------------------------------------------------------
// Functions to provide information about a panel or part of a panel.

/**
 * Get the content from a given pane.
 *
 * @param $pane
 *   The pane to retrieve content from.
 * @param $args
 *   The arguments sent to the display.
 * @param $context
 *   The panels context.
 * @param $incoming_content
 *   Any incoming content if this display is a wrapper.
 */
function panels_get_pane_content($display, $pane, $args = array(), $context = NULL, $incoming_content = '') {
  ctools_include('context');
  if (!$context) {
    $context = new ctools_context;
  }

  if (!$incoming_content === '') {
    $incoming_content = t('Incoming content will be displayed here.');
  }

  $content = FALSE;
  $caching = !empty($pane->cache['method']) ? TRUE : FALSE;
  if ($caching && ($cache = panels_get_cached_content($display, $args, $context, $pane))) {
    $content = $cache->content;
  }
  else {
    $content = panels_ct_get_content($pane->type, $pane->subtype, $pane->configuration, $args, $context, $incoming_content);
    foreach (module_implements('panels_pane_content_alter') as $module) {
      $function = $module . '_panels_pane_content_alter';
      $function($content, $pane, $args, $context);
    }
    if ($caching) {
      $cache = new panels_cache_object();
      $cache->set_content($content);
      panels_set_cached_content($cache, $display, $args, $context, $pane);
    }
  }

  return $content;
}

/**
 * Get cached content for a given display and possibly pane.
 *
 * @return
 *   The cached content, or FALSE to indicate no cached content exists.
 */
function panels_get_cached_content($display, $args, $context, $pane = NULL) {
  $method = $pane ? $pane->cache['method'] : $display->cache['method'];
  $function = panels_plugin_get_function('cache', $method, 'cache get');

  if (!$function) {
    return FALSE;
  }

  $conf = $pane ? $pane->cache['settings'] : $display->cache['settings'];
  $cache = $function($conf, $display, $args, $context, $pane);
  if (empty($cache)) {
    return FALSE;
  }

  // restore it.
  $cache->restore();
  return $cache;
}

/**
 * Store cached content for a given display and possibly pane.
 */
function panels_set_cached_content($cache, $display, $args, $context, $pane = NULL) {
  $method = $pane ? $pane->cache['method'] : $display->cache['method'];
  $function = panels_plugin_get_function('cache', $method, 'cache set');

  if (!$function) {
    return FALSE;
  }

  $conf = $pane ? $pane->cache['settings'] : $display->cache['settings'];

  // snapshot it.
  $cache->cache();
  return $function($conf, $cache, $display, $args, $context, $pane);
}

/**
 * Clear all cached content for a display.
 */
function panels_clear_cached_content($display) {
  // Figure out every method we might be using to cache content in this display:
  $methods = array();
  if (!empty($display->cache['method'])) {
    $methods[$display->cache['method']] = TRUE;
  }

  foreach ($display->content as $pane) {
    if (!empty($pane->cache['method'])) {
      $methods[$pane->cache['method']] = TRUE;
    }
  }

  foreach (array_keys($methods) as $method) {
    $function = panels_plugin_get_function('cache', $method, 'cache clear');
    if ($function) {
      $function($display);
    }
  }
}

/**
 * An object to hold caching information while it is happening.
 */
class panels_cache_object {
  var $content = '';
  var $head = NULL;
  var $css = NULL;
  var $js = NULL;
  var $ready = FALSE;

  /**
   * When constructed, take a snapshot of our existing out of band data.
   */
  function panels_cache_object() {
    $this->head = drupal_set_html_head();
    $this->css = drupal_add_css();

    foreach (array('header', 'footer') as $scope) {
      $this->js[$scope] = drupal_add_js(NULL, NULL, $scope);
    }
  }

  /**
   * Add content to the cache. This assumes a pure stream;
   * use set_content() if it's something else.
   */
  function add_content($content) {
    $this->content .= $content;
  }

  function set_content($content) {
    $this->content = $content;
  }

  /**
   * Set the object for storing. This overwrites.
   */
  function cache() {
    if ($this->ready) {
      return;
    }

    $this->ready = TRUE;

    // Simple replacement for head
    $this->head = str_replace($this->head, '', drupal_set_html_head());

    // Slightly less simple for CSS:
    $css = drupal_add_css();
    $start = $this->css;
    $this->css = array();

    foreach ($css as $media => $medias) {
      foreach ($medias as $type => $types) {
        foreach ($types as $path => $preprocess) {
          if (!isset($start[$media][$type][$path])) {
            $this->css[] = array($path, $type, $media, $preprocess);
          }
        }
      }
    }

    $js = array();
    // A little less simple for js
    foreach (array('header', 'footer') as $scope) {
      $js[$scope] = drupal_add_js(NULL, NULL, $scope);
    }

    $start = $this->js;
    $this->js = array();

    foreach ($js as $scope => $scopes) {
      foreach ($scopes as $type => $types) {
        foreach ($types as $id => $info) {
          if (!isset($start[$scope][$type][$id])) {
            switch ($type) {
              case 'setting':
                $this->js[] = array($info, $type, $scope);
                break;

              case 'inline':
                $this->js[] = array($info['code'], $type, $scope, $info['defer']);
                break;

              default:
                $this->js[] = array($id, $type, $scope, $info['defer'], $info['cache']);
            }
          }
        }
      }
    }
  }

  /**
   * Restore out of band data saved to cache.
   */
  function restore() {
    if (!empty($this->head)) {
      drupal_set_html_head($this->head);
    }
    if (!empty($this->css)) {
      foreach ($this->css as $args) {
        call_user_func_array('drupal_add_css', $args);
      }
    }
    if (!empty($this->js)) {
      foreach ($this->js as $args) {
        call_user_func_array('drupal_add_js', $args);
      }
    }
  }
}

/**
 * Get the title of a pane.
 *
 * @param $pane
 *   The $pane object.
 */
function panels_get_pane_title(&$pane, $context = array(), $incoming_content = NULL) {
  if (empty($pane->context)) {
    $pane->context = panels_pane_select_context($pane, $context);
    if ($pane->context === FALSE) {
      return t('Does not meet context requirements');
    }
  }
  return panels_ct_get_title($pane->type, $pane->subtype, $pane->configuration, $pane->context, $incoming_content);
}

/**
 * @} End of "defgroup panels_content".
 */

/**
 * Select a context for a pane.
 *
 * @param $pane
 *   A fully populated pane.
 * @param $contexts
 *   A keyed array of available contexts.
 *
 * @return
 *   The matching contexts or NULL if none or necessary, or FALSE if
 *   requirements can't be met.
 */
function panels_pane_select_context($pane, $contexts) {
  // Identify which of our possible contexts apply.
  if (empty($pane->subtype)) {
    return;
  }

  $subtype = panels_ct_get_subtype($pane->type, $pane->subtype);
  // If the pane requires a context, fetch it; if no context is returned,
  // do not display the pane.
  if (empty($subtype) || empty($subtype['required context'])) {
    return;
  }

  $context = ctools_context_select($contexts, $subtype['required context'],
    $pane->configuration['context']);

  return $context;
}


/**
 * Fetch metadata on a specific layout plugin.
 *
 * @param $layout
 *   Name of a panel layout.
 *
 * @return
 *   An array with information about the requested panel layout.
 */
function panels_get_layout($layout) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'layouts', $layout);
}

/**
 * Fetch metadata for all layout plugins.
 *
 * @return
 *   An array of arrays with information about all available panel layouts.
 */
function panels_get_layouts() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'layouts');
}

/**
 * Fetch metadata on a specific style plugin.
 *
 * @param $style
 *   Name of a panel style.
 *
 * @return
 *   An array with information about the requested panel style.
 */
function panels_get_style($style) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'styles', $style);
}

/**
 * Fetch metadata for all style plugins.
 *
 * @return
 *   An array of arrays with information about all available panel styles.
 */
function panels_get_styles() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'styles');
}


/**
 * Fetch metadata on a specific content_type plugin.
 *
 * @param $content type
 *   Name of a panel content type.
 *
 * @return
 *   An array with information about the requested panel content type.
 */
function panels_get_content_type($content_type) {
  ctools_include('context');
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'content_types', $content_type);
}

/**
 * Fetch metadata for all content_type plugins.
 *
 * @return
 *   An array of arrays with information about all available panel content types.
 */
function panels_get_content_types() {
  ctools_include('context');
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'content_types');
}

/**
 * Fetch metadata on a specific caching plugin.
 *
 * @param $cache
 *   Name of a panel cache.
 *
 * @return
 *   An array with information about the requested panel cache.
 */
function panels_get_cache($cache) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'cache', $cache);
}

/**
 * Fetch metadata for all context plugins.
 *
 * @return
 *   An array of arrays with information about all available panel caches.
 */
function panels_get_caches() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'cache');
}

/**
 * Get a function from a plugin, if it exists.
 *
 * @param $plugin
 *   The type of plugin
 * @param $which
 *   Either the loaded plugin object (or the same data in array form)
 *   or a string with the name of the desired the specific plugin.
 * @param $function_name
 *   The identifier of the function. For example, 'settings form'.
 *
 * @return
 *   The actual name of the function to call, or NULL if the function
 *   does not exist.
 */
function panels_plugin_get_function($plugin, $which, $function_name) {
  ctools_include('plugins');
  if (is_object($which) || is_array($which)) {
    return ctools_plugin_get_function($which, $function_name);
  }
  else {
    return ctools_plugin_load_function('panels', $plugin, $which, $function_name);
  }

}

// These are placeholders to prevent crashes from the former plugins

class panels_required_context {
  function filter() { }
};

class panels_optional_context extends panels_required_context {};
