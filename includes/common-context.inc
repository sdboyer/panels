<?php
// $Id$

/**
 * @file
 * Provide API for adding contexts for modules that embed displays.
 */

/**
 * Provide a list of the ways contexts can be embedded.
 */
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

/**
 * Get the data belonging to a particular context.
 */
function panels_common_context_data($type, $name) {
  $info = panels_common_context_info($type);
  if (function_exists($info['context function'])) {
    return $info['context function']($name);
  }
}

/**
 * Add the argument table plus gadget plus javascript to the form.
 */
function panels_common_add_argument_form($module, &$form, &$form_state, &$form_location, $object) {
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

function panels_common_add_context_form($module, &$form, &$form_state, &$form_location, $object) {
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

function panels_common_add_required_context_form($module, &$form, &$form_state, &$form_location, $object) {
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

function panels_common_add_relationship_form($module, &$form, &$form_state, &$form_location, $object) {
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
    'configure_path' => url("panels/common/ajax/edit/$module/$type/$name", array('absolute' => TRUE)),

    // The button that adds an item
    'add' => "input#edit-buttons-$type-add",
    // Path for ajax on adding an item
    'path' => url("panels/common/ajax/add/$module/$type/$name", array('absolute' => TRUE)),
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

  panels_common_add_item_table_buttons($type, $module, $form, $available_contexts);
}

function panels_common_add_item_table_buttons($type, $module, &$form, $available_contexts) {
  $form['buttons'] = array(
    '#tree' => TRUE,
  );

  if (!empty($available_contexts)) {
    $form['buttons'][$type]['item'] = array(
      '#type' => 'select',
      '#options' => $available_contexts,
    );

    $type_info = panels_common_context_info($type);

    $module = $form['#panels_context_module'];
    $name   = $form['#panel_name'];

    // The URL for this ajax button
    $form['buttons'][$type]['add-url'] = array(
      '#attributes' => array('class' => "panels-$type-add-url"),
      '#type' => 'hidden',
      '#value' => url("panels/ajax/context-add/$module/$type/$name", array('absolute' => TRUE)),
    );

    $form['buttons'][$type]['add'] = array(
      '#type' => 'submit',
      '#attributes' => array('class' => 'panels-ajax-link'),
      '#id' => "panels-$type-add",
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

  $form['settings'] = panels_ajax_image_button('icon-configure.png', "panels/ajax/configure-context/$type/$position", t('Configure settings for this item.'));
/*
  $form['settings'] = panels_add_button('icon-configure.png', t('Configure'),
    t('Configure this item'),
    $type . '-settings',
    $type . '-settings-' . $position
  );
*/
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
    $output .= drupal_render($form['buttons'][$type]);
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
function panels_ajax_context_item_add($module, $type, $panel_name) {
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
      panels_common_add_item_table_buttons('relationship', $module, $rel_form, $available_relationships);
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
    panels_ajax_render($output, $title, url($_GET['q'], array('absolute' => TRUE)));
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
    panels_ajax_render($output, $title, url($_GET['q'], array('absolute' => TRUE)));
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

