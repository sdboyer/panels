<?php
// $Id$
/**
 * @file views-view.tpl.php
 * Main view template
 *
 * Variables available:
 *  -
 *
 */
?>
<div class="dashboard-left">
  <h3 class="dashboard-title">Create new...</h3>
  <div class="dashboard-entry clear-block">
    <div class="dashboard-icon">
      <img src="<?php print $image_path ?>/icon-new-panel-page.png" />
    </div>
    <div class="dashboard-text">
      <div class="dashboard-link">
        <?php print $new_panel_page; ?>
      </div>
      <div class="description">
        <?php print $panel_page_description; ?>
      </div>
    </div>
  </div>

  <div class="dashboard-entry clear-block">
    <div class="dashboard-icon">
      <img src="<?php print $image_path ?>/new-panel-custom.png" />
    </div>
    <div class="dashboard-text">
      <div class="dashboard-link">
        <?php print $new_panel_custom; ?>
      </div>
      <div class="description">
        <?php print $panel_custom_description; ?>
      </div>
    </div>
  </div>

  <div class="dashboard-entry clear-block">
    <div class="dashboard-icon">
      <img src="<?php print $image_path ?>/new-panel-node.png" />
    </div>
    <div class="dashboard-text">
      <div class="dashboard-link">
        <?php print $new_panel_node; ?>
      </div>
      <div class="description">
        <?php print $panel_node_description; ?>
      </div>
    </div>
  </div>

  <div class="dashboard-entry clear-block">
    <div class="dashboard-icon">
      <img src="<?php print $image_path ?>/new-panel-mini.png" />
    </div>
    <div class="dashboard-text">
      <div class="dashboard-link">
        <?php print $new_panel_mini; ?>
      </div>
      <div class="description">
        <?php print $panel_mini_description; ?>
      </div>
    </div>
  </div>

  <h3 class="dashboard-title">Or customize a system page...</h3>
  <div class="dashboard-entry clear-block">
    <div class="dashboard-text">
      <div class="dashboard-link container-inline">
        <?php print $new_panel_override; ?>
      </div>
      <div class="description">
        <?php print $panel_override_description; ?>
      </div>
    </div>
  </div>
</div>

<div class="dashboard-right">
  <div class="dashboard-question">
    What would you like to see in this space? Give your opinion here:
    <a href="http://drupal.org/node/449842">http://drupal.org/node/449842</a>.
  </div>
</div>
