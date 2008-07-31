// $Id$
/**
 * @file
 * Implement the modal forms used in Panels.
 *
 * The modal form is implemented primarily from mc.js; this contains the
 * Drupal and Panel specific stuff to use it. The modal is now fairly
 * generic and it can be activated mostly by setting up the right
 * classes, but if you are using the modal you must include links to
 * the images in settings, because the javascript does not inherently 
 * know where the images are at.
 */

Drupal.Panels.Subform = {};

Drupal.Panels.Subform.bindAjaxResponse = function(data) {
  // On success, append the returned HTML to the panel's element.
  if (data.type == 'display') {
    // append the output
    $('#modalContent span.modal-title').html(data.title);
    $('#modalContent div.modal-content').html(data.output);

    Drupal.attachBehaviors('#modalContent');  

    // Bind forms to ajax submit.
    $('div.panels-modal-content form').unbind('submit'); // be safe here.
    $('div.panels-modal-content form').submit(function() {
      $(this).ajaxSubmit({
        url: data.url,
        data: '',
        method: 'post',
        success: Drupal.Panels.Subform.bindAjaxResponse,
        error: function() { 
          alert(Drupal.t('There was an error submitting the form to ' + data.url)); $('#panels-modal').unmodalContent(); 
        },
        dataType: 'json'
      });
      return false;     
    });

    // TODO: Make these their own thing set up with behaviors.

    if ($('#override-title-checkbox').size()) {
      Drupal.Panels.Checkboxes.bindCheckbox('#override-title-checkbox', ['#override-title-textfield']);
    }

    if ($('#use-pager-checkbox').size()) {
      Drupal.Panels.Checkboxes.bindCheckbox('#use-pager-checkbox', ['#use-pager-textfield']);
    }

  }
  else if (data.type == 'add') {
    // Give it all the goodies that our existing panes have.   
    $(data.region).append(data.output);
    
    Drupal.Panels.changed($(data.id));
    Drupal.attachBehaviors(data.id);  

    // dismiss the dialog
    Drupal.Panels.Subform.dismiss();
  }
  else if (data.type == 'replace') {
    // Replace the HTML in the pane
    $(data.id).replaceWith(data.output);

    Drupal.Panels.changed($(data.id));
    Drupal.attachBehaviors(data.id);  

    // dismiss the dialog
    Drupal.Panels.Subform.dismiss();
  }
  else if (data.type == 'dismiss') {
    // If an id was added, mark it as changed.
    if (data.id) {
      Drupal.Panels.changed($('#' + data.id));
    }
    // Dismiss the dialog.
    Drupal.Panels.Subform.dismiss();
  }
  else {
    // just dismiss the dialog.
    Drupal.Panels.Subform.dismiss();
  }

  // check for global replacements
  if (data.replace) {
    for (id in data.replace) {
      // Replace the HTML in the pane
      $(id).replaceWith(data.replace[id]);

      Drupal.Panels.changed($(id));
      Drupal.attachBehaviors(id);  
    }
  }

};

/**
 * Display the modal
 */
Drupal.Panels.Subform.show = function() {
  $('#panels-modal').modalContent({
    opacity: '.40', 
    background: '#fff'
  });
  $('#modalContent .modal-content').html($('div#panels-throbber').html());
};

/**
 * Hide the modal
 */
Drupal.Panels.Subform.dismiss = function() {
  $('#panels-modal').unmodalContent();
};

Drupal.Panels.Subform.createModal = function() {
  var html = ''
  html += '<div class="panels-hidden">';
  html += '  <div id="panels-modal">'
  html += '    <div class="panels-modal-content">'
  html += '      <div class="modal-header">';
  html += '        <a class="close" href="#">';
  html +=            Drupal.settings.panels.closeText + Drupal.settings.panels.closeImage;
  html += '        </a>';
  html += '        <span class="modal-title">&nbsp;</span>';
  html += '      </div>';
  html += '      <div class="modal-content">';
  html += '      </div>';
  html += '    </div>';
  html += '  </div>';
  html += '  <div id="panels-throbber">';
  html += '    <div class="panels-throbber-wrapper">';
  html +=        Drupal.settings.panels.throbber;
  html += '    </div>';
  html += '  </div>';
  html += '</div>';

  $('body').append(html);
};

/**
 * Generic replacement click handler to open the modal with the destination
 * specified by the href of the link.
 */
Drupal.Panels.clickAjaxLink = function() {
  // show the empty dialog right away.
  if (!$(this).hasClass('panels-no-modal')) {
    Drupal.Panels.Subform.show();
  }

  var url = $(this).attr('href');
  $.ajax({
    type: "POST",
    url: url,
    data: '',
    global: true,
    success: Drupal.Panels.Subform.bindAjaxResponse,
    error: function() { 
      alert("An error occurred while attempting to process " + url); 
      Drupal.Panels.Subform.dismiss(); 
    },
    dataType: 'json'
  });
  return false;
};

/**
 * Generic replacement click handler to open the modal with the destination
 * specified by the href of the link.
 */
Drupal.Panels.clickAjaxButton = function() {
  var url = '';
  // The URL for this gadget will be composed of the values of items by
  // taking the ID of this item and adding -url and looking for that
  // class. They need to be in the form in order since we will
  // concat them all together using '/'.
  var url_class = '.' + $(this).attr('id') + '-url';
  $(url_class).each(
    function() { 
      if (url && $(this).val()) { 
        url += '/'; 
      }
      url += $(this).val(); 
    });

  if (!url) {
    return false;
  }

  // show the empty dialog right away.
  if (!$(this).hasClass('panels-no-modal')) {
    Drupal.Panels.Subform.show();
  }

  $.ajax({
    type: "POST",
    url: url,
    data: '',
    global: true,
    success: Drupal.Panels.Subform.bindAjaxResponse,
    error: function() { 
      alert("An error occurred while attempting to process " + url); 
      Drupal.Panels.Subform.dismiss(); 
    },
    dataType: 'json'
  });
  return false;
};

Drupal.Panels.makeTableDraggable = function(item) {
  // transform the item from just an identifier to the actual item. This
  // is necessary or tabledrag information gets lost.
  item = $(item).get(0);
  // Only process rows that don't have drag handles already
  if ($('.tabledrag-handle', item).size() != 0) {
    return;
  }

  // figure out parent table id
  var parent = $(item).parents('table');
  // skip if parent hasn't already been processed
  if (!$(parent).hasClass('tabledrag-processed')) {
    return;
  }

  var id = $(parent).attr('id');
  if (Drupal.tableDrag[id]) {
    Drupal.tableDrag[id].makeDraggable(item);

    // redisplay cells so that hideColumns won't just exit
    for (var group in Drupal.tableDrag[id].tableSettings) {
      // Find the first field in Drupal.tableDrag[id] group.
      for (var d in Drupal.tableDrag[id].tableSettings[group]) {
        var field = $('.' + Drupal.tableDrag[id].tableSettings[group][d]['target'] + ':first', Drupal.tableDrag[id].table);
        if (field.size() && Drupal.tableDrag[id].tableSettings[group][d]['hidden']) {
          var hidden = Drupal.tableDrag[id].tableSettings[group][d]['hidden'];
          var cell = field.parents('td:first');
          break;
        }
      }
      $(cell).css('display', 'block');
    }
    // Hide columns containing affected form elements.
    Drupal.tableDrag[id].hideColumns();
  }
};

/**
 * Bind all modals to their buttons. They'll be in the settings like so:
 * Drupal.settings.panels.modals.button-id = url
 */
Drupal.Panels.Subform.autoAttach = function() {
  Drupal.Panels.Subform.createModal();
};

Drupal.behaviors.PanelsSubForm = function(context) {
  // Bind links
  $('a.panels-ajax-link:not(.display-processed)', context)
    .addClass('display-processed')
    .click(Drupal.Panels.clickAjaxLink);

  // Bind buttons
  $('input.panels-ajax-link:not(.display-processed)', context)
    .addClass('display-processed')
    .click(Drupal.Panels.clickAjaxButton);

  // tabledrag.js doesn't have a behavior to make new rows draggable
  // so we have to do it for it.
  $('tr.draggable', context).each(function() {
    Drupal.Panels.makeTableDraggable(this);
  });

  if ($(context).hasClass('draggable')) {
    Drupal.Panels.makeTableDraggable(context);
  }
};


$(Drupal.Panels.Subform.autoAttach);
