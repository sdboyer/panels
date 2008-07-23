<?php
// $Id$


/**
 * @file plugins.inc
 *
 * Contains helper code for plugins and contexts.
 */

/**
 * Forms the basis for describing and storing a context that a display
 * might be running in.
 */
class panels_context {
  var $type = NULL;
  var $data = NULL;
  // The title of this object.
  var $title = '';
  // The title of the page if this object exists
  var $page_title = '';
  // The identifier (in the UI) of this object
  var $identifier = '';
  var $argument = NULL;
  var $keyword = '';

  function panels_context($type = 'none', $data = NULL) {
    $this->type  = $type;
    $this->data  = $data;
    $this->title = t('Unknown context');
  }

  function is_type($type) {
    if ($type == 'any' || $this->type == 'any') {
      return TRUE;
    }

    $a = is_array($type) ? $type : array($type);
    $b = is_array($this->type) ? $this->type : array($this->type);
    return (bool) array_intersect($a, $b);
  }

  function get_argument() {
    return $this->argument;
  }

  function get_keyword() {
    return $this->keyword;
  }

  function get_identifier() {
    return $this->identifier;
  }

  function get_title() {
    return $this->title;
  }

  function get_page_title() {
    return $this->page_title;
  }
}

/**
 * This is used to explain to Panels what context is required.
 */
class panels_required_context {
  var $keywords = '';

  function panels_required_context() {
    $args = func_get_args();
    $this->title = array_shift($args);
    if (count($args) == 1) {
      $args = array_shift($args);
    }
    $this->keywords = $args;
  }

  function filter($contexts) {
    $result = array();

    // See which of these contexts are valid
    foreach ((array) $contexts as $cid => $context) {
      if ($context->is_type($this->keywords)) {
        $result[$cid] = $context;
      }
    }

    return $result;
  }

  function select($contexts, $context) {
    if (empty($context) || empty($contexts[$context])) {
      return FALSE;
    }
    return $contexts[$context];
  }
}

class panels_optional_context extends panels_required_context {
  function panels_optional_context() {
    $args = func_get_args();
    call_user_func_array(array($this, 'panels_required_context'), $args);
  }

  /**
   * Add the 'empty' context which is possible for optional
   */
  function add_empty(&$contexts) {
    $context = new panels_context('any');
    $context->title      = t('No context');
    $context->identifier = t('No context');
    $contexts = array_merge(array('empty' => $context), $contexts);
  }

  function filter($contexts) {
    $this->add_empty($contexts);
    return parent::filter($contexts);
  }

  function select($contexts, $context) {
    $this->add_empty($contexts);
    if (empty($context)) {
      return $contexts['empty'];
    }

    $result = parent::select($contexts, $context);

    // Don't flip out if it can't find the context; this is optional, put
    // in an empty.
    if ($result == FALSE) {
      $result = $contexts['empty'];
    }
    return $result;
  }
}

/**
 * @defgroup panels_content Panels content and pane helper/info functions
 * @{
 */

/**
 * Master pane access function; combines all the relevant parameters that
 * natively used by the Panels API to determine a pane's access. Called from
 * panels_render_panes().
 *
 * @param $pane
 *   The pane object to test for access.
 * @param $display
 *   The display object containing the pane object to be tested.
 */
function panels_pane_access($pane, $display) {
  global $user;

  // Administrator privileges
  if (user_access('view all pane', $user)) {
    return TRUE;
  }

  $role_access = _panels_pane_access_role($pane, $user);
  $type = panels_get_content_type($pane->type);

  if (!$visibility_check = panels_plugin_get_function('content_types', $type, 'visibility check')) {
    return $role_access;
  }
  // Call the content type's custom-defined visibility rendering check.
  // Pass as much possibly relevant data as possible.
  $visibility_access = $visibility_check($pane, $display, $user);
  // Content type marked both access modes to be ANDed together.
  if (!empty($type['roles and visibility'])) {
    return ($visibility_access === TRUE && $role_access === TRUE) ? TRUE : FALSE;
  }
  // Err on the safe side: if EVERYTHING else has failed, then don't render the pane.
  return isset($visibility_access) ? $visibility_access : FALSE;
}

/**
 * Determine role-based access to a panel pane for the current user
 *
 * @param object $pane
 *  The pane object to test.
 * @param object $account
 *  The current $user object.
 *
 * @return bool $role_access
 *  The boolean result of the roles-based segment of the Panels access system.
 */
function _panels_pane_access_role($pane, $account) {
  // All views with an empty access setting are available to all roles.
  if (!$pane->access || !is_array($pane->access)) {
    return TRUE;
  }

  // Otherwise, check roles
  static $roles = array();
  if (!isset($roles[$account->uid])) {
    $roles[$account->uid] = array_keys($account->roles);
    $roles[$account->uid][] = $account->uid ? DRUPAL_AUTHENTICATED_RID : DRUPAL_ANONYMOUS_RID;
  }

  return array_intersect($pane->access, $roles[$account->uid]) ? TRUE : FALSE;
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
function panels_ct_get_content($type, $conf, $args, $context, $incoming_content) {
  if ($function = panels_plugin_get_function('content_types', $type, 'render callback')) {
    $content = $function($conf, $args, $context, $incoming_content);
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
function panels_ct_get_title($type, $conf, $context = NULL, $incoming_content = NULL) {
  if ($function = panels_plugin_get_function('content_types', $type, 'title callback')) {
    return $function($conf, $context, $incoming_content);
  }
  return t('Deleted/missing content type @type', array('@type' => $type));
}

/**
 * Add the default FAPI elements to the content type configuration form
 */
function panels_ct_conf_form($content_type, $subtype, $contexts, $conf) {
  if (!empty($subtype['required context']) && is_array($contexts)) {
    $form['context'] = panels_context_selector($contexts, $subtype['required context'], isset($conf['context']) ? $conf['context'] : array());
  }

  $style_options = array('default' => t('Default'));
  foreach (panels_get_styles() as $name => $properties) {
    if (empty($properties['hidden']) && (!empty($properties['render pane']))) {
      $style_options[$name] = $properties['title'];
    }
  }

  asort($style_options);

  if (empty($conf['style'])) {
    $conf['style'] = 'default';
  }

  if ($style_options) {
    $form['style'] = array(
      '#type' => 'select',
      '#title' => t('Pane style'),
      '#default_value' => $conf['style'],
      '#options' => $style_options,
    );
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

  // Add CSS class and ID gadgets to all panes if advanced pane settings is set.
  if (user_access('administer advanced pane settings')) {
    $form['css_id'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($conf['css_id']) ? $conf['css_id'] : '',
      '#title' => t('CSS ID'),
      '#description' => t('CSS ID to apply to this content. This may be blank.'),
      '#weight' => 2,
      '#size' => 15,
    );
    $form['css_class'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($conf['css_class']) ? $conf['css_class'] : '',
      '#title' => t('CSS class'),
      '#description' => t('CSS class to apply to this content. This may be blank.'),
      '#weight' => 2,
      '#size' => 15,
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
function panels_ct_pane_validate_form($content_type, $form, &$form_values, $op) {
  if ($function = panels_plugin_get_function('content_types', $content_type, "$op validate callback")) {
    return $function($form, $form_values);
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
 * Get all of the individual types provided by a given content type. This
 * would be all of the blocks for the block type, or all of the views for
 * the view type.
 *
 * @param $type
 *   The content type to load.
 */
function panels_ct_get_types($type) {
  if (is_array($type)) {
    $content_type = $type;
  }
  else {
    $content_type = panels_get_content_type($type);
  }

  $function = $content_type['content_types'];
  if (is_array($function)) {
    return (array) $function;
  }

  if (function_exists($function)) {
    return (array) $function();
  }
  return array();
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

  foreach ($content_types as $id => $type) {
    foreach (panels_ct_get_types($type) as $cid => $cinfo) {
      // exclude items that require content if we're saying we don't
      // provide it.
      if (!empty($cinfo['requires content']) && !$has_content) {
        continue;
      }

      // Check to see if the content type can be used in this context.
      if (!empty($cinfo['required context'])) {
        if (!panels_context_filter($contexts, $cinfo['required context'])) {
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
    foreach (panels_ct_get_types($type) as $cid => $cinfo) {
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
  if (!$context) {
    $context = new panels_context;
  }
  // FIXME misplaced bang?
  if (!$incoming_content === '') {
    $incoming_content = t('Incoming content will be displayed here.');
  }

  $content = FALSE;
  $caching = !empty($pane->cache['method']) ? TRUE : FALSE;
  if ($caching && ($cache = panels_get_cached_content($display, $args, $context, $pane))) {
    $content = $cache->content;
  }
  else {
    $content = panels_ct_get_content($pane->type, $pane->configuration, $args, $context, $incoming_content);
    foreach (module_implements('panels_pane_content_alter') as $module) {
      // TODO This makes the third hook invocation on the render path. How badly is this hindering performance?
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
  return panels_ct_get_title($pane->type, $pane->configuration, $pane->context, $incoming_content);
}

/**
 * @} End of "defgroup panels_content".
 */

// ---------------------------------------------------------------------------
// panels argument helpers

/**
 * Get a context from an argument
 */
function panels_argument_get_context($argument, $arg, $empty = FALSE) {
  if ($function = panels_plugin_get_function('arguments', $argument['name'], 'context')) {
    $context = $function($arg, $argument['argument_settings'], $empty);

    if ($context) {
      $context->identifier = $argument['identifier'];
      $context->page_title = $argument['title'];
      $context->keyword    = $argument['keyword'];
      return $context;
    }
  }
}

/**
 * Pick which display an argument wants to use
 */
function panels_argument_choose_display($type, $conf, $context) {
  if ($function = panels_plugin_get_function('arguments', $type, 'choose display')) {
    return $function($conf, $context);
  }
}

/**
 * Determine a unique context ID for an argument
 */
function panels_argument_context_id($argument) {
  return "argument_$argument[name]_$argument[id]";
}

/**
 * Retrieve a list of empty contexts for all arguments
 */
function panels_argument_get_contexts($arguments) {
  $contexts = array();
  foreach ($arguments as $argument) {
    $context = panels_argument_get_context($argument, NULL, true);
    if ($context) {
      $contexts[panels_argument_context_id($argument)] = $context;
    }
  }
  return $contexts;
}

/**
 * Load the contexts for a given panel.
 *
 * @param $arguments
 *   The array of argument definitions
 * @param &$contexts
 *   The array of existing contexts.
 * @param $args
 *   The arguments to load
 *
 * @return
 *   FALSE if an argument wants to 404.
 */
function panels_argument_load_contexts($arguments, &$contexts, $args) {
  foreach ($arguments as $argument) {
    // pull the argument off the list.
    $arg = array_shift($args);
    $id = panels_argument_context_id($argument);

    // For % arguments embedded in the URL, our context is already loaded.
    // There is no need to go and load it again.
    if (empty($contexts[$id])) {
      if ($context = panels_argument_get_context($argument, $arg)) {
        $contexts[$id] = $context;
      }
    }
    else {
      $context = $contexts[$id];
    }

    if ((empty($context) || empty($context->data)) && $argument['default'] == '404') {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Choose a display based upon arguments and loaded contexts.
 *
 * @param $arguments
 *   The array of argument definitions
 * @param $contexts
 *   The array of existing contexts.
 *
 * @return
 *   The identification of the display to use
 */
function panels_argument_get_display($arguments, $contexts) {
  $display_candidate = NULL;
  foreach ($arguments as $i => $argument) {
    $id = panels_argument_context_id($argument);
    if (!empty($contexts[$id]) && !empty($contexts[$id]->data)) {
      $context = $contexts[$id];
      $d = panels_argument_choose_display($argument['name'], $argument['argument_settings'], $context);
      if ($d) {
        $display_candidate = "argument_$i" . '-' . $d;
      }
    }
  }

  return $display_candidate;
}

// ---------------------------------------------------------------------------
// panels relationship helpers

/**
 * Fetch all relevant relationships
 *
 * @param $contexts
 *   An array of contexts used to figure out which relationships are relevant.
 *
 * @return
 *   An array of relationship keys that are relevant for the given set of
 * arguments.
 */
function panels_get_relevant_relationships($contexts) {
  $relevant = array();
  $relationships = panels_get_relationships();

  // Go through each relationship
  foreach ($relationships as $rid => $relationship) {
    // For each relationship, see if there is a context that satisfies it.
    if (panels_context_filter($contexts, $relationship['required context'])) {
      $relevant[$rid] = $relationship['title'];
    }
  }

  return $relevant;
}

/**
 * Fetch all active relationships
 *
 * @param $relationships
 *   An keyed array of relationship data including:
 *   - name: name of relationship
 *   - context: context id relationship belongs to.
 *
 * @param $contexts
 *   A keyed array of contexts used to figure out which relationships
 *   are relevant. New contexts will be added to this.
 *
 */
function panels_relationship_get_contexts($relationships, &$contexts) {
  $return = array();

  foreach ($relationships as $rdata) {
    if (empty($contexts[$rdata['context']])) {
      continue;
    }
    $relationship = panels_get_relationship($rdata['name']);
    // If the relationship can't be found or its context can't be found,
    // ignore.
    if (!$relationship) {
      continue;
    }

    $cid = panels_relationship_context_id($rdata);
    if ($context = panels_relationship_get_context($rdata, $contexts[$rdata['context']])) {
      $contexts[$cid] = $context;
    }
  }
}

/**
 * Determine a unique context ID for an argument
 */
function panels_relationship_context_id($relationship) {
  return "relationship_$relationship[name]_$relationship[id]";
}

/**
 * Fetch a context from a relationship, given the context input.
 */
function panels_relationship_get_context($relationship, $arg) {
  if ($function = panels_plugin_get_function('relationships', $relationship['name'], 'context')) {
    $context = $function($arg, $relationship['relationship_settings']);
    if ($context) {
      $context->identifier = $relationship['identifier'];
      $context->page_title = $relationship['title'];
      $context->keyword    = $relationship['keyword'];
      return $context;
    }
  }
}

// ---------------------------------------------------------------------------
// panels context helpers

/**
 * Return a keyed array of context that match the given 'required context'
 * filters.
 *
 * @param $contexts
 *   A keyed array of all available contexts.
 * @param $required
 *   The required context string or array.
 *
 * @return
 *   A keyed array of contexts.
 */
function panels_context_filter($contexts, $required) {
  if (is_array($required)) {
    $result = array();
    foreach ($required as $r) {
      $result = array_merge($result, _panels_context_filter($contexts, $r));
    }
    return $result;
  }

  return _panels_context_filter($contexts, $required);
}

function _panels_context_filter($contexts, $required) {
  // TODO: Multiples
  $result = array();

  if (is_object($required)) {
    $result = $required->filter($contexts);
  }

  return $result;
}

/**
 * Create a select box to choose possible contexts. This only creates a
 * selector if there is actually a choice.
 *
 * @param $contexts
 *   A keyed array of all available contexts.
 * @param $required
 *   The required context string or array.
 *
 * @return
 *   A form element, or NULL if there are no contexts that satisfy the
 *   requirements.
 */
function panels_context_selector($contexts, $required, $default) {
  if (is_array($required)) {
    $result = array();
    $count = 1;
    foreach ($required as $id => $r) {
      $result[] = _panels_context_selector($contexts, $r, $default[$id], $count++);
    }
    return $result;
  }

  return _panels_context_selector($contexts, $required, $default);
}

function _panels_context_selector($contexts, $required, $default, $num = 0) {
  $filtered = panels_context_filter($contexts, $required);
  $count = count($filtered);

  $form = array();

  if ($count == 1) {
    $keys = array_keys($filtered);
    return array(
      '#type' => 'value',
      '#value' => $keys[0],
    );
  }

  if ($count > 1) {
    // If there's more than one to choose from, create a select widget.
    foreach ($filtered as $cid => $context) {
      $options[$cid] = $context->get_identifier();
    }
    if (!empty($required->title)) {
      $title = $required->title;
    }
    else {
      $title = $num ? t('Context %count', array('%count' => $num)) : t('Context');
    }

    return array(
      '#type' => 'select',
      '#options' => $options,
      '#title' => $title,
      '#description' => t('Multiple contexts are valid for this pane; one must be chosen.'),
      '#default_value' => $default,
    );
  }
}

/**
 * Choose a context based upon the selection made via panels_context_filter
 *
 * @param $contexts
 *   A keyed array of all available contexts
 * @param $required
 *   The required context object provided by the plugin
 * @param $context
 *   The selection made using panels_context_selector
 */
function panels_context_select($contexts, $required, $context) {
  if (is_array($required)) {
    $result = array();
    foreach ($required as $id => $r) {
      if (($result[] = _panels_context_select($contexts, $r, $context[$id])) == FALSE) {
        return FALSE;
      }
    }
    return $result;
  }

  return _panels_context_select($contexts, $required, $context);
}

function _panels_context_select($contexts, $required, $context) {
  if (!is_object($required)) {
    return FALSE;
  }

  return $required->select($contexts, $context);
}

/**
 * Create a new context.
 *
 * @param $type
 *   The type of context to create; this loads a plugin.
 * @param $data
 *   The data to put into the context.
 * @param $empty
 *   Whether or not this context is specifically empty.
 * @param $conf
 *   A configuration structure if this context was created via UI.
 *
 * @return
 *   A $context or NULL if one could not be created.
 */
function panels_context_create($type, $data = NULL, $conf = FALSE) {
  if ($function = panels_plugin_get_function('contexts', $type, 'context')) {
    return $function(FALSE, $data, $conf);
  }
}

function panels_context_create_empty($type) {
  if ($function = panels_plugin_get_function('contexts', $type, 'context')) {
    return $function(TRUE);
  }
}

/**
 * Fetch keywords for use in string substitutions.
 *
 * @param $contexts
 *   An array of contexts.
 *
 * @return
 *   An array of keyword substitutions suitable for @code{strtr()}
 */
function panels_context_get_keywords($contexts) {
  $keywords = array();
  foreach ($contexts as $id => $context) {
    if ($keyword = $context->get_keyword()) {
      $keywords["%$keyword"] = $context->get_title();
    }
  }
  return $keywords;
}

/**
 * Determine a unique context ID for a context
 *
 * @todo can probably combine the various versions of this.
 */
function panels_context_context_id($context, $type = 'context') {
  return $type . "_$context[name]_$context[id]";
}

/**
 * Get a context from a context object.
 */
function panels_context_get_context($context, $type = 'context') {
  if ($function = panels_plugin_get_function('contexts', $context['name'], 'context')) {
    $return = $function($type == 'requiredcontext', $context['context_settings'], TRUE);
    if ($return) {
      $return->identifier = $context['identifier'];
      $return->page_title = $context['title'];
      $return->keyword    = $context['keyword'];
      return $return;
    }
  }
}

/**
 * Retrieve a list of empty contexts for all contexts
 */
function panels_context_get_contexts($contexts, $type = 'context') {
  $return = array();
  foreach ($contexts as $context) {
    $ctext = panels_context_get_context($context, $type);
    if ($ctext) {
      $return[panels_context_context_id($context, $type)] = $ctext;
    }
  }
  return $return;
}

/**
 * Get a group of empty contexts for the object; this assumes all the
 * standard context types, and doesn't punish you if they aren't
 * available.
 */
function panels_context_load_contexts($object, $placeholders = TRUE, $contexts = array()) {
  if ($placeholders) {
    // This will load empty contexts as placeholders for arguments that come
    // from external sources. If this isn't set, it's assumed these context
    // will already have been matched up and loaded.
    if (!empty($object->requiredcontexts) && is_array($object->requiredcontexts)) {
      $contexts += panels_context_get_contexts($object->requiredcontexts, 'requiredcontext');
    }

    if (!empty($object->arguments) && is_array($object->arguments)) {
      $contexts += panels_argument_get_contexts($object->arguments);
    }
  }

  if (!empty($object->contexts) && is_array($object->contexts)) {
    $contexts += panels_context_get_contexts($object->contexts);
  }

  // add contexts from relationships
  if (!empty($object->relationships) && is_array($object->relationships)) {
    panels_relationship_get_contexts($object->relationships, $contexts);
  }

  return $contexts;
}

/**
 * Match up external contexts to our required contexts. It shouldn't be
 * possible to get here without the contexts already lining up properly.
 */
function panels_context_match_required_contexts($required, $contexts) {
  $return = array();
  if (!is_array($required)) {
    return $return;
  }

  foreach ($required as $r) {
    $return[panels_context_context_id($r, 'requiredcontext')] = array_shift($contexts);
  }

  return $return;
}

/**
 * Return the first context with a form id from a list of contexts.
 */
function panels_context_get_form($contexts) {
  foreach ($contexts as $context) {
    if (!empty($context->form_id)) {
      return $context;
    }
  }
}

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

  $content_type = panels_ct_get_types($pane->type);
  // If the pane requires a context, fetch it; if no context is returned,
  // do not display the pane.
  if (empty($content_type[$pane->subtype]['required context'])) {
    return;
  }

  $context = panels_context_select($contexts, $content_type[$pane->subtype]['required context'], $pane->configuration['context']);

  return $context;
}

// ---------------------------------------------------------------------------
// switcher passthrough

/**
 * Switch one object for another just prior to load.
 *
 * @param $plugin
 *   The name or loaded plugin to delegate switching to.
 * @param $type
 *   The object type. eg, 'panels_page', 'panels_mini', etc.
 * @param $name
 *   The name of the object that might be switched.
 * @param $candidates
 *   A list of possible objects to switch to. The data conforms to
 *   what was set in the settings, and is keyed to the panel.
 *
 * @return
 *   The object to switch to. NULL if electing not to switch.
 */
function panels_switcher_switch($plugin, $type, $name, $candidates) {
  $function = panels_plugin_get_function('switchers', $plugin, 'switch');
  if ($function) {
    return $function($type, $name, $candidates);
  }
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
  if (is_object($which) || is_array($which)) {
    $plugin_data = $which;
  }
  else {
    $hook = "panels_$plugin";
    $plugin_data = panels_get_plugins($plugin, $hook, $which);
  }

  if (isset($plugin_data[$function_name]) && function_exists($plugin_data[$function_name])) {
    return $plugin_data[$function_name];
  }
}

// ---------------------------------------------------------------------------
// panels data loading

/**
 * Load plugins from a directory.
 *
 * @param $plugin_type
 *   The plugin type, as well as the panels directory where the plugin is kept.
 *   A list of additional directories to search for relevant plugins is
 *   generated by invoking hook_panels_include_directory();
 * @param $hook
 *   The name of the hook to be invoked.
 * @param $file
 *   The file to load if we're looking for just one particular plugin.
 *
 * @return
 *   An array of information created for this plugin.
 */
function panels_load_includes($plugin_type, $hook, $file = NULL) {
  // Load all our plugins.
  $directories = panels_get_directories($plugin_type);
  $directories['panels_'] = panels_get_path($plugin_type);
  $file_list = array();
  foreach ($directories as $module => $path) {
    $file_list[$module] = drupal_system_listing("$file" . '.inc$', $path, 'name', 0);
  }
  $info = array();
  foreach (array_filter($file_list) as $module => $files) {
    foreach ($files as $file) {
      require_once ('./' . $file->filename);
      $result = _panels_process_plugin('panels', $module . $file->name, dirname($file->filename), $hook);
      if (is_array($result)) {
        $info = array_merge($info, $result);
      }
    }
  }
  return $info;
}

/**
 * Invocation of panels_include_directory(), part of the main panels API.
 *
 * This hook allows other modules to create their own panels .inc files according to the
 * same filename and directory schema utilized by Panels itself, or some other arbitrary
 * schema defined in implementations of the hook. Essentially, instead of having to declare all
 * all callback definitions for a given plugin in a single hook implementation specific to that plugin
 * calling this hook allows the caller to define a directory where Panels will then look for
 * .inc files with callback declarations that conform to the naming scheme. The net result is the
 * same, but implementing this hook does allow for better organization and flexibility in
 * your .inc files, which is particularly preferable if you're implementing a lot of them.
 *
 * PLEASE NOTE: There are strict naming conventions on implementing this hook; failure to
 * follow the conventions will likely cause your plugins not to work. Possibly even worse,
 * failure to follow the conventions can result in namespace collisions between your
 * module and other modules invoking the panels API.
 *
 * @ingroup mainapi
 *
 * @param string $plugintype
 *   The type of Panels plugin being requested. Can be any of the following seven:
 *  'content_types', 'contexts', 'arguments', 'layouts', 'styles', 'relationships'
 *
 * @return array $directories
 *   Returns an array of include subdirectories to call for the requested plugin type.
 *   Subdirectories should be ONLY the subdirectory of your module where the appropriate
 *   types of .inc files reside.
 */
function panels_get_directories($plugintype) {
  $directories = array();
  foreach (module_implements('panels_include_directory') as $module) {
    $result = module_invoke($module, 'panels_include_directory', $plugintype);
    if (isset($result) && is_string($result)) {
      $directories[$module .'_'] = drupal_get_path('module', $module) .'/'. $result;
    }
  }
  return $directories;
}

/**
 * Load plugin info for all hooks; this is handled separately from plugins
 * from files. This is cached so we don't find ourselves building this
 * repeatedly.
 *
 * @param $hook
 *   The hook being invoked.
 *
 * @return
 *   An array of info supplied by any hook implementations.
 */
function panels_load_hooks($hook) {
  $info = array();
  foreach (module_implements($hook) as $module) {
    $result = _panels_process_plugin($module, $module, drupal_get_path('module', $module), $hook);
    if (is_array($result)) {
      $info = array_merge($info, $result);
    }
  }
  return $info;
}

/**
 * Process a single hook implementation of a panels plugin.
 *
 * @param $module
 *   The module that owns the hook.
 * @param $identifier
 *   Either the module or 'panels_' . $file->name
 * @param $hook
 *   The name of the hook being invoked.
 */
function _panels_process_plugin($module, $identifier, $path, $hook) {
  $function = $identifier . '_' . $hook;
  if (!function_exists($function)) {
    return NULL;
  }
  $result = $function();
  if (!isset($result) || !is_array($result)) {
    return NULL;
  }

  // Fill in defaults.
  foreach ($result as $name => $plugin) {
    $result[$name] += array(
      'module' => $module,
      'name' => $name,
      'path' => $path,
    );

    // Add some content type-specific defaults, but allow them to be overridden by declarations in the content type.
    if (preg_match('/content_type/', $hook)) {
      $result[$name] = array_merge(array('single' => FALSE, 'visibility serialize' => FALSE, 'role-based access' => TRUE), $result[$name]);
    }
  }
  return $result;
}

/**
 * Fetch a group of plugins by name.
 *
 * @param $plugin
 *   This is the name of the plugin, and also the name of the directory.
 * @param $hook
 *   This is the hook to call to get the info for the plugin.
 *
 * @return
 *   An array of information arrays about the plugins received.
 */
function panels_get_plugins($plugin, $hook, $id = NULL) {
  static $plugins = array();
  static $all_hooks = array();
  static $all_files = array();

  // Always load all hooks if we need them.
  if (!isset($all_hooks[$plugin])) {
    $all_hooks[$plugin] = TRUE;
    $plugins[$plugin] = panels_load_hooks($hook);
  }

  // First, see if it's in our hooks before we even bother.
  if ($id && array_key_exists($id, $plugins[$plugin])) {
    return $plugins[$plugin][$id];
  }

  // Then see if we should load all files. We only do this if we
  // want a list of all plugins.
  if (!$id && empty($all_files[$plugin])) {
    $all_files[$plugin] = TRUE;
    $plugins[$plugin] = array_merge($plugins[$plugin], panels_load_includes($plugin, $hook));
  }

  // If no id was requested, we are finished here:
  if (!$id) {
    return $plugins[$plugin];
  }

  // Check to see if we need to look for the file
  if (!array_key_exists($id, $plugins[$plugin])) {
    $result = panels_load_includes($plugin, $hook, $id);
    // Set to either what was returned or NULL.
    $plugins[$plugin][$id] = isset($result[$id]) ? $result[$id] : NULL;
  }

  // At this point we should either have the plugin, or a NULL.
  return $plugins[$plugin][$id];
}

/**
 * Fetch a layout plugin
 *
 * @param $layout
 *   Name of a panel layout.
 *
 * @return
 *   An array with information about the requested panel layout.
 */
function panels_get_layout($layout) {
  return panels_get_plugins('layouts', 'panels_layouts', $layout);
}

/**
 * Fetch all layout plugins
 *
 * @return
 *   An array of arrays with information about all available panel layouts.
 */
function panels_get_layouts() {
  return panels_get_plugins('layouts', 'panels_layouts');
}

/**
 * Collate information about a specific panel style.
 *
 * @param $style
 *   Name of a panel style.
 *
 * @return
 *   An array with information about the requested panel style.
 */
function panels_get_style($style) {
  return panels_get_plugins('styles', 'panels_styles', $style);
}

/**
 * Collate information about all available panel styles.
 *
 * @return
 *   An array of arrays with information about all available panel styles.
 */
function panels_get_styles() {
  return panels_get_plugins('styles', 'panels_styles');
}

/**
 * Collate information about a specific panel argument.
 *
 * @param $argument
 *   Name of a panel argument.
 *
 * @return
 *   An array with information about the requested panel argument.
 */
function panels_get_argument($argument) {
  return panels_get_plugins('arguments', 'panels_arguments', $argument);
}

/**
 * Collate information about all available panel arguments.
 *
 * @return
 *   An array of arrays with information about all available panel arguments.
 */
function panels_get_arguments() {
  return panels_get_plugins('arguments', 'panels_arguments');
}

/**
 * Fetch a content_type plugin
 *
 * @param $content type
 *   Name of a panel content type.
 *
 * @return
 *   An array with information about the requested panel content type.
 */
function panels_get_content_type($content_type) {
  return panels_get_plugins('content_types', 'panels_content_types', $content_type);
}

/**
 * Fetch all content type plugins
 *
 * @return
 *   An array of arrays with information about all available panel content types.
 */
function panels_get_content_types() {
  return panels_get_plugins('content_types', 'panels_content_types');
}

/**
 * Fetch a relationship plugin
 *
 * @param $content type
 *   Name of a panel content type.
 *
 * @return
 *   An array with information about the requested relationship
 */
function panels_get_relationship($relationship) {
  return panels_get_plugins('relationships', 'panels_relationships', $relationship);
}

/**
 * Fetch all relationship plugins
 *
 * @return
 *   An array of arrays with information about all available relationships.
 */
function panels_get_relationships() {
  return panels_get_plugins('relationships', 'panels_relationships');
}

/**
 * Fetch a context plugin
 *
 * @param $context
 *   Name of a panel context.
 *
 * @return
 *   An array with information about the requested panel context.
 */
function panels_get_context($context) {
  return panels_get_plugins('contexts', 'panels_contexts', $context);
}

/**
 * Fetch all context plugins
 *
 * @return
 *   An array of arrays with information about all available panel contexts.
 */
function panels_get_contexts() {
  return panels_get_plugins('contexts', 'panels_contexts');
}

/**
 * Fetch a cache plugin
 *
 * @param $cache
 *   Name of a panel cache.
 *
 * @return
 *   An array with information about the requested panel cache.
 */
function panels_get_cache($cache) {
  return panels_get_plugins('cache', 'panels_cache', $cache);
}

/**
 * Fetch all cache plugins
 *
 * @return
 *   An array of arrays with information about all available panel caches.
 */
function panels_get_caches() {
  return panels_get_plugins('cache', 'panels_cache');
}

/**
 * Fetch a switcher plugin
 *
 * @param $switcher
 *   Name of a panel switcher.
 *
 * @return
 *   An array with information about the requested panel switcher.
 */
function panels_get_switcher($switcher) {
  return panels_get_plugins('switchers', 'panels_switchers', $switcher);
}

/**
 * Fetch all switcher plugins
 *
 * @return
 *   An array of arrays with information about all available panel switchers.
 */
function panels_get_switchers() {
  return panels_get_plugins('switchers', 'panels_switchers');
}

