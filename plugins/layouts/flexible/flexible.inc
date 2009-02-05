<?php
// $Id$

/**
 * Implementation of hook_panels_layouts()
 */
function panels_flexible_panels_layouts() {
  $items['flexible'] = array(
    'title' => t('Flexible'),
    'icon' => 'flexible.png',
    'theme' => 'panels_flexible',
    'css' => 'flexible.css',
    'settings form' => 'panels_flexible_settings_form',
    'settings submit' => 'panels_flexible_settings_submit',
    'settings validate' => 'panels_flexible_settings_validate',
    'panels function' => 'panels_flexible_panels',
  );

  return $items;
}

function panels_flexible_default_panels() {
  return array(
    'percent_width' => 100,
    'rows' => 3,
    'width_type' => '%',
    'row_1' => array(
      'columns' => 1,
      'width_1' => 100,
      'names' => array(t('Top')),
    ),
    'row_2' => array(
      'columns' => 3,
      'width_1' => 25,
      'width_2' => 50,
      'width_3' => 25,
      'names' => array(t('Left'), t('Middle'), t('Right')),
    ),
    // row 3
    'row_3' => array(
      'columns' => 1,
      'width_1' => 100,
      'names' => array(t('Bottom')),
    ),
    'sidebars' => array(
      'left' => FALSE,
      'left_width' => 200,
      'right' => FALSE,
      'right_width' => 200,
      'width_type' => 'px',
    ),
  );
}

function panels_flexible_settings_form($display, $layout, $settings) {
  if (empty($settings)) {
    // default for a new flexible layout
    $settings = panels_flexible_default_panels();
  }

  // Special check for updating:
  if (empty($settings['width_type'])) {
    $settings['width_type'] = '%';
    $settings['percent_width'] = 100;
    $settings['sidebars']['left'] = FALSE;
    $settings['sidebars']['left_width'] = 200;
    $settings['sidebars']['right'] = FALSE;
    $settings['sidebars']['right_width'] = 200;
    $settings['sidebars']['width_type'] = 'px';
  }

  $form['instructions'] = array(
    '#value' => t('<p>Here you may determine the number of rows and columns your layout may have. Each row can have its own number of columns, and each column can have its width set independently. When changing the number of rows or columns, click Save to update the form so you can set the widths for new cells properly.</p><p><strong>Note: Removing cells which contain panes will cause those panes to be disappear. Please move any content you wish to keep.</strong></p>'),
  );

  $form['width_type'] = array(
    '#type' => 'select',
    '#title' => t('Width unit type'),
    '#options' => array('%' => t('% (percentage)'), 'px' => t('px (pixels)'), 'em' => t('em (current)')),
    '#description' => t('The width unit type this layout can have: %, px or em. When using percentage, your layout will be fluid; when using px or em, your layout will be fixed.'),
    '#default_value' => $settings['width_type'],
  );

  $form['percent_width'] = array(
    '#type' => 'textfield',
    '#size' => 2,
    '#width' => 10,
    '#title' => t('Total width'),
    '#description' => t('If using the percentage width, choose the total width that this layout must add up to; if you are having problems with your flexible layout having columns fall off, try lowering this number and adjusting the width of individual columns to match.'),
    '#default_value' => $settings['percent_width'],
  );

  $form['rows'] = array(
    '#type' => 'textfield',
    '#size' => 2,
    '#width' => 10,
    '#title' => t('Rows'),
    '#description' => t('The number of rows this layout can have.'),
    '#default_value' => $settings['rows'],
  );

  for ($row = 1; $row <= intval($settings['rows']); $row++) {
    $form["row_$row"] = array(
      '#type' => 'fieldset',
      '#title' => t('Row @d', array('@d' => $row)),
    );
    $form["row_$row"]["columns"] = array(
      '#prefix' => '<div style="float: left; padding-right: 2em">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#size' => 2,
      '#width' => 10,
      '#title' => t('Columns'),
//      '#description' => t('The number of columns in row @d.', array('@d' => $row)),
      '#default_value' => $settings["row_$row"]["columns"],
    );
    for ($col = 1; $col <= intval($settings["row_$row"]["columns"]); $col++) {
      $form["row_$row"]["width_$col"] = array(
        '#prefix' => '<div style="float: left; padding-right: 2em">',
        '#suffix' => '</div>',
        '#type' => 'textfield',
        '#size' => 2,
        '#width' => 10,
        '#title' => t('Width @d', array('@d' => $col)),
        '#default_value' => $settings["row_$row"]["width_$col"],
      );
    }
    if (is_array($settings["row_$row"]["names"])) {
      $names = implode(', ', $settings["row_$row"]["names"]);
    }
    else {
      $names = '';
    }
    $form["row_$row"]['names'] = array(
      '#prefix' => '<div style="float: left;">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Column titles, separated by commas'),
      '#default_value' => $names,
    );
  }

  $form['sidebars'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sidebars'),
  );

  $form['sidebars']['left_aligner_start'] = array(
    '#value' => '<div class="option-text-aligner">',
    '#tree' => TRUE,
  );
  $form['sidebars']['left'] = array(
    '#type' => 'checkbox',
    '#id' => 'sidebar-left-checkbox',
    '#title' => t('Sidebar left'),
    '#default_value' => $settings['sidebars']['left'],
  );
  $form['sidebars']['left_width'] = array(
    '#type' => 'textfield',
    '#id' => 'sidebar-left-width',
    '#size' => 2,
    '#width' => 10,
    '#default_value' => $settings['sidebars']['left_width'],
  );
  $form['sidebars']['right'] = array(
    '#type' => 'checkbox',
    '#id' => 'sidebar-right-checkbox',
    '#title' => t('Sidebar right'),
    '#default_value' => $settings['sidebars']['right'],
  );

  $form['sidebars']['right_width'] = array(
    '#type' => 'textfield',
    '#id' => 'sidebar-right-width',
    '#size' => 2,
    '#width' => 10,
    '#default_value' => $settings['sidebars']['right_width'],
  );
  $form['sidebars']['left_aligner_stop'] = array(
    '#value' => '</div><div style="clear: both; padding: 0; margin: 0"></div>',
  );
  $form['sidebars']['left_title_markup'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => t('If a sidebar is selected, enter the width of the sidebar.'),
  );

  $form['sidebars']['width_type'] = array(
    '#type' => 'select',
    '#title' => t('Width unit type'),
    '#options' => array('%' => t('% (percentage)'), 'px' => t('px (pixels)'), 'em' => t('em (current)')),
    '#description' => t('The width unit type activated sidebars will have: %, px or em. When using percentage, your sidebars will be fluid; when using px or em, your sidebars will be fixed.'),
    '#default_value' => $settings['sidebars']['width_type'],
  );

  $js_settings = array('panels' => array('checkboxes' => array(
    '#sidebar-left-checkbox' => array('#sidebar-left-width'),
    '#sidebar-right-checkbox' => array('#sidebar-right-width'),
  )));
  // not needed, automatically happens on the layout settings page.
//  drupal_add_js(panels_get_path('js/checkboxes.js'));
  drupal_add_js($js_settings, 'setting');
  return $form;
}

function panels_flexible_settings_validate($values, $form, $display, $layout, $settings) {
  if (empty($settings)) {
    $settings = panels_flexible_default_panels();
  }

  if ($values['rows'] < 1) {
    form_error($form['rows'], t('Rows must be a positive integer.'));
    return;
  }

  // Validate that percentages add up to the stated maximum.
  if ($values['width_type'] == '%') {
    for ($row = 1; $row <= intval($values['rows']); $row++) {
      // This takes into account whether or even had a previous setting here.
      if ($settings['rows'] >= $row) {
        if ($values["row_$row"]['columns'] < 1) {
          form_error($form["row_$row"]['columns'], t('Columns must be a positive integer.'));
          return;
        }
        $total = 0;
        for ($col = 1; $col <= intval($values["row_$row"]["columns"]); $col++) {
          $total += $values["row_$row"]["width_$col"];
        }
        if ($total != $values['percent_width']) {
          form_error($form["row_$row"]['columns'], t('Column widths must add up to @count.', array('@count' => $values['percent_width'])));
        }
      }
    }
  }
}

function panels_flexible_settings_submit(&$values, $display, $layout, $settings) {
  if (empty($settings)) {
    $settings = panels_flexible_default_panels();
  }

  for ($row = 1; $row <= $values['rows']; $row++) {
    if ($row > $settings['rows'] && empty($values["row_$row"]['columns'])) {
      $values["row_$row"]['columns'] = 1;
      $values["row_$row"]['width_1'] = 100;
    }
    if (!empty($values["row_$row"]['names'])) {
      $names = explode(',', $values["row_$row"]['names']);
      foreach ($names as $nid => $name) {
        $names[$nid] = trim($name);
      }
      $values["row_$row"]['names'] = $names;
    }
  }
}

/**
 * Define the actual list of columns and rows for this flexible panel.
 */
function panels_flexible_panels($display, $settings) {
  $panels = array();
  if (empty($settings)) {
    $settings = panels_flexible_default_panels();
  }

  if (!empty($settings['sidebars']['left'])) {
    $panels['sidebar_left'] = t('Left sidebar');
  }

  if (!empty($settings['sidebars']['right'])) {
    $panels['sidebar_right'] = t('Right sidebar');
  }

  for ($row = 1; $row <= intval($settings['rows']); $row++) {
    for ($col = 1; $col <= intval($settings["row_$row"]['columns']); $col++) {
      if (!empty($settings["row_$row"]['names'][$col - 1])) {
        $panels["row_${row}_$col"] = $settings["row_$row"]['names'][$col - 1];
      }
      else {
        $panels["row_${row}_$col"] = t("Row @row, Column @col", array('@row' => $row, '@col' => $col));
      }
    }
  }
  return $panels;
}

/**
 * Theme the flexible layout.
 */
function theme_panels_flexible($id, $content, $settings) {
  if (empty($settings)) {
    $settings = panels_flexible_default_panels();
  }

  ctools_include('css');
  // Generate an id based upon rows + columns:
  $css_id = 'flexible:' . $id . ':';
  for ($row = 1; $row <= intval($settings['rows']); $row++) {
    $css_id .= 'r' . $row;
    for ($col = 1; $col <= intval($settings["row_$row"]['columns']); $col++) {
      $css_id .= 'c' . $settings["row_$row"]["width_$col"];
    }
  }

  $filename = ctools_css_retrieve($css_id);
  if (!$filename) {
    $filename = ctools_css_store($css_id, panels_flexible_get_css($id, $settings), FALSE);
  }

  drupal_add_css($filename, 'module', 'all', FALSE);

  // Special check for updating.
  if (empty($settings['width_type'])) {
    $settings['width_type'] = '%';
    $settings['percent_width'] = 100;
  }

  if ($id) {
    $idstr = " id='$id'";
  }
  else {
    $idstr = '';
  }

  $output = '';

  for ($row = 1; $row <= intval($settings['rows']); $row++) {
    $output .= "<div class=\"panel-row panel-row-$row clear-block\">\n";
    for ($col = 1; $col <= intval($settings["row_$row"]["columns"]); $col++) {
      // We do a width reduction formula to help IE out a little bit. If width is 100%, we take 1%
      // off the total; by dividing by the # of columns, that gets us the reduction overall.
      $reduce = 0;
      if ($settings['width_type'] == '%' && $settings['percent_width'] == 100) {
        $reduce = 1 / $settings["row_$row"]["columns"];
      }
      if ($col == 1) {
        if (intval($settings["row_$row"]["columns"]) == 1) {
          $class = 'panel-col-only';
        }
        else {
          $class = 'panel-col-first';
        }
      }
      elseif ($col == intval($settings["row_$row"]["columns"])) {
        $class = 'panel-col-last';
      }
      else {
        $class = 'panel-col-inside';
      }
      $output .= "<div class=\"panel-col panel-col-$col $class\">\n";
      $output .= "<div class=\"inside\">" . $content["row_${row}_$col"] . "</div>\n";
      $output .= "</div>\n"; // panel-col-$col
    }
    $output .= "</div>\n"; // panel-row-$row
  }

  // Add our potential sidebars
  if (!empty($settings['sidebars']['left']) || !empty($settings['sidebars']['right'])) {
    // provide a wrapper if we have a sidebar
    $output = "<div class=\"panel-sidebar-middle panel-sidebar\">\n$output</div>\n";
  }

  if (!empty($settings['sidebars']['left'])) {
    $size = intval($settings['sidebars']['left_width']) . $settings['sidebars']['width_type'];
    $output = "<div class=\"panel-sidebar panel-sidebar-left panel-col panel-col-first\"><div class=\"inside\">\n" . $content["sidebar_left"] . "</div>\n</div>\n" . $output;
  }

  if (!empty($settings['sidebars']['right'])) {
    $size = intval($settings['sidebars']['right_width']) . $settings['sidebars']['width_type'];
    $output .= "<div class=\"panel-sidebar panel-sidebar-right panel-col panel-col-last\"><div class=\"inside\">\n" . $content["sidebar_right"] . "</div>\n</div>\n";
  }

  // Wrap the whole thing up nice and snug
  $output = "<div class=\"panel-flexible clear-block\" $idstr>\n<div class=\"panel-flexible-sidebars\">\n" . $output . "</div>\n</div>\n";
  return $output;
}

/**
 * Theme the flexible layout.
 */
function panels_flexible_get_css($id, $settings) {
  if (empty($settings)) {
    $settings = panels_flexible_default_panels();
  }

  // Special check for updating.
  if (empty($settings['width_type'])) {
    $settings['width_type'] = '%';
    $settings['percent_width'] = 100;
  }

  if ($id) {
    $idcss = "#$id";
  }
  else {
    $idcss = "div.panel-flexible";
  }

  $css = '';

  for ($row = 1; $row <= intval($settings['rows']); $row++) {
    for ($col = 1; $col <= intval($settings["row_$row"]["columns"]); $col++) {
      // We do a width reduction formula to help IE out a little bit. If width is 100%, we take 1%
      // off the total; by dividing by the # of columns, that gets us the reduction overall.
      $reduce = 0;
      if ($settings['width_type'] == '%' && $settings['percent_width'] == 100) {
        $reduce = 1 / $settings["row_$row"]["columns"];
      }
      $css .= "$idcss div.panel-row-$row div.panel-col-$col { width: " . ((intval($settings["row_$row"]["width_$col"])) - $reduce) . $settings["width_type"] ."; }\n";
    }
  }

  // Add our potential sidebars
  if (!empty($settings['sidebars']['left']) || !empty($settings['sidebars']['right'])) {
    // provide a wrapper if we have a sidebar
    if ($settings['sidebars']['width_type'] == '%') {
      $css .= "$idcss div.panel-flexible-sidebars div.panel-sidebar-middle { width: " . (intval($settings['percent_width']) - intval($settings['sidebars']['left_width']) - intval($settings['sidebars']['right_width'])) . "; }\n";
    }
  }

  if (!empty($settings['sidebars']['left'])) {
    $size = intval($settings['sidebars']['left_width']) . $settings['sidebars']['width_type'];
    $css .= "$idcss div.panel-flexible-sidebars div.panel-sidebar-left { width: $size; margin-left: -$size; }\n";
    $css .= "$idcss div.panel-flexible-sidebars { padding-left: $size; }\n";
    // IE hack
    $css .= "* html $idcss div.panel-flexible-sidebars div.panel-sidebar-left { left: $size; }\n";
  }

  if (!empty($settings['sidebars']['right'])) {
    $size = intval($settings['sidebars']['right_width']) . $settings['sidebars']['width_type'];
    $css .= "$idcss div.panel-flexible-sidebars div.panel-sidebar-right { width: $size; margin-right: -$size; }\n";
    $css .= "$idcss div.panel-flexible-sidebars { padding-right: $size; }\n";
  }

  return $css;
}
