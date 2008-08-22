<?php
// $Id$

/**
 * @file includes/common-context.inc
 * Provide API for adding contexts for modules that embed displays.
 */

/**
 * Provide a list of the ways contexts can be embedded.
 *
 * This provides a full list of context types that Panels understands
 * and can let modules utilize.
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
        'form id' => 'panels_common_edit_argument_form',
        'key' => 'arguments', // the key that data will be stored on an object, eg $panel_page
        'sortable' => TRUE,
      ),
      'relationship' => array(
        'title' => t('Relationships'),
        'singular title' => t('relationship'),
        'description' => t('Relationships are contexts that are created from already existing contexts; the add relationship button will only appear once there is another context available. Relationships can load objects based upon how they are related to each other; for example, the author of a node, or a taxonomy term attached to a node, or the vocabulary of a taxonomy term.'),
        'add button' => t('Add relationship'),
        'context function' => 'panels_get_relationship',
        'form id' => 'panels_common_edit_relationship_form',
        'key' => 'relationships',
        'sortable' => FALSE,
      ),
      'context' => array(
        'title' => t('Contexts'),
        'singular title' => t('context'),
        'description' => t('Contexts are embedded directly into the panel; you generally must select an object in the panel. For example, you could select node 5, or the term "animals" or the user "administrator"'),
        'add button' => t('Add context'),
        'context function' => 'panels_get_context',
        'form id' => 'panels_common_edit_context_form',
        'key' => 'contexts',
        'sortable' => FALSE,
      ),
      'requiredcontext' => array(
        'title' => t('Required contexts'),
        'singular title' => t('required context'),
        'description' => t('Required contexts are passed in from some external source, such as a containing panel. If a mini panel has required contexts, it can only appear when that context is available, and therefore will not show up as a standard Drupal block.'),
        'add button' => t('Add required context'),
        'context function' => 'panels_get_context',
        'form id' => 'panels_common_edit_requiredcontext_form',
        'key' => 'requiredcontexts',
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
    '#prefix' => '<div id="panels-arguments-table">',
    '#suffix' => '</div>',
    '#theme' => 'panels_common_context_item_form',
    '#panel_name' => $object->name,
    '#panels_context_type' => 'argument',
    '#panels_context_module' => $module,
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
}

function panels_common_add_context_form($module, &$form, &$form_state, &$form_location, $object) {
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
}

function panels_common_add_required_context_form($module, &$form, &$form_state, &$form_location, $object) {
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

  $choices = array();
  foreach (panels_get_contexts() as $name => $arg) {
    $choices[$name] = $arg['title'];
  }

  asort($choices);

  if (!empty($choices) || !empty($object->contexts)) {
    panels_common_add_item_table('requiredcontext', $form_location, $choices, $object->requiredcontexts);
  }
}

function panels_common_add_relationship_form($module, &$form, &$form_state, &$form_location, $object) {
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

  $available_relationships = panels_get_relevant_relationships(panels_context_load_contexts($object));

  panels_common_add_item_table('relationship', $form_location, $available_relationships, $object->relationships);

}

function panels_common_add_context_js($base) {
  panels_load_include('ajax');
  panels_modal_js_includes();
  // TODO: extract the parts necessary for the modal form into their own css file.
  drupal_add_css(panels_get_path('css/panels_dnd.css'));
}

/**
 * Add the context table to the page.
 */
function panels_common_add_item_table($type, &$form, $available_contexts, $items) {
  $form[$type] = array(
    '#tree' => TRUE,
  );

  $module = $form['#panels_context_module'];
  $name   = $form['#panel_name'];

  if (isset($items) && is_array($items)) {
    foreach ($items as $position => $context) {
      panels_common_add_item_to_form($module, $type, $name, $form[$type][$position], $position, $context);
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
    $type_info = panels_common_context_info($type);

    $module = $form['#panels_context_module'];
    $name   = $form['#panel_name'];

    // The URL for this ajax button
    $form['buttons'][$type]['add-url'] = array(
      '#attributes' => array('class' => "panels-$type-add-url"),
      '#type' => 'hidden',
      '#value' => url("panels/ajax/context-add/$module/$type/$name", array('absolute' => TRUE)),
    );

    // This also will be in the URL.
    $form['buttons'][$type]['item'] = array(
      '#attributes' => array('class' => "panels-$type-add-url"),
      '#type' => 'select',
      '#options' => $available_contexts,
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
function panels_common_add_item_to_form($module, $type, $name, &$form, $position, $item) {
  panels_load_include('ajax');
  // This is the single function way to load any plugin by variable type.
  $info = panels_common_context_data($type, $item['name']);
  $form['title'] = array(
    '#value' => check_plain($item['identifier']),
  );

  // Relationships not sortable.
  $type_info = panels_common_context_info($type);

  if (!empty($type_info['sortable'])) {
    $form['position'] = array(
      '#type' => 'weight',
      '#default_value' => $position,
      '#attributes' => array('class' => 'drag-position'),
    );
  }

  $form['remove'] = array('#value' => panels_ajax_image_button('icon-delete.png', "panels/ajax/context-delete/$module/$type/$name/$position", t('Remove this item.'), 'panels-no-modal'));

  $form['settings'] = array('#value' => panels_ajax_image_button('icon-configure.png', "panels/ajax/context-configure/$module/$type/$name/$position", t('Configure settings for this item.')));
}

/**
 * Theme the form item for the context entry.
 */
function theme_panels_common_context_item_row($type, $form, $position, $count, $with_tr = TRUE) {
  $output = '<td class="title">&nbsp;' . drupal_render($form['title']) . '</td>';
  if (!empty($form['position'])) {
    $output .= '<td class="position">&nbsp;' . drupal_render($form['position']) . '</td>';
  }
  $output .= '<td class="operation">' . drupal_render($form['settings']);
  $output .= drupal_render($form['remove']) . '</td>';

  if ($with_tr) {
    $output = '<tr id="' . $type . '-row-' . $position . '" class="draggable ' . $type . '-row ' . ($count % 2 ? 'even' : 'odd') . '">' . $output . '</tr>';
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
    $count = 0;
    $rows = '';
    foreach (array_keys($form[$type]) as $id) {
      if (!is_numeric($id)) {
        continue;
      }
      $rows .= theme('panels_common_context_item_row', $type, $form[$type][$id], $id, $count++);
    }

    $output .= '<table id="' . $type . '-table">';
    $output .= '<thead>';
    $output .= '<tr>';
    $output .= '<th class="title">' . $type_info['title'] . '</th>';
    if (!empty($type_info['sortable']) && $count) {
      $output .= '<th class="position">' . t('Weight') . '</th>';
    }
    $output .= '<th class="operation">' . t('Operation') . '</th>';
    $output .= '</thead>';
    $output .= '<tbody>';

    $output .= $rows;

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

  if (!empty($type_info['sortable'])) {
    drupal_add_tabledrag($type . '-table', 'order', 'sibling', 'drag-position');
  }

  return $output;
}

// ---------------------------------------------------------------------------
// AJAX forms and stuff.

/**
 * Ajax entry point to add an context
 */
function panels_ajax_context_item_add($module = NULL, $type = NULL, $panel_name = NULL, $name = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');

  if (!$name) {
    return panels_ajax_render();
  }

  // Load stored object from cache.
  if (!($object = panels_cache_get("panel_object:$module", $panel_name))) {
    panels_ajax_render(t('Invalid panel name.'));
  }

  // Get info about what we're adding.
  $info = panels_common_context_data($type, $name);
  if (empty($info)) {
    panels_ajax_render();
  }
  $type_info = panels_common_context_info($type);

  // Create a reference to the place our context lives. Since this is fairly
  // generic, this is the easiest way to get right to the place of the
  // object without knowing precisely what data we're poking at.
  $ref = &$object->{$type_info['key']};

  // Give this item an id, which is really just the nth version
  // of this particular context.
  $id = panels_common_get_arg_id($ref, $name) + 1;

  // Figure out the position for our new context.
  $position = empty($ref) ? 0 : max(array_keys($ref)) + 1;

  // Create the basis for our new context.
  $ref[$position] = array(
    'identifier' => $info['title'] . ($id > 1 ? ' ' . $id : ''),
    'keyword' => panels_common_get_keyword($object, $info['keyword']),
    'id' => $id,
    'name' => $name,
  );

  $contexts = panels_context_load_contexts($object);

  $form_state = array(
    'object' => &$object,
    'module' => $module,
    'type' => $type,
    'name' => $name,
    'ajax' => TRUE,
    'info' => $info,
    'position' => $position,
    'contexts' => $contexts,
    'ref' => &$ref,
    'title' => t('Add @type "@context"', array('@type' => $type_info['singular title'], '@context' => $info['title'])),
  );

  $output = panels_ajax_form_wrapper($type_info['form id'], $form_state);

  if (empty($output)) {
    // successful submit
    panels_cache_set("panel_object:$module", $panel_name, $object);

    $arg_form_state = array();

    $arg_form = array(
      '#post' => array(),
      '#programmed' => FALSE,
      '#tree' => FALSE,
    );

    // Build a chunk of the form to merge into the displayed form
    $arg_form[$type] = array(
      '#tree' => TRUE,
    );
    $arg_form[$type][$position] = array(
      '#tree' => TRUE,
    );

    panels_common_add_item_to_form($module, $type, $panel_name, $arg_form[$type][$position], $position, $ref[$position]);
    $arg_form = form_builder($type_info['form id'], $arg_form, $arg_form_state);

    // Build the relationships table so we can ajax it in.
    // This is an additional thing that goes in here.
    $rel_form = array(
      '#theme' => 'panels_common_context_item_form',
      '#panel_name' => $panel_name,
      '#panels_context_type' => 'relationship',
      '#panels_context_module' => $module,
      '#only_buttons' => TRUE,
      '#post' => array(),
      '#programmed' => FALSE,
      '#tree' => FALSE,
    );

    $rel_form['relationship'] = array(
      '#tree' => TRUE,
    );

    $all_contexts = panels_context_load_contexts($object);
    $available_relationships = panels_get_relevant_relationships($all_contexts);

    $output = new stdClass();
    if (!empty($available_relationships)) {
      panels_common_add_item_table_buttons('relationship', $module, $rel_form, $available_relationships);
      $rel_form = form_builder('dummy_form_id', $rel_form, $arg_form_state);
      $output->replace = array('div#panels-relationships-table div.buttons' => drupal_render($rel_form));
    }

    $output->type     = 'add';
    $output->output   = theme('panels_common_context_item_row', $type, $arg_form[$type][$position], $position, $position);
    $output->region = '#' . $type . '-table tbody';
    $output->id = '#' . $type . '-row-' . $position;
  }

  panels_ajax_render($output);
}

/**
 * Ajax entry point to edit an item
 */
function panels_ajax_context_item_edit($module = NULL, $type = NULL, $panel_name = NULL, $position = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');

  if (!isset($position)) {
    return panels_ajax_render();
  }

  // Load stored object from cache.
  if (!($object = panels_cache_get("panel_object:$module", $panel_name))) {
    panels_ajax_render(t('Invalid panel name.'));
  }

  $type_info = panels_common_context_info($type);

  // Create a reference to the place our context lives. Since this is fairly
  // generic, this is the easiest way to get right to the place of the
  // object without knowing precisely what data we're poking at.
  $ref = &$object->{$type_info['key']};

  $name = $ref[$position]['name'];
  if (empty($name)) {
    panels_ajax_render();
  }

  // load the context
  $info = panels_common_context_data($type, $name);
  if (empty($info)) {
    panels_ajax_render(t('Invalid context type'));
  }

  $contexts = panels_context_load_contexts($object);

  // Remove this context, because we can't really allow circular contexts.
  unset($contexts[panels_context_context_id($ref[$position])]);

  $form_state = array(
    'object' => &$object,
    'module' => $module,
    'type' => $type,
    'name' => $name,
    'ajax' => TRUE,
    'info' => $info,
    'position' => $position,
    'contexts' => $contexts,
    'ref' => &$ref,
    'title' => t('Edit @type "@context"', array('@type' => $type_info['singular title'], '@context' => $info['title'])),
  );

  $output = panels_ajax_form_wrapper($type_info['form id'], $form_state);

  if (empty($output)) {
    // successful submit
    panels_cache_set("panel_object:$module", $panel_name, $object);

    $output = new stdClass();
    $output->type = $output->output = 'dismiss';

    $arg_form = array(
      '#post' => array(),
      '#programmed' => FALSE,
      '#tree' => FALSE,
    );

    // Build a chunk of the form to merge into the displayed form
    $arg_form[$type] = array(
      '#tree' => TRUE,
    );
    $arg_form[$type][$position] = array(
      '#tree' => TRUE,
    );

    panels_common_add_item_to_form($module, $type, $panel_name, $arg_form[$type][$position], $position, $ref[$position]);
    $arg_form = form_builder($type_info['form id'], $arg_form, $arg_form_state);

    $output->replace = array(
      '#' . $type . '-row-' . $position => theme('panels_common_context_item_row', $type, $arg_form[$type][$position], $position, $position)
    );
  }
  panels_ajax_render($output);
}

/**
 * Form (for ajax use) to add a context
 */
function panels_common_edit_context_form(&$form_state) {
  $object = $form_state['object'];
  $context = $form_state['info'];
  $position = $form_state['position'];
  $contexts = $form_state['contexts'];

  $ctext = $object->contexts[$position];

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
    '#type' => 'hidden',
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

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * validate a  context edited/added via ajax
 */
function panels_common_edit_context_form_validate($form, &$form_state) {
  $context = $form_state['info'];

  if (isset($context['settings form validate']) && function_exists($context['settings form validate'])) {
    $context['settings form validate']($form['context']['context_settings'], $form_state['values']['context']['context_settings'], $form_state);
  }
}

/**
 * Updates an context edited/added via ajax
 */
function panels_common_edit_context_form_submit($form, &$form_state) {
  $info = $form_state['info'];

  if (isset($info['settings form submit']) && function_exists($info['settings form submit'])) {
    $info['settings form submit']($form, $form_state['values']['context']['context_settings'], $form_state);
  }

  $context = $form_state['values']['context'];
  $form_state['ref'][$form_state['position']] = $context;
}

/**
 * Form (for ajax use) to add a context
 */
function panels_common_edit_requiredcontext_form(&$form_state) {
  $object = $form_state['object'];
  $context = $form_state['info'];
  $position = $form_state['position'];
  $contexts = $form_state['contexts'];

  $ctext = $object->requiredcontexts[$position];
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
    '#type' => 'hidden',
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
function panels_common_edit_requiredcontext_form_submit($form, &$form_state) {
  $form_state['ref'][$form_state['position']] = $form_state['values']['requiredcontext'];
}

/**
 * Form (for ajax use) to add a relationship
 */
function panels_common_edit_relationship_form(&$form_state) {
  $object = $form_state['object'];
  $relationship = $form_state['info'];
  $position = $form_state['position'];
  $contexts = $form_state['contexts'];

  $rel = $object->relationships[$position];

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
    '#type' => 'hidden',
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

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * validate an relationship edited/added via ajax
 */
function panels_common_edit_relationship_form_validate($form, &$form_state) {
  $relationship = $form_state['info'];

  if (isset($relationship['settings form validate']) && function_exists($relationship['settings form validate'])) {
    $relationship['settings form validate']($form['relationship']['relationship_settings'], $form_state['values']['relationship']['relationship_settings'], $form_state);
  }
}

/**
 * Updates an relationship edited/added via ajax
 */
function panels_common_edit_relationship_form_submit($form, &$form_state) {
  $relationship = $form_state['info'];

  if (isset($relationship['settings form submit']) && function_exists($relationship['settings form submit'])) {
    $relationship['settings form submit']($form, $form_state['values']['relationship_settings'], $form_state);
  }

  $form_state['ref'][$form_state['position']] = $form_state['values']['relationship'];
}

/**
 * Form (for ajax use) to add an argument
 */
function panels_common_edit_argument_form(&$form_state) {
  // Basic values required to orient ourselves
  $object = $form_state['object'];
  $argument = $form_state['info'];
  $position = $form_state['position'];
  $contexts = $form_state['contexts'];

  $arg = $object->arguments[$position];

  if (!isset($arg['default'])) {
    $arg['default'] = 'ignore';
    $arg['title'] = '';
  }

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

  $form['argument']['id'] = array(
    '#type' => 'hidden',
    '#value' => $arg['id'],
  );

  $form['argument']['default'] = array(
    '#type' => 'select',
    '#title' => t('Default'),
    '#options' => array(
      'ignore' => t('Ignore it; content that requires this context will not be available.'),
      '404' => t('Display page not found or display nothing at all.'),
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


  if (isset($argument['settings form']) && function_exists($argument['settings form'])) {
    $form['argument']['argument_settings'] = $argument['settings form']($argument_settings);
  }
  $form['argument']['argument_settings']['#tree'] = TRUE;

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * validate an argument edited/added via ajax
 */
function panels_common_edit_argument_form_validate($form, &$form_state) {
  $argument = $form_state['info'];

  if (isset($argument['settings form validate']) && function_exists($argument['settings form validate'])) {
    $argument['settings form validate']($form, $form_state['values']['argument_settings'], $form_state);
  }
}

/**
 * Updates an argument edited/added via ajax
 */
function panels_common_edit_argument_form_submit($form, &$form_state) {
  $argument = $form_state['info'];

  if (!isset($form_state['values']['argument_settings'])) {
    $form_state['values']['argument_settings'] = array();
  }

  if (isset($argument['settings form submit']) && function_exists($argument['settings form submit'])) {
    $argument['settings form submit']($form, $form_state['values']['argument_settings'], $form_state);
  }

  $form_state['ref'][$form_state['position']] = $form_state['values']['argument'];
}

/**
 * Ajax entry point to edit an item
 */
function panels_ajax_context_item_delete($module = NULL, $type = NULL, $panel_name = NULL, $position = NULL) {
  panels_load_include('plugins');
  panels_load_include('ajax');

  if (!isset($position)) {
    return panels_ajax_render();
  }

  // Load stored object from cache.
  if (!($object = panels_cache_get("panel_object:$module", $panel_name))) {
    panels_ajax_render(t('Invalid panel name.'));
  }

  $type_info = panels_common_context_info($type);

  // Create a reference to the place our context lives. Since this is fairly
  // generic, this is the easiest way to get right to the place of the
  // object without knowing precisely what data we're poking at.
  $ref = &$object->{$type_info['key']};

  if (!array_key_exists($position, $ref)) {
    panels_ajax_render(t('Unable to delete missing item!'));
  }

  unset($ref[$position]);
  panels_cache_set("panel_object:$module", $panel_name, $object);

  $output = new stdClass();
  $output->type = $output->output = 'dismiss';

  $output->replace = array('#' . $type . '-row-' . $position => '');
  $output->exec = "Drupal.Panels.restripeTable('#$type-table');";
  panels_ajax_render($output);
}

// --- End of contexts

function panels_common_save_context($type, &$ref, $form_values) {
  $type_info = panels_common_context_info($type);

  // Organize arguments
  $new = array();
  $order = array();

  foreach ($ref as $id => $context) {
    $position = $form_values[$type][$id]['position'];
    $order[$position] = $id;
  }

  ksort($order);
  foreach ($order as $id) {
    $new[] = $ref[$id];
  }
  $ref = $new;
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
  while (!empty($keywords[$keyword])) {
    $keyword = $word . '_' . ++$count;
  }
  return $keyword;
}

