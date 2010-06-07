// $Id$

(function ($) {
  Drupal.PanelsIPE = Drupal.PanelsIPE || {};

  Drupal.PanelsIPE.bindClickDelete = function(context) {
    $('a.pane-delete:not(.pane-delete-processed)', context)
      .addClass('pane-delete-processed')
      .click(function() {
        if (confirm('Remove this pane?')) {
          $(this).parents('div.panels-ipe-pane').remove();
        }
        return false;
      });
  };

  $(function() {
    var draggable_options = {
      revert: 'invalid',
      helper: 'clone', // required for "flawless" interoperation with sortables
      connectToSortable: ($.ui.version === '1.6') ? ['div.panels-ipe-region'] : 'div.panels-ipe-region',
      appendTo: 'body',
      handle: 'panels-ipe-draghandle',
    }

    /**
     * See http://jqueryui.com/demos/sortable/ for details on the configuration
     * parameters used here.
     */
    var sortable_options = {
      revert: true,
      dropOnEmpty: true, // default
      opacity: 0.75, // opacity of sortable while sorting
      // placeholder: 'draggable-placeholder',
      // forcePlaceholderSize: true,
      items: 'div.panels-ipe-pane',
      handle: 'div.panels-ipe-draghandle',
    }

    $('div.panels-ipe-region').sortable(sortable_options);
    // Since the connectWith option only does a one-way hookup, iterate over
    // all sortable regions to connect them with one another.
    $('div.panels-ipe-region').each(function() {
      $(this).sortable('option', 'connectWith', ['div.panels-ipe-region'])
    })
  });

  Drupal.behaviors.PanelsInPlaceEditor = function(context) {
    Drupal.PanelsIPE.bindClickDelete(context);
  }
})(jQuery);
