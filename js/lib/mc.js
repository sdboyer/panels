/**
 * modalContent jQuery Plugin
 *
 * @version   0.9
 * @since     2006-11-28
 * @copyright Copyright (c) 2006 Glyphix Studio, Inc. http://www.glyphix.com
 * @author    Gavin M. Roy <gmr@glyphix.com>
 * @license   MIT http://www.opensource.org/licenses/mit-license.php
 * @requires  >= jQuery 1.0.3     http://jquery.com/
 * @requires  dimensions.js       http://jquery.com/dev/svn/trunk/plugins/dimensions/dimensions.js?format=raw
 *
 * Call modalContent() on a DOM object and it will make a centered modal box over a div overlay preventing access to the page.
 * Create an element (anchor/img/etc) with the class "close" in your content to close the modal box on click.
 */

/**
 * modalContent
 * @param content string to display in the content box
 * @param css obj of css attributes
 * @param animation (fade, slide, show)
 * @param speed (valid animation speeds slow, medium, fast or # in ms)
 */
jQuery.modalContent = function(content, css, animation, speed) {

  // if we already ahve a modalContent, remove it
  if ( $('#modalBackdrop') ) $('#modalBackdrop').remove();
  if ( $('#modalContent') ) $('#modalContent').remove();

  // hide the scrollbars
  $('body').css('overflow', 'hidden');

  // position code lifted from http://www.quirksmode.org/viewport/compatibility.html
  if (self.pageYOffset) { // all except Explorer
    var wt = self.pageYOffset;
  } else if (document.documentElement && document.documentElement.scrollTop) { 	// Explorer 6 Strict
      var wt = document.documentElement.scrollTop;
  } else if (document.body) { // all other Explorers
      var wt = document.body.scrollTop;
  }

  // Get our dimensions
  var winHeight = $(window).height();
  var winWidth = $(window).width();

  // Create our divs
  $('body').append('<div id="modalBackdrop" style="z-index: 1000; display: none;"></div><div id="modalContent" style="z-index: 1001; position: absolute;">' + $(content).html() + '</div>');

  // Create our content div, get the dimensions, and hide it
  var modalContent = $('#modalContent').css('top','-1000px');
  var mdcTop = wt + ( winHeight / 2 ) - (  modalContent.outerHeight() / 2);
  var mdcLeft = ( winWidth / 2 ) - ( modalContent.outerWidth() / 2);
  modalContent.hide();

  // Apply our css and positioning, then show
  if ( animation == 'fade' )
  {
    $('#modalBackdrop').css('top',wt).css('height', winHeight + 'px').css('width', winWidth + 'px').show();
     modalContent.css('top', mdcTop + 'px').css('left', mdcLeft + 'px').fadeIn(speed);
  } else {
    if ( animation == 'slide' )
    {
      $('#modalBackdrop').css('top', wt).css('height', winHeight + 'px').css('width', winWidth + 'px').show();
      modalContent.hide().css('top', mdcTop + 'px').css('left', mdcLeft + 'px').slideDown(speed);
    } else {
      if ( animation == 'show')
      {
        $('#modalBackdrop').css('top', wt).css('height', winHeight + 'px').css('width', winWidth + 'px').show();
        modalContent.css('top', mdcTop + 'px').css('left', mdcLeft + 'px').show();
      }
    }
  }

  // Bind a click for closing the modalContent
  $('#modalContent a.close').click(function(){close(); return false});

  // Close the open modal content and backdrop
  function close() {
    $(window).unbind('resize');

    // hide the scrollbars
    $('body').css('overflow', 'auto');

    if ( animation == 'fade' ) {
      $('#modalContent').fadeOut(speed,function(){$('#modalBackdrop').fadeOut(speed, function(){$(this).remove();});$(this).remove();});
    } else {
      if ( animation == 'slide' ) {
        $('#modalContent').slideUp(speed,function(){$('#modalBackdrop').slideUp(speed, function(){$(this).remove();});$(this).remove();});
      } else {
        $('#modalContent').remove();$('#modalBackdrop').remove();
      }
    }
  };

  // Move and resize the modalBackdrop and modalContent on resize of the window
  $(window).resize(function(){
    // Get our heights
    var winHeight = $(window).height();
    var winWidth = $(window).width();

    // Get where we should move content to
    var modalContent = $('#modalContent');
    var mdcTop = ( winHeight / 2 ) - (  modalContent.outerHeight() / 2);
    var mdcLeft = ( winWidth / 2 ) - ( modalContent.outerWidth() / 2);

    // Apply the changes
    $('#modalBackdrop').css('height', winHeight + 'px').css('width', winWidth + 'px').show();
    modalContent.css('top', mdcTop + 'px').css('left', mdcLeft + 'px').show();
  });
};

/**
 * jQuery function init
 */
jQuery.fn.modalContent = function(css, animation, speed)
{
  // If our animation isn't set, make it just show/pop
  if (!animation) { var animation = 'show'; } else {
    // If our animation isn't "fade" then it always is show
    if ( ( animation != 'fade' ) && ( animation != 'slide') ) animation = 'show';
  }
  if ( !speed ) var speed = 'fast';

  // Build our base attributes and allow them to be overriden
  css = jQuery.extend({
    position: 'absolute',
    left: '0px',
    margin: '0px',
    background: '#000',
    opacity: '.55'
  }, css);

  // jQuery mojo
  this.each(function(){
    $(this).hide();
    new jQuery.modalContent($(this), css, animation, speed);
  });

  // return this object
  return this;
};

/**
 * unmodalContent
 * @param animation (fade, slide, show)
 * @param speed (valid animation speeds slow, medium, fast or # in ms)
 */
jQuery.fn.unmodalContent = function(animation, speed)
{
  // If our animation isn't set, make it just show/pop
  if (!animation) { var animation = 'show'; } else {
    // If our animation isn't "fade" then it always is show
    if ( ( animation != 'fade' ) && ( animation != 'slide') ) animation = 'show';
  }
  // Set a speed if we dont have one
  if ( !speed ) var speed = 'fast';

  // Unbind the resize we bound
  $(window).unbind('resize');

  // hide the scrollbars
  $('body').css('overflow', 'auto');

  // jQuery magic loop through the instances and run the animations or removal.
  this.each(function(){
    if ( animation == 'fade' ) {
      $('#modalContent').fadeOut(speed,function(){$('#modalBackdrop').fadeOut(speed, function(){$(this).remove();});$(this).remove();});
    } else {
      if ( animation == 'slide' ) {
        $('#modalContent').slideUp(speed,function(){$('#modalBackdrop').slideUp(speed, function(){$(this).remove();});$(this).remove();});
      } else {
        $('#modalContent').remove();$('#modalBackdrop').remove();
      }
    }
  });
};
