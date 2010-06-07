// $Id$


$(function() {
  var draggable_options = {
    revert: 'invalid',
    helper: 'clone', // required for "flawless" interoperation with sortables
    connectToSortable: ($.ui.version === '1.6') ? ['div.panels-ipe-region'] : 'div.panels-ipe-region',
    appendTo: 'body',
  }

  var sortable_options = {
    revert: true,
    opacity: 0.75, // opacity of sortable while sorting
    // placeholder: 'draggable-placeholder',
    // forcePlaceholderSize: true,
    items: 'div.panels-ipe-pane'
  }

  $('div.panels-ipe-region').sortable(sortable_options);
  // $('div.panels-ipe-pane').draggable(draggable_options);
  // Since the connectWith option only does a one-way hookup, iterate over
  // all sortable regions to connect them with one another.
  $('div.panels-ipe-region').each(function() {
    $(this).sortable('option', 'connectWith', ['div.panels-ipe-region'])
  })
})