<?php
// $Id$


/**
 * @file
 * Functions used by more than one panels client module.
 */

/**
 * Class definition for the allowed layouts governing structure.
 *
 * @ingroup mainapi
 *
 * This class is designed to handle panels allowed layouts data from start to finish, and sees
 * action at two times:\n
 *    - When a client module wants to generate a form allowing an admin to create or edit a set
 *      of allowed layouts. In this case, either a new panels_allowed_layouts object is created
 *      or one is retrieved from storage and panels_allowed_layouts::set_allowed() is called to
 *      generate the allowed layouts form. \n
 *    - When a client module is calling panels_edit_layout(), a saved instantiation of this object
 *      can be called up and passed in to the fourth parameter, and only the allowed layouts saved
 *      in that object will be displayed on the form. \n
 * Because the panels API does not impose a data structure on the allowed_layouts data, client
 * modules can create as many of these objects as they want, and organize them around any concept:
 * node types, date published, author roles...anything.
 *
 * To call the settings form, instantiate this class - or, if your client module's needs are
 * heavy-duty, extend this class and instantiate your subclass - assign values to any relevant
 * desired members, and call panels_allowed_layouts::set_allowed(). See the documentation on
 * that method for a sample implementation.
 *
 * Note that when unserializing saved tokens of this class, you must
 * run panels_load_include('common') before unserializing in order to ensure
 * that the object is properly loaded.
 *
 * Client modules extending this class should implement a save() method and use it for
 * their custom data storage routine. You'll need to rewrite other class methods if
 * you choose to go another route.
 *
 * @see panels_edit_layout()
 * @see _panels_edit_layout()
 *
 */
class panels_allowed_layouts {

  /**
   *  Specifies whether newly-added layouts (as in, new .inc files) should be automatically
   *  allowed (TRUE) or disallowed (FALSE) for $this. Defaults to TRUE, which is more
   *  permissive but less of an administrative hassle if/when you add new layouts. Note
   *  that this parameter will be derived from $allowed_layouts if a value is passed in.
   */
  var $allow_new = TRUE;

  /**
   *  Optional member. If provided, the Panels API will generate a drupal variable using
   *  variable_set($module_name . 'allowed_layouts', serialize($this)), thereby handling the
   *  storage of this object entirely within the Panels API. This object will be
   *  called and rebuilt by panels_edit_layout() if the same $module_name string is passed in
   *  for the $allowed_types parameter. \n
   *  This is primarily intended for convenience - client modules doing heavy-duty implementations
   *  of the Panels API will probably want to create their own storage method.
   * @see panels_edit_layout()
   */
  var $module_name = NULL;

  /**
   *  An associative array of all available layouts, keyed by layout name (as defined
   *  in the corresponding layout plugin definition), with value = 1 if the layout is
   *  allowed, and value = 0 if the layout is not allowed.
   *  Calling array_filter(panels_allowed_layouts::$allowed_layout_settings) will return an associative array
   *  containing only the allowed layouts, and wrapping that in array_keys() will
   *  return an indexed version of that array.
   */
  var $allowed_layout_settings = array();

  /**
   * Hack-imitation of D6's $form_state. Used by the panels_common_set_allowed_types()
   * form to indicate whether the returned value is in its 'render', 'failed-validate',
   * or 'submit' stage.
   */
  var $form_state;

  /**
   * Constructor function; loads the $allowed_layout_settings array with initial values according
   * to $start_allowed
   *
   * @param bool $start_allowed
   *  $start_allowed determines whether all available layouts will be marked
   *  as allowed or not allowed on the initial call to panels_allowed_layouts::set_allowed()
   *
   */
  function panels_allowed_layouts($start_allowed = TRUE) {
    // TODO would be nice if there was a way to just fetch the names easily
    foreach ($this->list_layouts() as $layout_name) {
      $this->allowed_layout_settings[$layout_name] = $start_allowed ? 1 : 0;
    }
  }

  /**
   * Manage panels_common_set_allowed_layouts(), the FAPI code for selecting allowed layouts.
   *
   * MAKE SURE to set panels_allowed_layouts::allow_new before calling this method. If you want the panels API
   * to handle saving these allowed layout settings, panels_allowed_layouts::module_name must also be set.
   *
   * Below is a sample implementation; refer to the rest of the class documentation to understand all the
   * specific pieces. Values that are intended to be replaced are wrapped with <>.
   *
   * \n @code
   *  function docdemo_allowed_layouts() {
   *    panels_load_include('common');
   *    if (!is_a($allowed_layouts = unserialize(variable_get('panels_common_allowed_layouts', serialize(''))), 'panels_allowed_layouts')) {
   *     $allowed_layouts = new panels_allowed_layouts();
   *      $allowed_layouts->allow_new = TRUE;
   *      $allowed_layouts->module_name = '<client_module_name>';
   *    }
   *    $result = $allowed_layouts->set_allowed('<Desired client module form title>');
   *    if (in_array($allowed_layouts->form_state, array('failed-validate', 'render'))) {
   *     return $result;
   *    }
   *    elseif ($allowed_layouts->form_state == 'submit') {
   *      drupal_goto('</path/to/desired/redirect>');
   *    }
   *  }
   * @endcode \n
   *
   * If $allowed_layouts->form_state == 'failed-validate' || 'render', then you'll need to return
   * $result as it contains the structured form HTML generated by drupal_render_form() and is ready
   * to be passed through index.php's call to theme('page', ...).
   *
   * However, if $allowed_layouts->form_state == 'submit', then the form has been submitted and we should
   * react. It's really up to your client module how you handle the rest; panels_allowed_layouts::save() (or
   * panels_allowed_layouts::api_save(), if that's the route you're going) will have already been called,
   * so if those methods handle your save routine, then all there is left to do is handle redirects, if you
   * want. The current implementation of the allowed layouts form currently never redirects, so it's up to
   * you to control where the user ends up next.
   *
   * @param string $title
   *  Used to set the title of the allowed layouts form. If no value is given, defaults to
   *  'Panels: Allowed Layouts'.
   *
   * @return mixed $result
   *  - On the first passthrough when the form is being rendered, $result is the form's structured
   *    HTML, ready to be pushed to the screen with a call to theme('page', ...).
   *  - A successful second passthrough indicates a successful submit, and
   *    $result === panels_allowed_layouts::allowed_layout_settings. Returning it is simply for convenience.
   */
  function set_allowed($title = 'Panels: Allowed Layouts') {
    $this->sync_with_available();
    $form_id = 'panels_common_set_allowed_layouts';
    $form = drupal_retrieve_form($form_id, $this, $title);

    if ($result = drupal_process_form($form_id, $form)) {
      // successful submit
      $this->form_state = 'submit';
      return $result;
    }
    $this->form_state = isset($_POST['op']) ? 'failed-validate' : 'render';
    $result = drupal_render_form($form_id, $form);
    return $result;
  }

  /**
   * Checks for newly-added layouts and deleted layouts. If any are found, updates panels_allowed_layouts::allowed_layout_settings;
   * new additions are made according to panels_allowed_layouts::allow_new, while deletions are unset().
   *
   * Note that any changes made by this function are not saved in any permanent location.
   */
  function sync_with_available() {
    $layouts = $this->list_layouts();
    foreach (array_diff($layouts, array_keys($this->allowed_layout_settings)) as $new_layout) {
      $this->allowed_layout_settings[$new_layout] = $this->allow_new ? 1 : 0;
    }
    foreach (array_diff(array_keys($this->allowed_layout_settings), $layouts) as $deleted_layout) {
      unset($this->allowed_layout_settings[$deleted_layout]);
    }
  }

  /**
   * Use panels_allowed_layouts::module_name to generate a variable for variable_set(), in which
   * a serialized version of $this will be stored.
   *
   * Does nothing if panels_allowed_layouts::module_name is not set.
   *
   * IMPORTANT NOTE: if you use variable_get() in a custom client module save() method, you MUST
   * wrap $this in serialize(), then unserialize() what you get from variable_get(). Failure to
   * do so will result in an incomplete object. The following code will work:
   * @code
   *  $allowed_layouts = unserialize(variable_get('your_variable_name', serialize(''));
   * @endcode
   *
   * If you don't serialize the second parameter of variable_get() and the variable name you provide
   * can't be found, an E_STRICT warning will be generated for trying to unserialize an entity
   * that has not been serialized.
   *
   */
  function api_save() {
    if (!is_null($this->module_name)) {
      variable_set($this->module_name . "_allowed_layouts", serialize($this));
    }
  }

  /**
   * Snag a list of the current layouts for internal use.
   *
   * Data is not saved in a class member in order to ensure that it's
   * fresh.
   *
   * @return array $layouts
   *  An indexed array of the system names for all currently available layouts.
   */
  function list_layouts() {
    static $layouts = array();
    if (empty($layouts)) {
      panels_load_include('plugins');
      $layouts = array_keys(panels_get_layouts());
    }
    return $layouts;
  }
}

/**
 * A common settings page for Panels modules, because this code is relevant to
 * any modules that don't already have special requirements.
 */
function panels_common_settings($module_name = 'panels_common') {
  panels_load_include('plugins');
  $content_types = panels_get_content_types();
  $default_types = variable_get($module_name . '_default', NULL);
  if (!isset($default_types)) {
    $default_types = array('block' => TRUE, 'views' => TRUE, 'other' => TRUE);
    $skip = TRUE;
  }

  foreach ($content_types as $id => $info) {
    if (empty($info['single'])) {
      $default_options[$id] = t('New @s', array('@s' => $info['title']));
    }
  }

  $default_options['other'] = t('New content of other types');
  $form['panels_common_default'] = array(
    '#type' => 'checkboxes',
    '#title' => t('New content behavior'),
    '#description' => t('Select the default behavior of new content added to the system. If checked, new content will automatically be immediately available to be added to Panels pages. If not checked, new content will not be available until specifically allowed here.'),
    '#options' => $default_options,
    '#default_value' => array_keys(array_filter($default_types)),
  );

  if ($skip) {
    $form['markup'] = array('#value' => t('<p>Click Submit to be presented with a complete list of available content types set to the defaults you selected.</p>'));
    $form['skip'] = array('#type' => 'value', '#value' => TRUE);
  }
  else {
    // Rebuild the entire list, setting appropriately from defaults. Give
    // each type its own checkboxes set unless it's 'single' in which
    // case it can go into our fake other set.
    $available_content_types = panels_get_all_content_types();
    $allowed_content_types = variable_get($module_name . '_allowed_types', array());

    foreach ($available_content_types as $id => $types) {
      foreach ($types as $type => $info) {
        $key = $id . '-' . $type;
        $checkboxes = empty($content_types[$id]['single']) ? $id : 'other';
        $options[$checkboxes][$key] = $info['title'];
        if (!isset($allowed_content_types[$key])) {
          $allowed[$checkboxes][$key] = isset($default_types[$id]) ? $default_types[$id] : $default_types['other'];
        }
        else {
          $allowed[$checkboxes][$key] = $allowed_content_types[$key];
        }
      }
    }

    $form['content_types'] = array('#tree' => TRUE);
    // cheat a bit
    $content_types['other'] = array('title' => t('Other'), 'weight' => 10);
    foreach ($content_types as $id => $info) {
      if (isset($allowed[$id])) {
        $form['content_types'][$id] = array(
          '#prefix' => '<div class="panels-page-type-container">',
          '#suffix' => '</div>',
          '#type' => 'checkboxes',
          '#title' => t('Allowed @s content', array('@s' => $info['title'])),
          '#options' => $options[$id],
          '#default_value' => array_keys(array_filter($allowed[$id])),
        );
      }
    }
  }

  $form['module_name'] = array(
    '#type' => 'value',
    '#value' => $module_name,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  drupal_add_css(panels_get_path('css/panels_page.css'));
  return $form;
}

/**
 * Submit hook for panels_common_settings
 */
function panels_common_settings_submit($form_id, $form_values) {
  $module_name = $form_values['module_name'];
  variable_set($module_name . '_default', $form_values['panels_common_default']);
  if (!$form_values['skip']) {
    // merge the broken apart array neatly back together
    variable_set($module_name . '_allowed_types', call_user_func_array('array_merge', $form_values['content_types']));
  }
  drupal_set_message(t('Your changes have been saved.'));
}

/**
 * Based upon the settings, get the allowed types for this node.
 */
function panels_common_get_allowed_types($module, $contexts = array(), $has_content = FALSE, $default_defaults = array(), $default_allowed_types = array()) {

  // Get a list of all types that are available

  $default_types = variable_get($module . '_defaults', $default_defaults);
  $allowed_types = variable_get($module . '_allowed_types', $default_allowed_types);

  // By default, if they haven't gone and done the initial setup here,
  // let all 'other' types (which will be all types) be available.
  if (!isset($default_types['other'])) {
    $default_types['other'] = TRUE;
  }

  panels_load_include('plugins');
  $content_types = panels_get_available_content_types($contexts, $has_content, $allowed_types, $default_types);

  return $content_types;
}

/**
 * The FAPI code for generating an 'allowed layouts' selection form.
 *
 * NOTE: Because the Panels API does not guarantee a particular method of storing the data on allowed layouts,
 * it is not_possible for the Panels API to implement any checks that determine whether reductions in
 * the set of allowed layouts conflict with pre-existing layout selections. $displays in that category
 * will continue to function with their current layout as normal until the user/owner/admin attempts
 * to change layouts on that display, at which point they will have to select from the new set of
 * allowed layouts. If this is not the desired behavior for your client module, it's up to you to
 * write a validation routine that determines what should be done with conflicting layouts.
 *
 * Remember that changing layouts where panes have already been created can result in data loss;
 * consult panels_change_layout() to see how the Panels API handles that process. Running
 * drupal_execute('panels_change_layout', ...) is one possible starting point.
 *
 * @ingroup forms
 *
 * @param array $allowed_layouts
 *  The set of allowed layouts that should be used as the default values
 *  for this form. If none is provided, then by default no layouts will be restricted.
 * @param string $title
 *  The title that will be used for the form. Defaults to 'Panels: Allowed Layouts' if
 *  no value was provided in panels_allowed_layouts::set_allowed.
 */
// TODO need to add something that handles $finish & $destination-type stuff.
function panels_common_set_allowed_layouts($allowed_layouts, $title) {
  $layouts = panels_get_layouts();
  foreach ($layouts as $id => $layout) {
    $options[$id] = panels_print_layout_icon($id, $layout, check_plain($layout['title']));
  }

  drupal_set_title($title);

  $form['variables'] = array('#type' => 'value', '#value' => array($allowed_layouts));

  drupal_add_js(panels_get_path('js/layout.js'));
  $form['layouts'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select allowed layouts'),
    '#options' => $options,
    '#description' => t('Check the boxes for all layouts you want to allow users choose from when picking a layout. You must allow at least one layout.'),
    '#default_value' => array_keys(array_filter($allowed_layouts->allowed_layout_settings)),
  );

  $form['clearer'] = array(
    // TODO: FIx this to use clear-block instead
    '#value' => '<div style="clear: both;"></div>',
  );
  $form['#redirect'] = FALSE;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['#token'] = FALSE;
  return $form;
}

function panels_common_set_allowed_layouts_validate($form_id, $form_values, $form) {
  $selected = array_filter($form_values['layouts']);
  if (empty($selected)) {
    form_set_error('layouts', 'You must choose at least one layout to allow.');
  }
}

function panels_common_set_allowed_layouts_submit($form_id, $form_values) {
  list($allowed_layouts) = $form_values['variables'];
  foreach ($form_values['layouts'] as $layout => $setting) {
    $allowed_layouts->allowed_layout_settings[$layout] = $setting === 0 ? 0 : 1;
  }
  method_exists($allowed_layouts, 'save') ? $allowed_layouts->save() : $allowed_layouts->api_save();
  return $allowed_layouts->allowed_layout_settings;
}

/**
 * The layout information fieldset displayed at admin/edit/panel-%implementation%/add/%layout%.
 */
function panels_common_get_layout_information($panel_implementation, $contexts = array()) {
  $form = array();
  panels_load_include('plugins');
  $layout = panels_get_layout($panel_implementation->display->layout);

  $form = array(
    '#type' => 'fieldset',
    '#title' => t('Layout'),
  );

  $form['layout-icon'] = array(
    '#value' => panels_print_layout_icon($panel_implementation->display->layout, $layout),
  );

  $form['layout-display'] = array(
    '#value' => check_plain($layout['title']),
  );
  $content = '<dl class="content-list">';

  foreach (panels_get_panels($layout, $panel_implementation->display) as $panel_id => $title) {
    $content .= "<dt>$title</dt><dd>";
    if ($panel_implementation->display->panels[$panel_id]) {
      $content .= '<ol>';
      foreach ($panel_implementation->display->panels[$panel_id] as $pid) {
        $content .= '<li>'. panels_get_pane_title($panel_implementation->display->content[$pid], $contexts) .'</li>';
      }
      $content .= '</ol>';
    }
    else {
      $content .= t('Empty');
    }
    $content .= '</dd>';
  }
  $content .= '</dl>';

  $form['layout-content'] = array(
    '#value' => $content,
  );

  return $form;
}

// ---------------------------------------------------------------------------
// Context, argument, relationship tools

// ---------------------------------------------------------------------------
// contexts -- highly resemble arguments.
function panels_common_context_info($type = NULL) {
  static $info = NULL;

  // static doesn't work with functions like t().
  if (empty($info)) {
    $info = array(
      'argument' => array(
        'title' => t('Arguments'),
        'singular title' => t('argument'),
        'description' => t("Arguments are parsed from the URL and translated into contexts that may be added to the display via the 'content' tab. These arguments are parsed in the order received, and you may use % in your URL to hold the place of an object; the rest of the arguments will come after the URL. For example, if the URL is node/%/panel and your user visits node/1/panel/foo, the first argument will be 1, and the second argument will be foo."),
        'add button' => t('Add argument'),
        'context function' => 'panels_get_argument',
        'sortable' => TRUE,
      ),
      'relationship' => array(
        'title' => t('Relationships'),
        'singular title' => t('relationship'),
        'description' => t('Relationships are contexts that are created from already existing contexts; the add relationship button will only appear once there is another context available. Relationships can load objects based upon how they are related to each other; for example, the author of a node, or a taxonomy term attached to a node, or the vocabulary of a taxonomy term.'),
        'add button' => t('Add relationship'),
        'context function' => 'panels_get_relationship',
        'sortable' => FALSE,
      ),
      'context' => array(
        'title' => t('Contexts'),
        'singular title' => t('context'),
        'description' => t('Contexts are embedded directly into the panel; you generally must select an object in the panel. For example, you could select node 5, or the term "animals" or the user "administrator"'),
        'add button' => t('Add context'),
        'context function' => 'panels_get_context',
        'sortable' => FALSE,
      ),
      'requiredcontext' => array(
        'title' => t('Required contexts'),
        'singular title' => t('required context'),
        'description' => t('Required contexts are passed in from some external source, such as a containing panel. If a mini panel has required contexts, it can only appear when that context is available, and therefore will not show up as a standard Drupal block.'),
        'add button' => t('Add required context'),
        'context function' => 'panels_get_context',
        'sortable' => TRUE,
      ),
    );
  }

  if ($type === NULL) {
    return $info;
  }

  return $info[$type];
}

function panels_common_context_data($type, $name) {
  $info = panels_common_context_info($type);
  if (function_exists($info['context function'])) {
    return $info['context function']($name);
  }
}

/**
 * Add the argument table plus gadget plus javascript to the form.
 */
function panels_common_add_argument_form($module, &$form, &$form_location, $object) {
  $form_location = array(
    '#theme' => 'panels_common_context_item_form',
    '#panel_name' => $object->name,
    '#panels_context_type' => 'argument',
    '#panels_context_module' => $module,
  );

  $form['arguments'] = array(
    '#type' => 'value',
    '#value' => $object->arguments,
  );

  // Store the order the choices are in so javascript can manipulate it.
  $form['argument_order'] = array(
    '#type' => 'hidden',
    '#id' => 'argument-order',
    '#default_value' => $object->arguments ? implode(',', array_keys($object->arguments)) : '',
  );

  $args = panels_get_arguments();
  $choices = array();
  foreach ($args as $name => $arg) {
    $choices[$name] = $arg['title'];
  }

  asort($choices);

  if (!empty($choices) || !empty($object->arguments)) {
    panels_common_add_item_table('argument', $form_location, $choices, $object->arguments);
  }
  return _panels_common_context_js($object->name, $module, 'argument');
}

function panels_common_add_context_form($module, &$form, &$form_location, $object) {
  $form['contexts'] = array(
    '#type' => 'value',
    '#value' => $object->contexts,
  );

  $form_location = array(
    '#prefix' => '<div id="panels-contexts-table">',
    '#suffix' => '</div>',
    '#theme' => 'panels_common_context_item_form',
    '#panel_name' => $object->name,
    '#panels_context_type' => 'context',
    '#panels_context_module' => $module,
  );

  // Store the order the choices are in so javascript can manipulate it.
  $form_location['markup'] = array(
    '#value' => '&nbsp;',
  );
  $form['context_order'] = array(
    '#type' => 'hidden',
    '#id' => 'context-order',
    '#default_value' => $object->contexts ? implode(',', array_keys($object->contexts)) : '',
  );

  $choices = array();
  foreach (panels_get_contexts() as $name => $arg) {
    if (empty($arg['no ui'])) {
      $choices[$name] = $arg['title'];
    }
  }

  asort($choices);

  if (!empty($choices) || !empty($object->contexts)) {
    panels_common_add_item_table('context', $form_location, $choices, $object->contexts);
  }
  return _panels_common_context_js($object->name, $module, 'context');
}

function panels_common_add_required_context_form($module, &$form, &$form_location, $object) {
  $form['requiredcontexts'] = array(
    '#type' => 'value',
    '#value' => $object->requiredcontexts,
  );

  $form_location = array(
    '#prefix' => '<div id="panels-requiredcontexts-table">',
    '#suffix' => '</div>',
    '#theme' => 'panels_common_context_item_form',
    '#panel_name' => $object->name,
    '#panels_context_type' => 'requiredcontext',
    '#panels_context_module' => $module,
  );

  // Store the order the choices are in so javascript can manipulate it.
  $form_location['markup'] = array(
    '#value' => '&nbsp;',
  );
  $form['requiredcontext_order'] = array(
    '#type' => 'hidden',
    '#id' => 'requiredcontext-order',
    '#default_value' => $object->requiredcontexts ? implode(',', array_keys($object->requiredcontexts)) : '',
  );

  $choices = array();
  foreach (panels_get_contexts() as $name => $arg) {
    $choices[$name] = $arg['title'];
  }

  asort($choices);

  if (!empty($choices) || !empty($object->contexts)) {
    panels_common_add_item_table('requiredcontext', $form_location, $choices, $object->requiredcontexts);
  }
  return _panels_common_context_js($object->name, $module, 'requiredcontext');
}

function panels_common_add_relationship_form($module, &$form, &$form_location, $object) {
  $form['relationships'] = array(
    '#type' => 'value',
    '#value' => $object->relationships,
  );

  $form_location = array(
    '#prefix' => '<div id="panels-relationships-table">',
    '#suffix' => '</div>',
    '#theme' => 'panels_common_context_item_form',
    '#panel_name' => $object->name,
    '#panels_context_type' => 'relationship',
    '#panels_context_module' => $module,
  );

  // Store the order the choices are in so javascript can manipulate it.
  $form_location['markup'] = array(
    '#value' => '&nbsp;',
  );
  $form['relationship_order'] = array(
    '#type' => 'hidden',
    '#id' => 'relationship-order',
    '#default_value' => $object->relationships ? implode(',', array_keys($object->relationships)) : '',
  );

  $available_relationships = panels_get_relevant_relationships(panels_context_load_contexts($object));

//  if (!empty($available_relationships) || !empty($object->relationships)) {
    panels_common_add_item_table('relationship', $form_location, $available_relationships, $object->relationships);
//  }

  return _panels_common_context_js($object->name, $module, 'relationship');
}

function _panels_common_context_js($name, $module, $type) {
  return array($type . '-table' => array(
    // The buttons that do stuff.
    'remove' => "input.$type-remove",
    // The gadget that stores our the order of items.
    'order' => "input#$type-order",
    'up' => "input.$type-up",
    'down' => "input.$type-down",
    'configure' => "input.$type-settings",
    'configure_path' => url("panels/common/ajax/edit/$module/$type/$name", NULL, NULL, TRUE),

    // The button that adds an item
    'add' => "input#edit-buttons-$type-add",
    // Path for ajax on adding an item
    'path' => url("panels/common/ajax/add/$module/$type/$name", NULL, NULL, TRUE),
    // Which items to post when adding
    'post' => array("#edit-buttons-$type-item", "input#edit-buttons-$type-add"),
    // Where to get the id of an item
    'tr' => $type . '-row-',
    'row_class' => "tr.$type-row",
    // Additional HTML to replace on add.
    'replace' => array('div#panels-relationships-table div.buttons' => 'relationships_table'),
  ));
}

function panels_common_add_context_js($base) {
  $settings = array(
    'list' => $base,
    'panels' => array(
      'closeText' => t('Close Window'),
      'closeImage' => theme('image', panels_get_path('images/close.gif'), t('Close window'), t('Close window')),
      'throbber' => theme('image', panels_get_path('images/throbber.gif'), t('Loading...'), t('Loading')),
    ),
  );

  drupal_add_js($settings, 'setting');
  drupal_add_js(panels_get_path('js/list.js'));
  drupal_add_js(panels_get_path('js/lib/dimensions.js'));
  drupal_add_js(panels_get_path('js/lib/mc.js'));
  drupal_add_js('misc/jquery.form.js');
  drupal_add_js(panels_get_path('js/modal_forms.js'));
  drupal_add_css(panels_get_path('css/panels_dnd.css'));
}

/**
 * Add the context table to the page.
 */
function panels_common_add_item_table($type, &$form, $available_contexts, $items) {
  $form[$type] = array(
    '#tree' => TRUE,
  );

  if (isset($items) && is_array($items)) {
    foreach ($items as $position => $context) {
      panels_common_add_item_to_form($type, $form[$type][$position], $position, $context);
    }
  }

  $type_info = panels_common_context_info($type);
  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => $type_info['description'],
  );

  panels_common_add_item_table_buttons($type, $form, $available_contexts);
}

function panels_common_add_item_table_buttons($type, &$form, $available_contexts) {
  $form['buttons'] = array(
    '#tree' => TRUE,
  );

  if (!empty($available_contexts)) {
    $form['buttons'][$type]['item'] = array(
      '#type' => 'select',
      '#options' => $available_contexts,
    );

    $type_info = panels_common_context_info($type);
    $form['buttons'][$type]['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => $type . '-add'),
      '#value' => $type_info['add button'],
    );
  }
}

/**
 * Add a row to the form. Used both in the main form and by
 * the ajax to add an item.
 */
function panels_common_add_item_to_form($type, &$form, $position, $item) {
  // This is the single function way to load any plugin by variable type.
  $info = panels_common_context_data($type, $item['name']);
  $form['title'] = array(
    '#value' => check_plain($item['identifier']),
  );

  // Relationships not sortable.
  $type_info = panels_common_context_info($type);

  if (!empty($type_info['sortable'])) {
    $form['up'] = panels_add_button('go-up.png', t('Up'),
      t('Move this item up'),
      $type . '-up',
      $type . '-up-' . $position
    );

    $form['down'] = panels_add_button('go-down.png', t('Down'),
      t('Move this item down'),
      $type . '-down',
      $type . '-down-' . $position
    );
  }

  $form['remove'] = panels_add_button('icon-delete.png', t('Remove'),
    t('Remove this item'),
    $type . '-remove',
    $type . '-remove-' . $position
  );

  $form['settings'] = panels_add_button('icon-configure.png', t('Configure'),
    t('Configure this item'),
    $type . '-settings',
    $type . '-settings-' . $position
  );
}

/**
 * Theme the form item for the context entry.
 */
function theme_panels_common_context_item_row($type, $form, $position, $count, $with_tr = TRUE) {
  $output = '<td class="title">&nbsp;' . drupal_render($form['title']) . '</td>';
  $output .= '<td class="operation">' . drupal_render($form['settings']);
  $type_info = panels_common_context_info($type);
  if (!empty($type_info['sortable'])) {
    $output .= drupal_render($form['up']) . drupal_render($form['down']);
  }
  $output .= drupal_render($form['remove']) . '</td>';

  if ($with_tr) {
    $output = '<tr id="' . $type . '-row-' . $position . '" class="' . $type . '-row ' . ($count % 2 ? 'even' : 'odd') . '">' . $output . '</tr>';
  }
  return $output;
}

/**
 * Add the contexts form to panel page settings
 */
function theme_panels_common_context_item_form($form) {
  $output = '';
  $type   = $form['#panels_context_type'];
  $module = $form['#panels_context_module'];
  $name   = $form['#panel_name'];

  $type_info = panels_common_context_info($type);

  if (!empty($form[$type]) && empty($form['#only_buttons'])) {
    $output .= '<table id="' . $type . '-table">';
    $output .= '<thead>';
    $output .= '<tr>';
    $output .= '<th class="title">' . $type_info['title'] . '</th>';
    $output .= '<th class="operation operation">' . t('Operation') . '</th>';
    $output .= '</thead>';
    $output .= '<tbody>';

    $count = 0;
    foreach (array_keys($form[$type]) as $id) {
      if (!is_numeric($id)) {
        continue;
      }
      $output .= theme('panels_common_context_item_row', $type, $form[$type][$id], $id, $count++);
    }

    $output .= '</tbody>';
    $output .= '</table>';
  }

  if (!empty($form['buttons'])) {
    // Display the add context item.
    $row   = array();
    $row[] = array('data' => drupal_render($form['buttons'][$type]['item']), 'class' => 'title');
    $row[] = array('data' => drupal_render($form['buttons'][$type]['add']), 'class' => 'add', 'width' => "60%");
    $output .= '<div class="buttons">';
    $output .= theme('table', array(), array($row), array('id' => $type . '-add-table'));
    $output .= '</div>';
  }
  if (!empty($form['description'])) {
    $output .= drupal_render($form['description']);
  }

  return $output;
}

/**
 * Ajax entry point to add an context
 */
function panels_common_ajax_context_item_add($module, $type, $panel_name) {
  $object = panels_cache_get("panel_object:$module", $panel_name);
  if (!$object || !$type) {
    panels_ajax_render();
  }

  // Figure out which context we're adding
  if (isset($_POST['buttons'][$type]['item'])) {
    $name = $_POST['buttons'][$type]['item'];
    // Unset $_POST so fapi doesn't get confused and try to process this
    // as a form.
    unset($_POST);
  }
  else if (isset($_POST[$type]['name'])) {
    $name = $_POST[$type]['name'];
  }

  if (empty($name)) {
    panels_ajax_render();
  }

  $info = panels_common_context_data($type, $name);
  if (empty($info)) {
    panels_ajax_render();
  }

  // Create a reference to the place our context lives.
  $keyword = $type . 's';
  $ref = &$object->$keyword;

  // Give this argument an id, which is really just the nth version
  // of this particular context.
  $id = panels_common_get_arg_id($ref, $name) + 1;

  // Figure out the position for our new context.
  $position = empty($ref) ? 0 : max(array_keys($ref)) + 1;

  // Create the basis for our new context.
  $ref[$position] = array(
    'identifier' => $info['title'] . ($id > 1 ? ' ' . $id : ''),
    'keyword' => panels_common_get_keyword($object, $info['keyword']),
    'id' => $id,
  );

  $contexts = panels_context_load_contexts($object);

  $form_id = 'panels_common_edit_' . $type . '_form';
  $form = drupal_retrieve_form($form_id, $object, $info, $position, $contexts);
  if ($_POST && $_POST['form_id'] == $form_id) {
    $form['#redirect'] = FALSE;
  }

  $retval = drupal_process_form($form_id, $form);
  if ($retval) {
    // successful submit

    // Save changes
    $ref[$position] = $retval;
    panels_cache_set("panel_object:$module", $panel_name, $object);

    // Build a chunk of the form to merge into the displayed form
    $arg_form[$type] = array(
      '#tree' => TRUE,
    );

    panels_common_add_item_to_form($type, $arg_form[$type], $position, $retval);
    $arg_form = form_builder($form_id, $arg_form);

    // Build the relationships table so we can ajax it in.
    // This is an additional thing that goes in here.
    $rel_form = array(
      '#theme' => 'panels_common_context_item_form',
      '#panel_name' => $panel_name,
      '#panels_context_type' => 'relationship',
      '#panels_context_module' => $module,
      '#only_buttons' => TRUE,
    );

    $rel_form['relationship'] = array(
      '#tree' => TRUE,
    );

    $available_relationships = panels_get_relevant_relationships(panels_context_load_contexts($object));

    $output = new stdClass();
    if (!empty($available_relationships)) {
      panels_common_add_item_table_buttons('relationship', $rel_form, $available_relationships);
      $rel_form = form_builder('dummy_form_id', $rel_form);
      $output->relationships_table = drupal_render($rel_form);
    }

    $output->type     = 'add';
    $output->output   = theme('panels_common_context_item_row', $type, $arg_form[$type], $position, $position);
    $output->position = $position;
    panels_ajax_render($output);
  }
  else {
    $type_info = panels_common_context_info($type);
    $title = t('Add @type "@context"', array('@type' => $type_info['singular title'], '@context' => $info['title']));
    $output = theme('status_messages');
    $output .= drupal_render_form($form_id, $form);
    panels_ajax_render($output, $title, url($_GET['q'], NULL, NULL, TRUE));
  }
}

/**
 * Ajax entry point to edit an item
 */
function panels_common_ajax_context_item_edit($module, $type, $panel_name) {
  $object = panels_cache_get("panel_object:$module", $panel_name);
  if (!$object) {
    panels_ajax_render();
  }

  // Figure out which context we're adding
  if (isset($_POST['position'])) {
    $position = $_POST['position'];
  }

  if (!isset($_POST['form_id'])) {
    // Unset $_POST so fapi doesn't get confused and try to process this
    // as a form.
    unset($_POST);
  }

  // Create a reference to the place our context lives.
  $keyword = $type . 's';
  $ref = &$object->$keyword;

  $name = $ref[$position]['name'];
  if (empty($name)) {
    panels_ajax_render();
  }

  // load the context
  $info = panels_common_context_data($type, $name);
  if (empty($info)) {
    panels_ajax_render();
  }

  $type_info = panels_common_context_info($type);
  $title = t('Edit @type "@context"', array('@type' => $type_info['singular title'], '@context' => $info['title']));

  $contexts = panels_context_load_contexts($object);

  // Remove this context, because we can't really allow circular contexts.
  // TODO: FIX THIS!!!
  unset($contexts[panels_context_context_id($ref[$position])]);

  $form_id = 'panels_common_edit_' . $type . '_form';
  $form = drupal_retrieve_form($form_id, $object, $info, $position, $contexts);
  if ($_POST && $_POST['form_id'] == $form_id) {
    // TODO: Make sure the form does this.
    $form['#redirect'] = FALSE;
  }

  $retval = drupal_process_form($form_id, $form);
  if ($retval) {
    $output = new stdClass();
    // successful submit

    // Save changes
    $ref[$position] = $retval;
    panels_cache_set("panel_object:$module", $panel_name, $object);

    $output->type = $output->output = 'dismiss';


    // Build a chunk of the form to merge into the displayed form
    $arg_form[$type] = array(
      '#tree' => TRUE,
    );

    panels_common_add_item_to_form($type, $arg_form[$type], $position, $retval);
    $arg_form = form_builder($form_id, $arg_form);

    $output->replace = theme('panels_common_context_item_row', $type, $arg_form[$type], $position, $position, FALSE);
    $output->replace_id = '#' . $type . '-row-' . $position;

    panels_ajax_render($output);
  }
  else {
    $output = theme('status_messages');
    $output .= drupal_render_form($form_id, $form);
    panels_ajax_render($output, $title, url($_GET['q'], NULL, NULL, TRUE));
  }
}

/**
 * Form (for ajax use) to add a context
 */
function panels_common_edit_context_form($object, $context, $position, $contexts) {
  $ctext = $object->contexts[$position];
  $form['position'] = array(
    '#type' => 'hidden',
    '#value' => $position,
  );

  $form['start_form'] = array('#value' => '<div class="modal-form clear-block">');

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($context['description']),
  );

  // Basic context values
  $form['context']['#tree'] = TRUE;

  $form['context']['name'] = array(
    '#type' => 'hidden',
    '#value' => $context['name'],
  );

  $form['context']['id'] = array(
    '#type' => 'value',
    '#value' => $ctext['id'],
  );

  $form['context']['identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#description' => t('Enter a name to identify this !type on administrative screens.', array('!type' =>t('context'))),
    '#default_value' => $ctext['identifier'],
  );

  $form['context']['keyword'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#description' => t('Enter a keyword to use for substitution in titles.'),
    '#default_value' => $ctext['keyword'],
  );

  // Settings particular to this context
  $context_settings = array();
  if (isset($ctext['context_settings'])) {
    $context_settings = $ctext['context_settings'];
  }

  if (isset($context['settings form']) && function_exists($context['settings form'])) {
    $form['context']['context_settings'] = $context['settings form']($context_settings);
    $form['context']['context_settings']['#tree'] = TRUE;
  }

  $form['context_info'] = array(
    '#type' => 'value',
    '#value' => $context,
  );

  $form['end_form'] = array('#value' => '</div>');

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * validate a  context edited/added via ajax
 */
function panels_common_edit_context_form_validate($form_id, $form_values, $form) {
  $context = $form_values['context_info'];

  if (isset($context['settings form validate']) && function_exists($context['settings form validate'])) {
    $context['settings form validate']($form['context']['context_settings'], $form_values['context']['context_settings']);
  }
}

/**
 * Updates an context edited/added via ajax
 */
function panels_common_edit_context_form_submit($form_id, $form_values) {
  $context = $form_values['context'];
  $info = $form_values['context_info'];

  if (isset($info['settings form submit']) && function_exists($info['settings form submit'])) {
    $info['settings form submit']($form_values['context_settings']);
  }

  return $context;
}

/**
 * Form (for ajax use) to add a context
 */
function panels_common_edit_requiredcontext_form($object, $context, $position, $contexts) {
  $ctext = $object->requiredcontexts[$position];
  $form['position'] = array(
    '#type' => 'hidden',
    '#value' => $position,
  );

  $form['start_form'] = array('#value' => '<div class="modal-form clear-block">');

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($context['description']),
  );

  // Basic context values
  $form['requiredcontext']['#tree'] = TRUE;

  $form['requiredcontext']['name'] = array(
    '#type' => 'hidden',
    '#value' => $context['name'],
  );

  $form['requiredcontext']['id'] = array(
    '#type' => 'value',
    '#value' => $ctext['id'],
  );

  $form['requiredcontext']['identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#description' => t('Enter a name to identify this !type on administrative screens.', array('!type' =>t('required context'))),
    '#default_value' => $ctext['identifier'],
  );

  $form['requiredcontext']['keyword'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#description' => t('Enter a keyword to use for substitution in titles.'),
    '#default_value' => $ctext['keyword'],
  );

  $form['context_info'] = array(
    '#type' => 'value',
    '#value' => $context,
  );

  $form['end_form'] = array('#value' => '</div>');

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Updates a required context edited/added via ajax
 */
function panels_common_edit_requiredcontext_form_submit($form_id, $form_values) {
  $context = $form_values['requiredcontext'];
  return $context;
}

/**
 * Form (for ajax use) to add a relationship
 */
function panels_common_edit_relationship_form($panel_page, $relationship, $position, $contexts) {
  $rel = $panel_page->relationships[$position];
  $form['position'] = array(
    '#type' => 'hidden',
    '#value' => $position,
  );

  $form['start_form'] = array('#value' => '<div class="modal-form clear-block">');

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($relationship['description']),
  );

  // Basic relationship values
  $form['relationship']['#tree'] = TRUE;

  $form['relationship']['context'] = panels_context_selector($contexts, $relationship['required context'], $rel['context']);

  $form['relationship']['name'] = array(
    '#type' => 'hidden',
    '#value' => $relationship['name'],
  );

  $form['relationship']['id'] = array(
    '#type' => 'value',
    '#value' => $rel['id'],
  );

  $form['relationship']['identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#description' => t('Enter a name to identify this !type on administrative screens.', array('!type' =>t('relationship'))),
    '#default_value' => $rel['identifier'],
  );

  $form['relationship']['keyword'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#description' => t('Enter a keyword to use for substitution in titles.'),
    '#default_value' => $rel['keyword'],
  );

  // Settings particular to this relationship
  $relationship_settings = array();
  if (isset($rel['relationship_settings'])) {
    $relationship_settings = $rel['relationship_settings'];
  }

  if (isset($relationship['settings form']) && function_exists($relationship['settings form'])) {
    $form['relationship']['relationship_settings'] = $relationship['settings form']($relationship_settings);
    $form['relationship']['relationship_settings']['#tree'] = TRUE;
  }

  $form['relationship_info'] = array(
    '#type' => 'value',
    '#value' => $relationship,
  );

  $form['end_form'] = array('#value' => '</div>');

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * validate an relationship edited/added via ajax
 */
function panels_common_edit_relationship_form_validate($form_id, $form_values, $form) {
  $relationship = $form_values['relationship_info'];

  if (isset($relationship['settings form validate']) && function_exists($relationship['settings form validate'])) {
    $relationship['settings form validate']($form['relationship']['relationship_settings'], $form_values['relationship']['relationship_settings']);
  }
}

/**
 * Updates an relationship edited/added via ajax
 */
function panels_common_edit_relationship_form_submit($form_id, $form_values) {
  $relationship = $form_values['relationship'];

  if (isset($relationship['settings form submit']) && function_exists($relationship['settings form submit'])) {
    $relationship['settings form submit']($form_values['relationship_settings']);
  }

  return $relationship;
}

/**
 * Form (for ajax use) to add an argument
 */
function panels_common_edit_argument_form($panel_page, $argument, $position) {
  // Basic values required to orient ourselves
  $arg = $panel_page->arguments[$position];

  $form['position'] = array(
    '#type' => 'hidden',
    '#value' => $position,
  );

  $form['start_form'] = array('#value' => '<div class="modal-form">');

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($argument['description']),
  );

  // Basic argument values
  $form['argument']['#tree'] = TRUE;
  $form['argument']['name'] = array(
    '#type' => 'hidden',
    '#value' => $argument['name'],
  );

  $form['argument']['default'] = array(
    '#type' => 'select',
    '#title' => t('Default'),
    '#options' => array(
      'ignore' => t('Ignore it; content that requires this context will not be available.'),
      '404' => t('Display page not found.'),
    ),
    '#default_value' => $arg['default'],
    '#description' => t('If the argument is missing or is not valid, select how this should behave.'),
  );

  $form['argument']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $arg['title'],
    '#description' => t('Enter a title to use when this argument is present. You may use %KEYWORD substitution, where the keyword is specified by the administrator.'),
  );

  $form['argument']['id'] = array(
    '#type' => 'value',
    '#value' => $arg['id'],
  );

  $form['argument']['identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#description' => t('Enter a name to identify this !type on administrative screens.', array('!type' =>t('argument'))),
    '#default_value' => $arg['identifier'],
  );

  $form['argument']['keyword'] = array(
    '#type' => 'textfield',
    '#title' => t('Keyword'),
    '#description' => t('Enter a keyword to use for substitution in titles.'),
    '#default_value' => $arg['keyword'],
  );

  // Settings particular to this argument
  $argument_settings = array();
  if (isset($arg['argument_settings'])) {
    $argument_settings = $arg['argument_settings'];
  }

  $form['arg'] = array(
    '#type' => 'value',
    '#value' => $argument,
  );

  if (isset($argument['settings form']) && function_exists($argument['settings form'])) {
    $form['argument']['argument_settings'] = $argument['settings form']($argument_settings);
    $form['argument']['argument_settings']['#tree'] = TRUE;
  }

  $form['end_form'] = array('#value' => '</div>');

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * validate an argument edited/added via ajax
 */
function panels_common_edit_argument_form_validate($form_id, $form_values, $form) {
  $argument = $form_values['arg'];

  if (isset($argument['settings form validate']) && function_exists($argument['settings form validate'])) {
    $argument['settings form validate']($form['argument']['argument_settings'], $form_values['argument']['argument_settings']);
  }
}

/**
 * Updates an argument edited/added via ajax
 */
function panels_common_edit_argument_form_submit($form_id, $form_values) {
  $argument = $form_values['arg'];
  $position = $form_values['position'];

  if (isset($argument['settings form submit']) && function_exists($argument['settings form submit'])) {
    $argument['settings form submit']($form_values['argument']['argument_settings']);
  }

  // Because we're being all ajaxy, our caller will save this for us.
  return $form_values['argument'];
}

// --- End of contexts

function panels_common_save_context($type, &$ref, $form_values) {
  // Organize arguments
  $ref = array();
  if (isset($form_values[$type . '_order']) && $form_values[$type . '_order'] !== '') {
    foreach (explode(',', $form_values[$type . '_order']) as $position) {
      // We retain the original position here because we need argument IDs.
      $ref[$position] = $form_values[$type . 's'][$position];
    }
  }
}

// ---------------------------------------------------------------------------
// Ajax tools

/**
 * Incoming menu function for ajax calls. This routes to the proper 'module'
 * -- we really only need this because common.inc relies on panels.module for
 * its menu hook, and this way the code stays together.
 */
function panels_common_ajax($module = NULL, $data = NULL, $info = NULL, $info2 = NULL) {
  switch ($module) {
    case 'edit':
      return panels_common_ajax_context_item_edit($data, $info, $info2);

    case 'add':
      return panels_common_ajax_context_item_add($data, $info, $info2);

    case 'panel_settings':
      return panels_common_panel_settings_ajax($data, $info);

    default:
      panels_ajax_render(t('An error occurred'), t('Error'));
  }
}

// TODO: Move this somewhere more appropriate
function panels_common_get_arg_id($arguments, $name) {
  // Figure out which instance of this argument we're creating
  $id = 0;
  foreach ($arguments as $arg) {
    if ($arg['name'] == $name) {
      if ($arg['id'] > $id) {
        $id = $arg['id'];
      }
    }
  }
  return $id;
}

function panels_common_get_keyword($page, $word) {
  // Create a complete set of keywords
  $keywords = array();
  foreach (array('arguments', 'relationships', 'contexts', 'requiredcontexts') as $type) {
    if (!empty($page->$type) && is_array($page->$type)) {
      foreach ($page->$type as $info) {
        $keywords[$info['keyword']] = TRUE;
      }
    }
  }

  $keyword = $word;
  $count = 0;
  while ($keywords[$keyword]) {
    $keyword = $word . '_' . ++$count;
  }
  return $keyword;
}

/**
 * Create a visible list of content in a display.
 * Note that the contexts must be pre-loaded.
 */
function theme_panels_common_content_list($display) {
  $layout = panels_get_layout($display->layout);
  $content = '<dl class="content-list">';
  foreach (panels_get_panels($layout, $display) as $panel_id => $title) {
    $content .= "<dt>$title</dt><dd>";
    if ($display->panels[$panel_id]) {
      $content .= '<ol>';
      foreach ($display->panels[$panel_id] as $pid) {
        $content .= '<li>' . panels_get_pane_title($display->content[$pid], $display->context) . '</li>';
      }
      $content .= '</ol>';
    }
    else {
      $content .= t('Empty');
    }
    $content .= '</dd>';
  }
  $content .= '</dl>';
  return $content;
}

/**
 * Create a visible list of all the contexts available on an object.
 * Assumes arguments, relationships and context objects.
 *
 * Contexts must be preloaded.
 */
function theme_panels_common_context_list($object) {
  $titles = array();
  $output = '';
  $count  = 1;
  // First, make a list of arguments. Arguments are pretty simple.
  if (!empty($object->arguments)) {
    foreach ($object->arguments as $argument) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('Argument @count', array('@count' => $count)) . '</em></td>';
      $output .= '<td>' . check_plain($argument['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_argument_context_id($argument)] = $argument['identifier'];
      $count++;
    }
  }
  $count = 1;
  // Then, make a nice list of contexts.
  if (!empty($object->contexts)) {
    foreach ($object->contexts as $context) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('Context @count', array('@count' => $count)) . '</em></td>';
      $output .= '<td>' . check_plain($context['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_context_context_id($context)] = $context['identifier'];
      $count++;
    }
  }
  // And relationships
  if (!empty($object->relationships)) {
    foreach ($object->relationships as $relationship) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('From @title', array('@title' => $titles[$relationship['context']])) . '</em></td>';
      $output .= '<td>' . check_plain($relationship['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_relationship_context_id($relationship)] = $relationship['identifier'];
      $count++;
    }
  }
  if ($output) {
    return "<table><tbody>$output</tbody></table>\n";
  }
}

/**
 * Create a visible list of content in a display.
 * Note that the contexts must be pre-loaded.
 */
function theme_panels_common_content_list($display) {
  $layout = panels_get_layout($display->layout);
  $content = '<dl class="content-list">';
  foreach (panels_get_panels($layout, $display) as $panel_id => $title) {
    $content .= "<dt>$title</dt><dd>";
    if ($display->panels[$panel_id]) {
      $content .= '<ol>';
      foreach ($display->panels[$panel_id] as $pid) {
        $content .= '<li>' . panels_get_pane_title($display->content[$pid], $display->context) . '</li>';
      }
      $content .= '</ol>';
    }
    else {
      $content .= t('Empty');
    }
    $content .= '</dd>';
  }
  $content .= '</dl>';
  return $content;
}

/**
 * Create a visible list of all the contexts available on an object.
 * Assumes arguments, relationships and context objects.
 *
 * Contexts must be preloaded.
 */
function theme_panels_common_context_list($object) {
  $titles = array();
  $output = '';
  $count = 1;
  // First, make a list of arguments. Arguments are pretty simple.
  if (!empty($object->arguments)) {
    foreach ($object->arguments as $argument) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('Argument @count', array('@count' => $count)) . '</em></td>';
      $output .= '<td>' . check_plain($argument['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_argument_context_id($argument)] = $argument['identifier'];
      $count++;
    }
  }
  $count = 1;
  // Then, make a nice list of contexts.
  if (!empty($object->contexts)) {
    foreach ($object->contexts as $context) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('Context @count', array('@count' => $count)) . '</em></td>';
      $output .= '<td>' . check_plain($context['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_context_context_id($context)] = $context['identifier'];
      $count++;
    }
  }
  // And relationships
  if (!empty($object->relationships)) {
    foreach ($object->relationships as $relationship) {
      $output .= '<tr>';
      $output .= '<td><em>' . t('From @title', array('@title' => $titles[$relationship['context']])) . '</em></td>';
      $output .= '<td>' . check_plain($relationship['identifier']) . '</td>';
      $output .= '</tr>';
      $titles[panels_relationship_context_id($relationship)] = $relationship['identifier'];
      $count++;
    }
  }
  if ($output) {
    return "<table><tbody>$output</tbody></table>\n";
  }
}

/**
 * Theme the form item for the context entry.
 */
function theme_panels_common_context_item_row($type, $form, $position, $count, $with_tr = TRUE) {
  $output = '<td class="title">&nbsp;' . drupal_render($form['title']) . '</td>';

  $arrows = '';
  $type_info = panels_common_context_info($type);
  if (!empty($type_info['sortable'])) {
    $arrows = drupal_render($form['up']) .
              drupal_render($form['down']) .
              drupal_render($form['top']) .
              drupal_render($form['bottom']);
  }

  $output .= '<td class="operation">' . drupal_render($form['settings']) .
    $arrows . drupal_render($form['remove']) .
  '</td>';
  if ($with_tr) {
    $output = '<tr id="' . $type . '-row-' . $position . '" class="' . $type . '-row ' . ($count % 2 ? 'even' : 'odd') . '">' . $output . '</tr>';
  }
  return $output;
}

/**
 * Add the contexts form to panel page settings
 */
function theme_panels_common_context_item_form($form) {
  $output = '';
  $type = $form['#panels_context_type'];
  $module = $form['#panels_context_module'];
  $name = $form['#panel_name'];

  $type_info = panels_common_context_info($type);

  if (!empty($form[$type]) && empty($form['#only_buttons'])) {
    $output .= '<table id="' . $type . '-table">';
    $output .= '<thead>';
    $output .= '<tr>';
    $output .= '<th class="title">' . $type_info['title'] . '</th>';
    $output .= '<th class="operation operation">' . t('Operation') . '</th>';
    $output .= '</thead>';
    $output .= '<tbody>';

    $count = 0;
    foreach (array_keys($form[$type]) as $id) {
      if (!is_numeric($id)) {
        continue;
      }
      $output .= theme('panels_common_context_item_row', $type, $form[$type][$id], $id, $count++);
    }

    $output .= '</tbody>';
    $output .= '</table>';
  }

  if (!empty($form['buttons'])) {
    // Display the add context item.
    $row = array();
    $row[] = array('data' => drupal_render($form['buttons'][$type]['item']), 'class' => 'title');
    $row[] = array('data' => drupal_render($form['buttons'][$type]['add']), 'class' => 'add', 'width' => "60%");
    $output .= '<div class="buttons">';
    $output .= theme('table', array(), array($row), array('id' => $type . '-add-table'));
    $output .= '</div>';
  }
  if (!empty($form['description'])) {
    $output .= drupal_render($form['description']);
  }

  return $output;
}

