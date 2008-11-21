// $Id$

/**
 * List object
 *
 * Settings:
 *
 * container
 * up
 * down
 * delete: input image
 * order: input hidden
 * add: input to add items
 * path: path to add items
 * tr: the base id of the tr
 */
Drupal.list = function(base, settings) {
  // Set the properties for this object.
  if (settings.container == null) {
    settings.container = settings.row_class;
  }

  var max = function(array) {
    return Math.max.apply(Math, array);
  };

  this.settings = settings;
  this.base = '#' + base;

  // helper function to swap items in an array
  var swap = function(array, a, b) {
    var temp = array[a];
    array[a] = array[b];
    array[b] = temp;
  };

  var saveOrder = function(order) {
    var new_order = '';
    for (i in order) {
      if (new_order) {
        new_order += ',';
      }
      new_order += order[i];
    }

    $(settings.order).val(new_order);
    if (settings.hidden) {
      $(settings.hidden).show();
    }
//    console.log(order);
  };

  var changeOrder = function(item, how) {
    // If there isn't an order field, don't process this.
    if (!settings.order) {
      return;
    }

//    console.log(settings.order);

    var order_text = $(settings.order).val();
    if (order_text == '') {
      var order = [];
    }
    else {
      var order = order_text.split(',');
    }

//    console.log(order);
    if (how != 'add') {
      var tr = $(item).parents('tr').get(0);
      var id = $(tr).attr('id').replace(settings.tr, '');
      var index = -1;
    
      for (var i in order) {
        if (order[i] == id) {
          index = parseInt(i);
          break;
        }
      }
    }

    switch (how) {
      case 'add':
        order.push(item);
        break;
      case 'delete':
        order.splice(index, 1);
        break;
      case 'up':
        swap(order, index, index - 1);
        break;
      case 'down':
        swap(order, index, index + 1);
        break;
    }
    
    saveOrder(order);
  };

  this.changeOrder = changeOrder;

  var restripeTable = function(table) {
    // :even and :odd are reversed because jquery counts from 0 and
    // we count from 1, so we're out of sync.
    $('tbody tr:not(:hidden)', $(table))
      .removeClass('even')
      .removeClass('odd')
      .filter(':even')
        .addClass('odd')
      .end()
      .filter(':odd')
        .addClass('even');
  };

  this.restripeTable = restripeTable;

  var changed = function(item) {
    if (!item.is('.changed')) {
      item.addClass('changed');
      item.children('td:first').append(' <span class="star">*</span> ');
    }
  };

  // Set as a function so we can be both a closure and called later
  // when more items get added.
  this.bindButtons = function() {
    if (settings.remove) {
      $(settings.remove + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          var item = $(this).parents(settings.container);
          changeOrder(this, 'delete');

          item.remove();
          restripeTable('#' + base);

          return false;
        });
    }

    if (settings.clear_list) {
      $(settings.clear_list + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          $(settings.row_class).each(function() { $(this).remove() });
          saveOrder([]);
        });
    }

    if (settings.up) {
      $(settings.up + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          var item = $(this).parents(settings.container);
          var prev = $(item).prev();
          if (prev.html() != null && item != prev) {
            // move item
            prev.before(item);
            restripeTable('#' + base);
            changed(item);
            changeOrder(this, 'up');
          }

          return false;
        });
    }

    if (settings.down) {
      $(settings.down + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          var item = $(this).parents(settings.container);
          var next = $(item).next();

          if (next.html() != null && item != next) {
            // move item
            next.after(item);
            restripeTable('#' + base);
            changed(item);
            changeOrder(this, 'down');
          }

          return false;
        });
    }

    var bindAjaxResponse = function (data) {
      // Parse response
      data = Drupal.parseJson(data);
      if (data.debug) {
        alert(data.debug);
      }
      switch(data.type) {
        case 'display':
          // Display our modal popup
          $('#panels-modal').modalContent({
              opacity: '.40', 
              background: '#fff'
            }
          );
          // append the output
          $('#modalContent span.modal-title').html(data.title);
          $('#modalContent div.modal-content').html(data.output);

          // Bind forms to ajax submit.
          $('div.panels-modal-content form').unbind('submit'); // be safe here.
          $('div.panels-modal-content form').submit(function() {
            $(this).ajaxSubmit({
              url: data.url,
              data: '',
              method: 'post',
              after: bindAjaxResponse
            });
            return false;
            
          });
          // hack: allow collapsible textareas to work
          Drupal.Panels.Subform.bindCollapsible();

          Drupal.Panels.Subform.bindAutocomplete();
          return;
        case 'add':
          // add new data
          Drupal.freezeHeight();
          // Hide the new content before adding to page.

          // Add the form and re-attach behavior.
          $('#' + base + ' tbody').append(data.output);
          changed($('#' + base + ' tbody tr:last'));

          changeOrder(data.position, 'add');

          // If we're told we have extra things to put out there, do so.
          // Must be done prior to button binding.
          if (settings.replace) {
            for (i in settings.replace) {
              if (data[settings.replace[i]]) {
                $(i).html(data[settings.replace[i]]);
              }
            }
          }

          // Run a bind for every list we know about
          for (list in Drupal.list.lists) {
            Drupal.list.lists[list].bindButtons();
          }

          Drupal.unfreezeHeight();

          // Add the extra data, if necessary
          if (data.extra && settings.extra) {
            var val = $(settings.extra).val();
            if (val) {
              val += ',';
            }
            val += data.extra;
            $(settings.extra).val(val);
          }

          if (settings.clear) {
            for (i in settings.clear) {
              $(settings.clear[i]).val('');
            }
          }

          $('#panels-modal').unmodalContent();
          return;
        case 'dismiss':
          // just dismiss the dialog.
//          console.log(data.replace);
          if (data.replace_id && data.replace) {
            $(data.replace_id).html(data.replace);
            changed($(data.replace_id));
            // Run a bind for every list we know about
            for (list in Drupal.list.lists) {
              Drupal.list.lists[list].bindButtons();
            }
          }

          $('#panels-modal').unmodalContent();
          return;
        default:
          $('#panels-modal').unmodalContent();
          alert("Unknown response from server.");
          return;
      }
    };

    if (settings.add) {
      $(settings.add + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          var input = this;
          $(input).attr('disabled', true);
          $(input).parent().addClass('throbbing');
          var data = { };
          for (i in settings.post) {
            data[$(settings.post[i]).attr('name')] = $(settings.post[i]).val();
          }

          $.ajax({
            type: 'POST',
            data: data,
            url: settings.path,
            global: true,
            success: function(data) { 
              $(input).parent().removeClass('throbbing');
              $(input).attr('disabled', false);
              bindAjaxResponse(data);
            },
            error: function(data) {
              alert('An error occurred while attempting to process the data.');
              $(input).parent().removeClass('throbbing');
              $(input).attr('disabled', false);
            }
          });
          return false;
        });
    }

    if (settings.configure) {
      $(settings.configure + ':not(.list-processed)')
        .addClass('list-processed')
        .click(function() { return false; })
        .click(function(e) {
          var input = this;
          var tr = $(this).parents('tr').get(0);
          var id = $(tr).attr('id').replace(settings.tr, '');
          var data = { position: id };

          $.ajax({
            type: 'POST',
            data: data,
            url: settings.configure_path,
            global: true,
            success: bindAjaxResponse,
            error: function(data) {
              alert('An error occurred  while attempting to process the data.');
            }
          });
          return false;
        });
    }
  };

  // Focus if we're told to.
  if (settings.focus) {
    $(settings.focus).get(0).focus();
  }
  this.bindButtons();
}


Drupal.list.autoAttach = function() {
  if (Drupal.settings && Drupal.settings.list) {
    Drupal.list.lists = {};

    for (var base in Drupal.settings.list) {
      if (!$('#'+ base + '.list-processed').size()) {
        var settings = Drupal.settings.list[base];
        var list = new Drupal.list(base, settings);
        Drupal.list.lists[base] = list;
        $('#'+ base).addClass('list-processed');
      }
    }
  }
  Drupal.Panels.Subform.createModal();
};

if (Drupal.jsEnabled) {
  $(document).ready(Drupal.list.autoAttach);
}

Drupal.Panels = {};

