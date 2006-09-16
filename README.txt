The panels module is the ideological son, successor to and almost complete 
replacement for the dashboard module. This module allows you to create pages 
that are divided into areas of the page. Where the dashboard module only gave 
four areas--top, bottom, left and right--this one is a completely flexible 
system that includes a couple of 2 column and 3 column layouts by default, but 
is also highly extensible and other layouts can be plugged in with a little HTML 
and CSS knowledge, with just enough PHP knowledge to be able to edit an include 
file without breaking it.

Perhaps most importantly, unlike the dashboard module it requires no fiddling 
with PHP code to include the things you want; the interface lets you add blocks, 
nodes and custom content just by selecting and clicking. 

If you want to override the CSS of a panel, the easiest way is to just copy
the CSS into your theme directory and tweak; panels will look there before
including the CSS from the module, and if it exists, will not include the
module's CSS. If you want to just change a tiny bit but keep the basic
structure, just add your changes to your style.css instead.

If you're having problems with IE and your panels falling below your sidebars,
try setting the width of the main panel area (example, .panel-2col-stacked) to
98%.
