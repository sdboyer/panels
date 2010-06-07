// $Id$

(function($) {
  Drupal.PanelsIPE = {
    editors: {},
    bindClickDelete: function(context) {
      $('a.pane-delete:not(.pane-delete-processed)', context)
        .addClass('pane-delete-processed')
        .click(function() {
          if (confirm('Remove this pane?')) {
            $(this).parents('div.panels-ipe-pane').fadeOut(1000, function() {
              $(this).empty().remove();
            });
          }
          return false;
        });
    },
    addPaneMarker: function(context) {
      // Add a class so that the pane content proper can be more easily identified
      // FIXME this currently can't use context, since the parent/child logic seems to get borked. makes it inefficient - fix!
      $('.panels-ipe-pane > div:not(.panels-ipe-handlebar-wrapper,.panels-ipe-processed)')
        .addClass('panels-ipe-proper-pane panels-ipe-processed');
    },
    initEditing: function(context) {
      $(document.body).addClass('panels-ipe');
      var draggable_options = {
        revert: 'invalid',
        helper: 'clone', // required for "flawless" interoperation with sortables
        connectToSortable: ($.ui.version === '1.6') ? ['div.panels-ipe-region'] : 'div.panels-ipe-region',
        appendTo: 'body',
        handle: 'panels-ipe-draghandle',
      };

      // Add a class so that the direct parent container of the panes can be
      // more easily identified
      // $('div.panels-ipe-pane').parent().addClass('panels-ipe-region-innermost');

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
      };

      $('div.panels-ipe-region').sortable(sortable_options);
      // Since the connectWith option only does a one-way hookup, iterate over
      // all sortable regions to connect them with one another.
      $('div.panels-ipe-region').each(function() {
        $(this).sortable('option', 'connectWith', ['div.panels-ipe-region'])
      });
    }
  }

  Drupal.behaviors.PanelsIPE = function(context) {
    Drupal.PanelsIPE.bindClickDelete(context);
    // the below is very sloppy, 100% temporary
    if (!$(document.body).hasClass('panels-ipe')) {
      Drupal.PanelsIPE.initEditing(context);
    }
    Drupal.PanelsIPE.addPaneMarker(context);
    $('div.panels-ipe-startedit:not(panels-ipe-startedit-processed)', context)
      .addClass('panels-ipe-startedit-processed')
      .click(function() {
        var $this = $(this);
        var cache_key = $this.parent().attr('id').split('panels-ipe-control-')[1];
        // TODO _really_ need to reuse ctools ajax code here
        var url = '/panels_ipe/ajax/edit/' + cache_key;
        $.ajax({
          type: "POST",
          url: url,
          global: true,
          success: function(data) {
            $this.parent().fadeOut('normal', function() {
              $this.hide();
              $this.parent().append(data);
              $this.parent().show('normal', function() {
                $this.parent().data(cache_key, new DrupalPanelsIPE(cache_key));
              });
            });
          },
          error: function(xhr) {
            Drupal.CTools.AJAX.handleErrors(xhr, url);
          },
          dataType: 'json'
        });
    });
  }

  /**
   * Base object (class) definition for the Panels In-Place Editor.
   *
   *  A new instance of this object is instanciated whenever an IPE is
   *  initiated, and destroyed when editing is concluded (successfully or not).
   *
   * @param {string} cache_key
   */
  function DrupalPanelsIPE(cache_key) {
    this.key = cache_key;
    this.state = {};

    // Attach this IPE object into the global list
    Drupal.PanelsIPE.editors[cache_key] = this;

  }
})(jQuery);
