<?php
// Only valid if views module loaded.
if (module_exists('views')) {
  /**
   * Callback function to supply a list of content types.
   */
  function panels_views_panels_content_types() {
    $items['views'] = array(
      'callback' => 'panels_content_views',
      'admin' => 'panels_admin_views',
    );
    return $items;
  }

  /**
   * Output function for the 'views' content type. Outputs a views
   * based on the module and delta supplied in the configuration.
   */
  function panels_content_views($conf) {
    $view = views_get_view($conf['view']);
    if ($view) {
      $arguments = explode('/', $_GET['q']);
      $args = $conf['args'];
      
      foreach ($arguments as $id => $arg) {
        $args = str_replace("%$id", $arg, $args);
      }

      $args = preg_replace('/\%\d/', '', $args);
      $args = $args ? explode('/', $args) : array();

      if ($conf['url']) {
        $view->url = $conf['url'];
      }

      $output = views_build_view($conf['type'], $view, $args, intval($conf['pager_id']), intval($conf['nodes_per_page']));
    }
    return $output;
  }

  /** 
   * Callback to perform administrative functions on the content views
   */
  function panels_admin_views($op, &$arg, $arg2 = NULL) {
    switch ($op) {
      case 'list':
        $conf = $arg;
        $view = views_get_view($conf['view']);
        return '<strong>Views</strong>: ' . $view->name . ' (' . $view->description . ')';
      case 'add button':
        $result = db_query("SELECT name, description FROM {view_view}");
        while ($view = db_fetch_object($result)) {
          $views[$view->name] = $view->name . ': ' . $view->description;
        }

        $default_views = _views_get_default_views();
        $views_status = variable_get('views_defaults', array());
        foreach ($default_views as $view) {
          if (!$views[$view->name] && 
            ($views_status[$view->name] == 'enabled' || (!$views_status[$view->name] && !$view->disabled))) {
            $views[$view->name] = check_plain($view->name . ': ' . $view->description);
          }
        }

        ksort($views);

        $form['view'] = array(
          '#type' => 'select',
          '#options' => $views,
          '#title' => t('Choose a view from the views module'),
        );
        $form['submit'] = array(
          '#type' => 'button',
          '#value' => t('Add view'),
        );

        $form['#prefix'] = '<div class="container-inline">';
        $form['#suffix'] = '</div>';
        return $form;
      case 'add':
        if ($_POST['op'] != t('Add view')) {
          return;
        }
        $conf = $arg;
        $view = views_get_view($conf['view']);
        if ($view->page) {
          $conf['type'] = 'page';
          $conf['nodes_per_page'] = $view->nodes_per_page;
        }
        else {
          $conf['type'] = 'block';
          $conf['nodes_per_page'] = $view->nodes_per_block;
        }
        $conf['pager_id'] = 0;
        return $conf;
      case 'edit':
        $conf = &$arg;
        $form['view'] = array(
          '#type' => 'hidden',
          '#default_value' => $conf['view'],
        );
        $form['type'] = array(
          '#type' => 'select',
          '#default_value' => $conf['type'],
          '#title' => t('View type'),
          '#description' => t('Select which type of the view to display.'),
          '#options' => array('page' => t('Page'), 'block' => t('Block'), 'embed' => t('Embedded')),
        );
        $form['pager_id'] = array(
          '#type' => 'textfield',
          '#default_value' => $conf['pager_id'],
          '#title' => t('Pager ID'),
          '#size' => 4,
          '#description' => t('Select the numeric pager ID to use, or 0 to not have use paging. Select "1" if you aren\'t sure what this means'),
        );

        $form['nodes_per_page'] = array(
          '#type' => 'textfield',
          '#default_value' => $conf['nodes_per_page'],
          '#title' => t('Posts to Display'),
          '#size' => 4,
          '#description' => t('Select the number of posts to display, or 0 to display all results.'),
        );

        $form['args'] = array(
          '#type' => 'textfield',
          '#default_value' => $conf['args'],
          '#title' => t('View arguments'),
          '#size' => 12,
          '#description' => t('Arguments to send to the view as if they were part of the URL in the form of arg1/arg2/arg3. You can use %0, %1, %2, etc, to use arguments from the actual URL. For example, if your panel URL is foo/bar, and someone hits foo/bar/5 use %2 to get the 5.'),
        );

        $form['url'] = array(
          '#type' => 'textfield',
          '#default_value' => $conf['url'],
          '#title' => t('Override URL'),
          '#size' => 12,
          '#description' => t('If this is set, override the View URL; this can sometimes be useful to set to the panel URL'),
        );

        return $form;
      case 'validate':
        // This one has nothing to validate.
        $form_values = &$arg;
        $form = $arg2;
        return;
      case 'save':
        // For this one, the form values go directly into the config.
        $form = &$arg;
        return $form;
    }
  }

}