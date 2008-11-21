/*
 * jQuery form plugin
 * @requires jQuery v1.0.2
 *
 * Dual licensed under the MIT and GPL licenses:
 *   http://www.opensource.org/licenses/mit-license.php
 *   http://www.gnu.org/licenses/gpl.html
 */

/**
 * ajaxSubmit() provides a mechanism for submitting an HTML form using AJAX.
 *
 * Options are provided via an options object.  The following options are supported:
 *
 *  target:   Identifies the element(s) in the page to be updated with the server response.
 *            This value may be specified as a jQuery selection string, a jQuery object,
 *            or a DOM element.
 *            default value: null
 *
 *  url:      URL to which the form data will be submitted.
 *            default value: value of form's 'action' attribute
 *
 *  method:   The method in which the form data should be submitted, 'GET' or 'POST'.
 *            default value: value of form's 'method' attribute (or 'GET' if none found)
 *
 *  before:   Callback method to be invoked before the form is submitted.
 *            default value: null
 *
 *  after:    Callback method to be invoked after the form has been successfully submitted.
 *            default value: null
 *            (Note: 'success' can be used in place of 'after')
 *
 *  dataType: Expected dataType of the response.  One of: null, 'xml', 'script', or 'json'
 *            default value: null
 *
 *  semantic: Boolean flag indicating whether data must be submitted in semantic order (slower).
 *            default value: false
 *
 * Note: When a target is specified and a dataType is not, this call is routed through
 *       jQuery's 'load' method in order to load content into the target element.  In all
 *       other cases this call is routed through jQuery's 'ajax' method and the options argument 
 *       is passed through to the ajax call.
 *
 * The 'before' callback can be provided as a hook for running pre-submit logic or for
 * validating the form data.  If the 'before' callback returns false then the form will
 * not be submitted. The 'before' callback is invoked with two arguments: the form data
 * in array format, and the jQuery object.  The form data array takes the following form:
 *
 *     [ { name: 'username', value: 'jresig' }, { name: 'password', value: 'secret' } ]
 *
 * If an 'after' callback method is provided it is invoked after the response has been returned
 * from the server.  It is passed the responseText or responseXML value (depending on dataType).
 * See jQuery.ajax for further details.
 *
 *
 * The dataType option provides a means for specifying how the server response should be handled.
 * This maps directly to the jQuery.httpData method.  The following values are supported:
 * 
 *      'xml':    if dataType == 'xml' the server response is treated as XML and the 'after'
 *                   callback method, if specified, will be passed the responseXML value
 *      'json':   if dataType == 'json' the server response will be evaluted and passed to
 *                   the 'after' callback, if specified
 *      'script': if dataType == 'script' the server response is evaluated in the global context
 *
 *
 * Note that it does not make sense to use both the 'target' and 'dataType' options.  If both
 * are provided the target will be ignored.
 *
 * The semantic argument can be used to force form serialization in semantic order.  If your
 * form must be submitted with name/value pairs in semantic order then pass true for this arg,
 * otherwise pass false (or nothing) to avoid the overhead for this logic (which can be
 * significant for very large forms).
 *
 * When used on its own, ajaxSubmit() is typically bound to a form's submit event like this:
 *
 * $("#form-id").submit(function() {
 *     $(this).ajaxSubmit(options);
 *     return false; // cancel conventional submit
 * });
 *
 * When using ajaxForm(), however, this is done for you.
 *
 *
 * @example
 * var options = {
 *     target: '#myTargetDiv'
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc Submit form and update page element with server response
 *
 *
 * @example
 * var options = {
 *     after: function(responseText) {
 *         alert(responseText);
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc Submit form and alert the server response
 *
 *
 * @example
 * var options = {
 *     before: function(formArray, jqForm) {
 *         if (formArray.length == 0) {
 *             alert('Please enter data.');
 *             return false;
 *         }
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc Pre-submit validation which aborts the submit operation if form data is empty
 *
 *
 * @example
 * var options = {
 *     url: myJsonUrl.php,
 *     dataType: 'json',
 *     after: function(data) {
 *        // 'data' is an object representing the the evaluated json data
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc json data returned and evaluated
 *
 *
 * @example
 * var options = {
 *     url: myXmlUrl.php,
 *     dataType: 'xml',
 *     after: function(responseXML) {
 *        // responseXML is XML document object
 *        var data = $('myElement', responseXML).text();
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc XML data returned from server
 *
 *
 * @example
 * $('#myForm).submit(function() {
 *    $(this).ajaxSubmit();
 *    return false;
 * });
 * @desc Bind form's submit event to use ajaxSubmit
 *
 *
 * @name ajaxSubmit
 * @type jQuery
 * @param options  object literal containing options which control the form submission process
 * @cat Plugins/Form
 * @return jQuery
 * @see formToArray
 * @see ajaxForm
 * @see load
 * @see $.ajax
 * @author jQuery Community
 */
jQuery.fn.ajaxSubmit = function(options) {
    options = jQuery.extend({
        target:   null,
        url:      this.attr('action') || '',
        method:   this.attr('method') || 'GET',
        before:   null,
        after:    null,
        dataType: null, // 'xml', 'script', or 'json' (@see jQuery.httpData)
        semantic: false
    }, options || {});

    // remap 'after' to 'success' for the load and ajax methods
    options.success = options.success || options.after;

    var a = this.formToArray(options.semantic);

    // give pre-submit callback an opportunity to abort the submit
    if (options.before && options.before(a, this, options) === false) return;

    var q = jQuery.param(a);
    var get = (options.method && options.method.toUpperCase() == 'GET');

    if (get)
        // if url already has a '?' then append args after '&'
        options.url += (options.url.indexOf('?') >= 0 ? '&' : '?') + q;

    // perform a load on the target only if dataType is not provided
    if (!options.dataType && options.target)
        jQuery(options.target).load(options.url, get ? null : a, options.success);
    else {
        // remap 'method' to 'type' for the ajax method
        options.type = options.method;
        options.data = get ? null : q;  // data is null for 'get' or the query string for 'post'
        // pass options along to ajax method
        jQuery.ajax(options);
    }
    return this;
};


/**
 * ajaxForm() provides a mechanism for fully automating form submission.
 *
 * The advantages of using this method instead of ajaxSubmit() are:
 *
 * 1: This method will include coordinates for <input type="image" /> elements (if the element
 *    is used to submit the form).
 * 2. This method will include the submit element's name/value data (for the element that was
 *    used to submit the form).
 * 3. This method binds the submit() method to the form for you.
 *
 * Note that for accurate x/y coordinates of image submit elements in all browsers
 * you need to also use the "dimensions" plugin (this method will auto-detect its presence).
 *
 * The options argument for ajaxForm works exactly as it does for ajaxSubmit.  ajaxForm merely
 * passes the options argument along after properly binding events for submit elements and
 * the form itself.  See ajaxSubmit for a full description of the options argument.
 *
 *
 * @example
 * var options = {
 *     target: '#myTargetDiv'
 * };
 * $('#myForm').ajaxSForm(options);
 * @desc Bind form's submit event so that 'myTargetDiv' is updated with the server response
 *       when the form is submitted.
 *
 *
 * @example
 * var options = {
 *     after: function(responseText) {
 *         alert(responseText);
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc Bind form's submit event so that server response is alerted after the form is submitted.
 *
 *
 * @example
 * var options = {
 *     before: function(formArray, jqForm) {
 *         if (formArray.length == 0) {
 *             alert('Please enter data.');
 *             return false;
 *         }
 *     }
 * };
 * $('#myForm').ajaxSubmit(options);
 * @desc Bind form's submit event so that pre-submit callback is invoked before the form
 *       is submitted.
 *
 *
 * @name   ajaxForm
 * @param  options  object literal containing options which control the form submission process
 * @return jQuery
 * @cat    Plugins/Form
 * @type   jQuery
 * @see    ajaxSubmit
 * @author jQuery Community
 */
jQuery.fn.ajaxForm = function(options) {
    return this.each(function() {
        jQuery("input:submit,input:image", this).click(function(ev) {
            this.form.clk = this;

            if (ev.offsetX != undefined) {
                this.form.clk_x = ev.offsetX;
                this.form.clk_y = ev.offsetY;
            } else if (typeof jQuery.fn.offset == 'function') { // try to use dimensions plugin
                var offset = $(this).offset();
                this.form.clk_x = ev.pageX - offset.left;
                this.form.clk_y = ev.pageY - offset.top;
            } else {
                this.form.clk_x = ev.pageX - this.offsetLeft;
                this.form.clk_y = ev.pageY - this.offsetTop;
            }
        })
    }).submit(function(e) {
        jQuery(this).ajaxSubmit(options);
        return false;
    });
};


/**
 * formToArray() gathers form element data into an array of objects that can
 * be passed to any of the following ajax functions: $.get, $.post, or load.
 * Each object in the array has both a 'name' and 'value' property.  An example of
 * an array for a simple login form might be:
 *
 * [ { name: 'username', value: 'jresig' }, { name: 'password', value: 'secret' } ]
 *
 * It is this array that is passed to pre-submit callback functions provided to the
 * ajaxSubmit() and ajaxForm() methods.
 *
 * The semantic argument can be used to force form serialization in semantic order.
 * If your form must be submitted with name/value pairs in semantic order then pass
 * true for this arg, otherwise pass false (or nothing) to avoid the overhead for
 * this logic (which can be significant for very large forms).
 *
 * @example var data = $("#myForm").formToArray();
 * $.post( "myscript.cgi", data );
 * @desc Collect all the data from a form and submit it to the server.
 *
 * @name formToArray
 * @param semantic true if serialization must maintain strict semantic ordering of elements (slower)
 * @type Array<Object>
 * @cat Plugins/Form
 * @see ajaxForm
 * @see ajaxSubmit
 * @author jQuery Community
 */
jQuery.fn.formToArray = function(semantic) {
    var a = [];
    var q = semantic ? ':input' : 'input,textarea,select,button';

    jQuery(q, this).each(function() {
        var n = this.name;
        var t = this.type;
        var tag = this.tagName.toLowerCase();

        if ( !n || this.disabled || t == 'reset' ||
            (t == 'checkbox' || t == 'radio') && !this.checked ||
            (t == 'submit' || t == 'image' || t == 'button') && this.form && this.form.clk != this ||
            tag == 'select' && this.selectedIndex == -1)
            return;

        if (t == 'image' && this.form.clk_x != undefined)
            return a.push(
                {name: n+'_x', value: this.form.clk_x},
                {name: n+'_y', value: this.form.clk_y}
            );

        if (tag == 'select') {
            // pass select element off to fieldValue to reuse the IE logic
            var val = jQuery.fieldValue(this, false); // pass false to optimize fieldValue
            if (t == 'select-multiple') {
                for (var i=0; i < val.length; i++)
                    a.push({name: n, value: val[i]});
            }
            else
                a.push({name: n, value: val});
        }
        else
            a.push({name: n, value: this.value});
    });
    return a;
};


/**
 * Serializes form data into a 'submittable' string. This method will return a string
 * in the format: name1=value1&amp;name2=value2
 *
 * The semantic argument can be used to force form serialization in semantic order.
 * If your form must be submitted with name/value pairs in semantic order then pass
 * true for this arg, otherwise pass false (or nothing) to avoid the overhead for
 * this logic (which can be significant for very large forms).
 *
 * @example var data = $("#myForm").formSerialize();
 * $.ajax('POST', "myscript.cgi", data);
 * @desc Collect all the data from a form into a single string
 *
 * @name formSerialize
 * @param semantic true if serialization must maintain strict semantic ordering of elements (slower)
 * @type String
 * @cat Plugins/Form
 * @see formToArray
 * @author jQuery Community
 */
jQuery.fn.formSerialize = function(semantic) {
    //hand off to jQuery.param for proper encoding
    return jQuery.param(this.formToArray(semantic));
};


/**
 * Serializes all field elements in the jQuery object into a query string. 
 * This method will return a string in the format: name1=value1&amp;name2=value2
 *
 * The successful argument controls whether or not serialization is limited to
 * 'successful' controls (per http://www.w3.org/TR/html4/interact/forms.html#successful-controls).
 * The default value of the successful argument is true.
 *
 * @example var data = $("input").formSerialize();
 * @desc Collect the data from all successful input elements into a query string
 *
 * @example var data = $(":radio").formSerialize();
 * @desc Collect the data from all successful radio input elements into a query string
 *
 * @example var data = $("#myForm :checkbox").formSerialize();
 * @desc Collect the data from all successful checkbox input elements in myForm into a query string
 *
 * @example var data = $("#myForm :checkbox").formSerialize(false);
 * @desc Collect the data from all checkbox elements in myForm (even the unchecked ones) into a query string
 *
 * @example var data = $(":input").formSerialize();
 * @desc Collect the data from all successful input, select, textarea and button elements into a query string
 *
 * @name fieldSerialize
 * @param successful true if only successful controls should be serialized (default is true)
 * @type String
 * @cat Plugins/Form
 */
jQuery.fn.fieldSerialize = function(successful) {
    var a = [];
    this.each(function() {
        if (!this.name) return;
        var val = jQuery.fieldValue(this, successful);
        if (val && val.constructor == Array) {
            for (var i=0; i < val.length; i++)
                a.push({name: this.name, value: val[i]});
        }
        else if (val !== null && typeof val != 'undefined')
            a.push({name: this.name, value: val});
    });
    //hand off to jQuery.param for proper encoding
    return jQuery.param(a);
};


/**
 * Returns the value of the field element in the jQuery object.  If there is more than one field element
 * in the jQuery object the value of the first successful one is returned.
 *
 * The successful argument controls whether or not the field element must be 'successful'
 * (per http://www.w3.org/TR/html4/interact/forms.html#successful-controls).
 * The default value of the successful argument is true.  If this value is false then
 * the value of the first field element in the jQuery object is returned.
 *
 * Note: The fieldValue returned for a select-multiple element or for a checkbox input will
 *       always be an array.
 *
 * @example var data = $("#myPasswordElement").formValue();
 * @desc Gets the current value of the myPasswordElement element
 *
 * @example var data = $("#myForm :input").formValue();
 * @desc Get the value of the first successful control in the jQuery object.
 *
 * @example var data = $("#myForm :checkbox").formValue();
 * @desc Get the array of values for the first set of successful checkbox controls in the jQuery object.
 *
 * @example var data = $("#mySingleSelect").formValue();
 * @desc Get the value of the select control
 *
 * @example var data = $("#myMultiSelect").formValue();
 * @desc Get the array of selected values for the select-multiple control
 *
 * @name fieldValue
 * @param Boolean successful true if value returned must be for a successful controls (default is true)
 * @type String or Array<String>
 * @cat Plugins/Form
 */
jQuery.fn.fieldValue = function(successful) {
    var cbVal = [], cbName = null;

    // loop until we find a value
    for (var i = 0; i < this.length; i++) {
        var el = this[i];
        if (el.type == 'checkbox') {
            if (!cbName) cbName = el.name || 'unnamed';
            if (cbName != el.name) // return if we hit a checkbox with a different name
                return cbVal;
            var val = jQuery.fieldValue(el, successful);
            if (val !== null && typeof val != 'undefined') 
                cbVal.push(val);
        }
        else {
            var val = jQuery.fieldValue(el, successful);
            if (val !== null && typeof val != 'undefined') 
                return val;
        }
    }
    return cbVal;
};

/**
 * Returns the value of the field element.
 *
 * The successful argument controls whether or not the field element must be 'successful'
 * (per http://www.w3.org/TR/html4/interact/forms.html#successful-controls).
 * The default value of the successful argument is true.  If the given element is not
 * successful and the successful arg is not false then the returned value will be null.
 *
 * Note: The fieldValue returned for a select-multiple element will always be an array.
 *
 * @example var data = jQuery.fieldValue($("#myPasswordElement")[0]);
 * @desc Gets the current value of the myPasswordElement element
 *
 * @name fieldValue
 * @param Element el The DOM element for which the value will be returned
 * @param Boolean successful true if value returned must be for a successful controls (default is true)
 * @type String or Array<String>
 * @cat Plugins/Form
 */
jQuery.fieldValue = function(el, successful) {
    var n = el.name;
    var t = el.type;
    var tag = el.tagName.toLowerCase();
    if (typeof successful == 'undefined') successful = true;

    if (successful && ( !n || el.disabled || t == 'reset' ||
        (t == 'checkbox' || t == 'radio') && !el.checked ||
        (t == 'submit' || t == 'image' || t == 'button') && el.form && el.form.clk != el ||
        tag == 'select' && el.selectedIndex == -1))
            return null;
    
    if (tag == 'select') {
        var a = [];
        for(var i=0; i < el.options.length; i++) {
            var op = el.options[i];
            if (op.selected) {
                // extra pain for IE...
                var v = jQuery.browser.msie && !(op.attributes['value'].specified) ? op.text : op.value;
                if (t == 'select-one')
                    return v;
                a.push(v);
            }
        }
        return a;
    }
    return el.value;
};
