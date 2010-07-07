// $Id$

// Ensure the $ alias is owned by jQuery.
(function($) {

Drupal.PanelsIPE = {
  editors: {},
  bindClickDelete: function(context) {
    $('a.pane-delete:not(.pane-delete-processed)', context)
      .addClass('pane-delete-processed')
      .click(function() {
        if (confirm('Remove this pane?')) {
          $(this).parents('div.panels-ipe-portlet-wrapper').fadeOut(1000, function() {
            $(this).empty().remove();
          });
          $(this).parents('div.panels-ipe-display-container').addClass('changed');
        }
        return false;
      });
  },
}

// A ready function should be sufficient for this, at least for now
$(function() {
  $.each(Drupal.settings.PanelsIPECacheKeys, function() {
    Drupal.PanelsIPE.editors[this] = new DrupalPanelsIPE(this, Drupal.settings.PanelsIPESettings[this]);
  });
});

Drupal.behaviors.PanelsIPE = function(context) {
  Drupal.PanelsIPE.bindClickDelete(context);
};

Drupal.CTools.AJAX.commands.initIPE = function(data) {
  if (Drupal.PanelsIPE.editors[data.key]) {
    Drupal.PanelsIPE.editors[data.key].initEditing(data.data);
  }
};

Drupal.CTools.AJAX.commands.endIPE = function(data) {
  if (Drupal.PanelsIPE.editors[data.key]) {
    Drupal.PanelsIPE.editors[data.key].endEditing(data.data);
  }
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
  var ipe = this;
  this.key = cache_key;
  this.state = {};
  this.topParent = $('div#panels-ipe-display-' + cache_key);
  this.control = $('div#panels-ipe-control-' + cache_key);
  this.initButton = $('div.panels-ipe-startedit', this.control);
  this.cfg = cfg;
  this.changed = false;

  this.initEditing = function(formdata) {
    // See http://jqueryui.com/demos/sortable/ for details on the configuration
    // parameters used here.
    this.changed = false;

    this.sortable_options = { // TODO allow the IPE plugin to control these
      revert: 200,
      dropOnEmpty: true, // default
      opacity: 0.75, // opacity of sortable while sorting
      // placeholder: 'draggable-placeholder',
      // forcePlaceholderSize: true,
      items: 'div.panels-ipe-portlet-wrapper',
      handle: 'div.panels-ipe-draghandle',
      tolerance: 'pointer',
      cursorAt: 'top',
      update: this.setChanged,
      scroll: true
      // containment: ipe.topParent,
    };

    $('div.panels-ipe-sort-container', ipe.topParent).sortable(ipe.sortable_options);

    // Since the connectWith option only does a one-way hookup, iterate over
    // all sortable regions to connect them with one another.
    $('div.panels-ipe-sort-container', ipe.topParent)
      .sortable('option', 'connectWith', ['div.panels-ipe-sort-container']);

    $('div.panels-ipe-sort-container', ipe.topParent).bind('sortupdate', function() {
      ipe.changed = true;
    });

    $('.panels-ipe-form-container', ipe.control).append(formdata);
    // bind ajax submit to the form
    $('form', ipe.control).submit(function(event) {
      url = $(this).attr('action');
      try {
        var ajaxOptions = {
          type: 'POST',
          url: url,
          data: { 'js': 1, 'key': ipe.key },
          global: true,
          success: Drupal.CTools.AJAX.respond,
          error: function(xhr) {
            Drupal.CTools.AJAX.handleErrors(xhr, url);
          },
          dataType: 'json'
        };
        $(this).ajaxSubmit(ajaxOptions);
      }
      catch (err) {
        alert("An error occurred while attempting to process " + url);
        return false;
      }
      return false;
    });

    $('input:submit', ipe.control).each(function() {
      if ($(this).val() == 'Save') {
        $(this).click(ipe.saveEditing);
      };
      if ($(this).val() == 'Cancel') {
        $(this).click(ipe.cancelEditing);
      };
    });
    this.backup = this.topParent.clone();

    // Perform visual effects in a particular sequence.
    ipe.initButton.css('position', 'absolute');
    ipe.initButton.fadeOut('normal');
    $('.panels-ipe-on').show('normal');
//    $('.panels-ipe-on').fadeIn('normal');
    ipe.topParent.addClass('panels-ipe-editing');
  }

  this.endEditing = function() {
    $('.panels-ipe-form-container', ipe.control).empty();
    // Re-show all the IPE non-editing meta-elements
    $('div.panels-ipe-off').show('fast');

    // Re-hide all the IPE meta-elements
    $('div.panels-ipe-on').hide('fast');
    ipe.initButton.css('position', 'normal');
    ipe.topParent.removeClass('panels-ipe-editing');
  };

  this.saveEditing = function() {
    $('div.panels-ipe-sort-container', ipe.topParent).each(function() {
      var val = '';
      $.each($(this).sortable('toArray'), function(i, v) {
        v = v.replace('panels-ipe-paneid-', '');
        val += val ? ',' + v : v;
      });
      $('input#edit-panel-pane-' + $(this).attr('id').replace('panels-ipe-regionid-', ''), ipe.control).val(val);
    });
  };

  this.cancelEditing = function() {
    if (ipe.topParent.hasClass('changed')) {
      ipe.changed = true;
    }

    if (!ipe.changed || confirm(Drupal.t('This will discard all unsaved changes. Are you sure?'))) {
//      window.location.reload(); // trigger a page refresh.
      // $('div.panels-ipe-region', ipe.topParent).sortable('destroy');
      ipe.topParent.fadeOut('medium', function() {
        ipe.topParent.replaceWith(ipe.backup.clone());
        ipe.topParent = $('div#panels-ipe-display-' + ipe.key);
        ipe.topParent.fadeIn('medium');
      });
    }
    else {
      // Cancel the submission.
      return false;
    }
  };

  var ajaxOptions = {
    type: "POST",
    url: ipe.cfg.formPath,
    data: { 'js': 1, 'key': ipe.key },
    global: true,
    success: Drupal.CTools.AJAX.respond,
    error: function(xhr) {
      Drupal.CTools.AJAX.handleErrors(xhr, ipe.cfg.formPath);
    },
    dataType: 'json'
  };

  $('div.panels-ipe-region', this.topParent).each(function() {
    $('div.panels-ipe-portlet-marker', this).parent()
      .wrapInner('<div class="panels-ipe-sort-container" />');

    // Move our gadgets outside of the sort container so that sortables
    // cannot be placed after them.
    $('div.panels-ipe-portlet-static', this).each(function() {
      $(this).appendTo($(this).parent().parent());
    });

    // Add a marker so we can drag things to empty containers.
    $('div.panels-ipe-sort-container', this).append('<div>&nbsp;</div>');
  });

  $('div.panels-ipe-startedit', this.control).click(function() {
    var $this = $(this);
    $.ajax(ajaxOptions);
  });
};

})(jQuery);
