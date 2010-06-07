// $Id$

(function($) {
  // A ready function should be sufficient for this, at least for now
  $(function() {
    $.each(Drupal.settings.PanelsIPECacheKeys, function() {
      Drupal.PanelsIPE.editors[this] = new DrupalPanelsIPE(this, Drupal.settings.PanelsIPESettings[this]);
    });
  });

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
    }
  }

  Drupal.behaviors.PanelsIPE = function(context) {
    Drupal.PanelsIPE.addPaneMarker(context);
  };

  /**
   * Base object (class) definition for the Panels In-Place Editor.
   *
   *  A new instance of this object is instanciated whenever an IPE is
   *  initiated, and destroyed when editing is concluded (successfully or not).
   *
   * @param {string} cache_key
   */
  function DrupalPanelsIPE(cache_key, cfg) {
    this.key = cache_key;
    this.state = {};
    this.outermost = $('div#panels-ipe-display-'+cache_key);
    this.control = $('div#panels-ipe-control-'+ cache_key);
    this.cfg = cfg;

    /**
     * Passthrough method to be attached to Drupal.behaviors
     *
     * @param {jQuery} context
     */
    this.behaviorsPassthrough = function(context) {
      Drupal.PanelsIPE.bindClickDelete(context);
    };
    
    Drupal.behaviors['PanelsIPE' + cache_key] = this.behaviorsPassthrough;

    this.initEditing = function() {
      /**
       * See http://jqueryui.com/demos/sortable/ for details on the configuration
       * parameters used here.
       */
      var sortable_options = { // TODO allow the IPE plugin to control these
        revert: true,
        dropOnEmpty: true, // default
        opacity: 0.75, // opacity of sortable while sorting
        // placeholder: 'draggable-placeholder',
        // forcePlaceholderSize: true,
        items: 'div.panels-ipe-pane',
        handle: 'div.panels-ipe-draghandle',
        containment: this.outermost,
      };
      $('div.panels-ipe-region', this.outermost).sortable(sortable_options);
      // Since the connectWith option only does a one-way hookup, iterate over
      // all sortable regions to connect them with one another.
      $('div.panels-ipe-region', this.outermost).each(function() {
        $(this).sortable('option', 'connectWith', ['div.panels-ipe-region'])
      });
      // Show all the hidden IPE elements
      $('div.panels-ipe-handlebar-wrapper,.panels-ipe-newblock').fadeIn('slow');
    }

    this.endEditing = function() {
      // Re-hide all the IPE meta-elements
      $('div.panels-ipe-handlebar-wrapper,.panels-ipe-newblock').hide('slow');
    };

    this.saveEditing = function() {
      $('div.panels-ipe-region', this.outermost).each(function() {
        var val = '';
        var region = $(this).attr('id').split('panels-ipe-regionid-')[1];
        $(this).children('div.panels-ipe-pane').each(function() {
          if (val) {
            val += ',';
          }
          val += $(this).attr('id').split('panels-ipe-paneid-')[1];
        });
        $('input#edit-panel-pane-' + region, this.control).val(val);
      });
    };

    this.cancelEditing = function() {

    };

    $('div.panels-ipe-startedit', this.control).click(function() {
      var $this = $(this);
      var url = this.cfg.formPath;
      $.ajax({
        type: "POST",
        url: url,
        global: true,
        success: function(data) {
          $this.parent().fadeOut('normal', function() {
            $this.hide();
            $this.parent().append(data);
            $this.parent().fadeIn('normal', this.initEditing());
          });
        },
        error: function(xhr) {
          Drupal.CTools.AJAX.handleErrors(xhr, url);
        },
        dataType: 'json'
      });
    });
  };
})(jQuery);
