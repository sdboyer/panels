<?php
/* $Id$ */

/*
 * @file
 * CSS filtering functions. Contains a disassembler, filter, compressor, and
 * decompressor. Separated out into this file for cleanliness, as it's likely
 * that these won't live in panels_page for long.
 */

/**
 * Re-assemble a css string and format it nicely.
 *
 * @param array $css_data
 *   An array of css data, as produced by @see panels_page_disassemble_css()
 *   disassembler and the @see panels_page_filter_css() filter.
 * @return string $css
 *   css optimized for human viewing.
 */
function panels_page_assemble_css($css_data) {
  // Initialize the output.
  $css = '';
  // Iterate through all the statements.
  foreach ($css_data as $selector_str => $declaration) {
    // Add the selectors, separating them with commas and line feeds.
    $css .= strpos($selector_str, ',') === FALSE ? $selector_str : preg_replace(", ", ",\n", $selector_str);
    // Add the opening curly brace.
    $css .= " {\n";
    // Iterate through all the declarations.
    foreach ($declaration as $property => $value) {
      $css .= "  " . $property . ": " . $value . ";\n";
    }
    // Add the closing curly brace.
    $css .= "}\n\n";
  }
  // Return the output.
  return $css;
}

/**
 * Compress css data (filter it first!) to optimize for use on view.
 *
 * @param array $css_data
 *   An array of css data, as produced by @see panels_page_disassemble_css()
 *   disassembler and the @see panels_page_filter_css() filter.
 * @return string $css
 *   css optimized for use.
 */
function panels_page_compress_css($css_data) {
  // Initialize the output.
  $css = '';
  // Iterate through all the statements.
  foreach ($css_data as $selector_str => $declaration) {
    if (empty($declaration)) {
      // Skip this statement if filtering removed all parts of the declaration.
      continue;
    }
    // Add the selectors, separating them with commas.
    $css .= $selector_str;
    // And, the opening curly brace.
    $css .= "{";
    // Iterate through all the statement properties.
    foreach ($declaration as $property => $value) {
      $css .= $property . ':' . $value . ';';
    }
    // Add the closing curly brace.
    $css .= "}";
  }
  // Return the output.
  return $css;
}

/**
 * Disassemble the css string.
 *
 * Strip the css of irrelevant characters, invalid/malformed selectors and
 * declarations, and otherwise prepare it for processing.
 *
 * @param string $css
 *   A string containing the css to be disassembled.
 * @return array $disassembled_css
 *   An array of disassembled, slightly cleaned-up/formatted css statements.
 */
function panels_page_disassemble_css($css) {
  $disassembled_css = array();
  // Remove comments.
  $css = preg_replace("/\/\*(.*)?\*\//Usi", "", $css);
  // Split out each statement
  $statements = explode("}", $css);
  // If we have any statements, parse them.
  if (!empty($statements)) {
    // Iterate through all of the statements.
    foreach ($statements as $statement) {
      // Get the selector(s) and declaration.
      list($selector_str, $declaration) = explode('{', $statement);

      // If the selector exists, then disassemble it, check it, and regenerate
      // the selector string.
      $selector_str = empty($selector_str) ? FALSE : panels_page_disassemble_css_selector($selector_str);
      if (empty($selector_str)) {
        // No valid selectors. Bomb out and start the next item.
        continue;
      }

      // Disassemble the declaration, check it and tuck it into an array.
      $disassembled_css[$selector_str] = panels_page_disassemble_css_declaration(strtolower($declaration));
    }
  }
  return $disassembled_css;
}


function panels_page_disassemble_css_selector($selector_str) {
  // Get all selectors individually.
  $selectors = explode(",", trim($selector_str));
  // Iterate through all the selectors, sanity check them and return if they
  // pass. Note that this handles 0, 1, or more valid selectors gracefully.
  foreach ($selectors as $key => $selector) {
    // Replace un-needed characters and do a little cleanup.
    $selector = preg_replace("/[\n|\t|\\|\s]+/", ' ', strtolower(trim($selector)));
    // Make sure this is still a real selector after cleanup.
    if (!empty($selector)) {
      $selectors[$key] = $selector;
    }
    else {
      // Selector is no good, so we scrap it.
      unset ($selectors[$key]);
    }
  }
    // Check for malformed selectors; if found, we skip this declaration.
  if (empty($selectors)) {
    return FALSE;
  }
  return implode(', ', $selectors);
}


function panels_page_disassemble_css_declaration($declaration) {
  $formatted_statement = array();
  $propval_pairs = explode(";", $declaration);
  // Make sure we actually have some properties to work with.
  if (!empty($propval_pairs)) {
    // Iterate through the remains and parse them.
    foreach ($propval_pairs as $key => $propval_pair) {
      // Check that we have a ':', otherwise it's an invalid pair.
      if (strpos($propval_pair, ':') === FALSE) {
        continue;
      }
      // Clean up the current property-value pair.
      $propval_pair =  preg_replace("/[\n|\t|\\|\s]+/", ' ', strtolower(trim($propval_pair)));
      // Explode the remaining fragements some more, but clean them up first.
      list($property, $value) = explode(':', $propval_pair);
      // If the property survived, toss it onto the stack.
      if(!empty($property)) {
        $formatted_statement[trim($property)] = trim($value);
      }
    }
  }
  return $formatted_statement;
}

/**
 *
 */
function panels_page_filter_css($css, $allowed_properties = array(), $allowed_values = array(), $allowed_values_regex = '', $disallowed_values_regex = '') {
//  function panels_page_filter_css($css, &$filtered = NULL, $allowed_properties = array(), $allowed_values = array(), $allowed_values_regex = '', $disallowed_values_regex = '') {
  // Retrieve the default list of allowed properties if none is provided.
  $allowed_properties = !empty($allowed_properties) ? $allowed_properties : panels_page_css_filter_default_allowed_properties();
  // Retrieve the default list of allowed values if none is provided.
  $allowed_values = !empty($allowed_values) ? $allowed_values : panels_page_css_filter_default_allowed_values();
  // Define allowed values regex if none is provided.
  $allowed_values_regex = !empty($allowed_values_regex) ? $allowed_values_regex : '/(#[0-9a-f]+|rgb\(\d+%?,\d*%?,?\d*%?\)?|\d{0,2}\.?\d{0,2}(cm|em|ex|in|mm|pc|pt|px|%|,|\))?)/';
  // Define disallowed url() value contents, if none is provided.
  // $disallowed_values_regex = !empty($disallowed_values_regex) ? $disallowed_values_regex : '/[url|expression]\s*\(\s*[^\s)]+?\s*\)\s*/';
  $disallowed_values_regex = !empty($disallowed_values_regex) ? $disallowed_values_regex : '/(url|expression)/';

  foreach ($css as $selector_str => $declaration) {
    foreach ($declaration as $property => $value) {
      if (!in_array($property, $allowed_properties)) {
        // $filtered['properties'][$selector_str][$property] = $value;
        unset($css[$selector_str][$property]);
        continue;
      }
      $value = str_replace('!important', '', $value);
      if (preg_match($disallowed_values_regex, $value) || !(in_array($value, $allowed_values) || preg_match($allowed_values_regex, $value))) {
        // $filtered['values'][$selector_str][$property] = $value;
        unset($css[$selector_str][$property]);
        continue;
      }
    }
  }
  return $css;
}


function panels_page_css_filter_default_allowed_properties() {
  return array(
    'azimuth',
    'background',
    'background-color',
    'background-image',
    'background-repeat',
    'background-attachment',
    'background-position',
    'border',
    'border-top-width',
    'border-right-width',
    'border-bottom-width',
    'border-left-width',
    'border-width',
    'border-top-color',
    'border-right-color',
    'border-bottom-color',
    'border-left-color',
    'border-color',
    'border-top-style',
    'border-right-style',
    'border-bottom-style',
    'border-left-style',
    'border-style',
    'border-top',
    'border-right',
    'border-bottom',
    'border-left',
    'clear',
    'color',
    'cursor',
    'direction',
    'display',
    'elevation',
    'float',
    'font',
    'font-family',
    'font-size',
    'font-style',
    'font-variant',
    'font-weight',
    'height',
    'letter-spacing',
    'line-height',
    'margin',
    'margin-top',
    'margin-right',
    'margin-bottom',
    'margin-left',
    'overflow',
    'padding',
    'padding-top',
    'padding-right',
    'padding-bottom',
    'padding-left',
    'pause',
    'pause-after',
    'pause-before',
    'pitch',
    'pitch-range',
    'richness',
    'speak',
    'speak-header',
    'speak-numeral',
    'speak-punctuation',
    'speech-rate',
    'stress',
    'text-align',
    'text-decoration',
    'text-indent',
    'unicode-bidi',
    'vertical-align',
    'voice-family',
    'volume',
    'white-space',
    'width',
    'fill',
    'fill-opacity',
    'fill-rule',
    'stroke',
    'stroke-width',
    'stroke-linecap',
    'stroke-linejoin',
    'stroke-opacity',
  );
}

function panels_page_css_filter_default_allowed_values() {
  return array(
    'auto',
    'aqua',
    'black',
    'block',
    'blue',
    'bold',
    'both',
    'bottom',
    'brown',
    'center',
    'collapse',
    'dashed',
    'dotted',
    'fuchsia',
    'gray',
    'green',
    'italic',
    'left',
    'lime',
    'maroon',
    'medium',
    'navy',
    'normal',
    'nowrap',
    'olive',
    'pointer',
    'purple',
    'red',
    'right',
    'solid',
    'silver',
    'teal',
    'top',
    'transparent',
    'underline',
    'white',
    'yellow',
  );
}
