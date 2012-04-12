<?php
/**
 * @file
 * Admin settings menu
 */

/**
 * Menu callback: Generate a form to configure Drupal profile to Capture mappings
 * used to pre-fill User and Profile2 fields with Capture data at signup
 * and import Capture profile data when a 3rd party account is linked.
 *
 * @ingroup forms
 * @see janrain_capture_mapping_form_submit()
 */
function janrain_capture_mapping_form($form, &$form_state) {
  $catalog = _janrain_capture_drupal_field_catalog();
  $map = _janrain_capture_get_field_map();
  $map[] = array();

  $capture_fields = _janrain_capture_get_mappable_capture_fields();

  foreach ($map as $mid => $mapping) {
    $form[$mid] = array(
      'fid' => array(
         '#type' => 'select',
         '#title' => t('Capture Field'),
         '#title_display' => 'invisible',
         '#options' => _janrain_capture_field_options(),
         '#empty_option' => t('- Select a data field -'),
         '#description' => t('Data path.'),
         '#attributes' => array('class' => array('janrain-capture-field-select', 'mid-' . $mid)),
      ),
      'separator' => array(
        '#markup' => '=>',
      ),
      'field_set' => array(
        '#type' => 'select',
        '#title' => t('Fieldset'),
        '#title_display' => 'invisible',
        '#options' => _janrain_capture_drupal_field_options($catalog, 'set'),
        '#empty_option' => t('- Select a fieldset -'),
        '#description' => t('Module or entity.'),
        '#attributes' => array('class' => array('field-set-select', 'mid-' . $mid)),
      ),
      'field_bundle' => array(
        '#type' => 'select',
        '#title' => t('Fieldset type'),
        '#title_display' => 'invisible',
        '#options' => _janrain_capture_drupal_field_options($catalog, 'bundle'),
        '#empty_option' => t('- Select a type -'),
        '#description' => t('Fieldset type.'),
        '#attributes' => array('class' => array('field-bundle-select', 'mid-' . $mid)),
      ),
      'field' => array(
        '#type' => 'select',
        '#title' => t('Field'),
        '#title_display' => 'invisible',
        '#options' => $catalog,
        '#empty_option' => t('- Select a field -'),
        '#description' => t('Field.'),
        '#attributes' => array('class' => array('field-select', 'mid-' . $mid)),
      ),
    );
  }

  if (count($form)) {
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));
  }

  $form['#attached']['js'][] = drupal_get_path('module', 'janrain_capture') . '/janrain_capture.js';
  $form['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('catalog' => $catalog, 'map' => $map, 'janrain_capture_fields' => $capture_fields),
  );
  $form['#tree'] = TRUE;

  return $form;
}

/**
 * Menu callback: Generate a form to configure available Capture fields
 * that may be used to map to Drupal fields.
 *
 * @ingroup forms
 * @see _janrain_capture_field_forms_submit()
 */
function janrain_capture_field_form($form, &$form_state) {
  $janrain_fields = _janrain_capture_get_mappable_capture_fields(array('fid', 'path', 'title'));
  $form = array();

  foreach ($janrain_fields as $key => $field) {
    $edit_field_path = JANRAIN_CAPTURE_PATH_FIELD_EDIT . DIRECTORY_SEPARATOR . $field['fid'];
    $delete_field_path = JANRAIN_CAPTURE_PATH_FIELD_DELETE . DIRECTORY_SEPARATOR . $field['fid'];

    $form[$field['fid']] = array(
      'title' => array(
        '#markup' => $field['title'],
      ),
      'path' => array(
        '#markup' => $field['path'],
      ),
      'edit' => array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => $edit_field_path,
      ),
      'delete' => array(
        '#type' => 'link',
        '#title' => t('delete'),
        '#href' => $delete_field_path,
      ),
    );
  }

  // Additional row: add new field.
  $form['_edit_field'] = array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('New field title'),
      '#title_display' => 'invisible',
      '#size' => 15,
      '#attributes' => array('class' => array('janrain_capture-field-title-input')),
      '#description' => t('Title'),
      '#prefix' => '<div class="add-new-placeholder">' . t('Add new field') . '</div>',
    ),
    'path' => array(
      '#type' => 'textfield',
      '#title' => t('New field path'),
      '#title_display' => 'invisible',
      '#size' => 30,
      '#attributes' => array('class' => array('janrain_capture-path-input')),
      '#description' => t('Capture data path'),
      '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save new field'),
  );
  $form['actions']['reset'] = array(
    '#type' => 'link',
    '#title' => t('Reset to defaults'),
    '#href' => JANRAIN_CAPTURE_PATH_FIELD_RESET,
  );

  $form['#tree'] = TRUE;
  $form['#validate'][] = '_janrain_capture_field_forms_validate';
  $form['#submit'][] = '_janrain_capture_field_forms_submit';

  return $form;
}

/**
 * Menu callback: Generate a form to edit a Capture profile field.
 *
 * @ingroup forms
 * @see _janrain_capture_field_forms_validate(), _janrain_capture_field_forms_submit()
 */
function janrain_capture_field_edit_form($form, &$form_state, $arg = NULL) {
  if (is_numeric($arg)) {
    $fid = $arg;

    $field = _janrain_capture_get_mappable_capture_fields(array('title', 'path'), array('fid' => $fid));
    $field = array_shift($field);

    if (!$field) {
      drupal_not_found();
      drupal_exit();
    }

    drupal_set_title(t('Edit %title Capture field', array('%title' => $field['title'])), PASS_THROUGH);

    $form['_edit_field']['fid'] = array(
      '#type' => 'value',
      '#value' => $fid,
    );
  }
  else {
    drupal_not_found();
    drupal_exit();
  }

  $form['_edit_field']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $field['title'],
    '#attributes' => array('class' => array('janrain_capture-field-title-input')),
    '#description' => t('The title of the field. The title is shown in the mapping form next to the data path. An example title is "Verified email".'),
  );

  $form['_edit_field']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Capture data path'),
    '#default_value' => $field['path'],
    '#attributes' => array('class' => array('janrain_capture-path-input')),
    '#description' => t("The path to the data within the Capture authentication dataset, in a PHP-like array referencing notation. An example path is <code>['profile']['verifiedEmail']</code> or <code>['merged_poco']['emails'][0]['value']</code>."),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field'),
  );

  $form['#tree'] = TRUE;

  $form['#validate'][] = '_janrain_capture_field_forms_validate';
  $form['#submit'][] = '_janrain_capture_field_forms_submit';

  return $form;
}

/**
 * Menu callback: Generate a form to delete a specific Capture field from
 * the list of available Capture fields. This will also delete any mappings
 * defined for that field.
 *
 * @ingroup forms
 * @see janrain_capture_field_delete_form_submit()
 */
function janrain_capture_field_delete_form($form, &$form_state, $fid = NULL) {
  $field = _janrain_capture_get_mappable_capture_fields(array('title'), array('fid' => $fid));
  $field = array_shift($field);

  if (!$field) {
    drupal_not_found();
    drupal_exit();
  }

  $form['fid'] = array('#type' => 'value', '#value' => $fid);
  $form['title'] = array('#type' => 'value', '#value' => $field['title']);

  return confirm_form($form,
    t('Are you sure you want to delete the Capture field %field?', array('%field' => $field['title'])), JANRAIN_CAPTURE_PATH_FIELDS,
    t('This action cannot be undone. The Drupal to Capture data mappings using this field will be deleted as well.'),
    t('Delete'), t('Cancel'));
}


/**
 * Submit handler to update changed Drupal to Capture data mapping.
 *
 * @see janrain_capture_mapping_form()
 */
function janrain_capture_mapping_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $old_map = _janrain_capture_get_field_map();
  $map = array();

  foreach (element_children($values) as $mid) {
    if (is_numeric($mid)) {
      // All field mappings should include a bundle, except for legacy
      // (non-fieldable) entity fields.
      if (!empty($values[$mid]['fid']) &&
          !empty($values[$mid]['field_set']) &&
          (!empty($values[$mid]['field_bundle']) || $values[$mid]['field_set'] == 'profile') &&
          !empty($values[$mid]['field'])) {

        $map[$mid] = array(
          'fid' => $values[$mid]['fid'],
          'set' => $values[$mid]['field_set'],
          'field' =>  $values[$mid]['field'],
          'update' => isset($old_map[$mid]['update']) ? $old_map[$mid]['update'] : 0,
        );

        $map[$mid]['bundle'] = ($values[$mid]['field_set'] == 'profile') ? '' : $values[$mid]['field_bundle'];
        if (isset($old_map[$mid]['providers'])) {
          $map[$mid]['providers'] = $old_map[$mid]['providers'];
        }
      }
      else {
      }
    }
  }

  _janrain_capture_set_field_map($map);
  drupal_set_message(t('Profile to Capture data mapping has been updated.'));
}

/**
 * Process janrain_capture_field_form and janrain_capture_field_edit_form submissions.
 *
 * @see janrain_capture_field_form(), janrain_capture_field_edit_form()
 */
function _janrain_capture_field_forms_submit($form, &$form_state) {
  $field = $form_state['values']['_edit_field'];

  // Remove all elements that are not janrain_capture_profile_field columns.
  $values = array_intersect_key($field, array_flip(array('title', 'path', 'parsed_path', 'fid')));

  $message = t('The field has been updated');

  _janrain_capture_add_mappable_capture_field($values);
  if (!isset($field['fid'])) {
    drupal_set_message(t('The field has been created'));
  }
  else {
    drupal_set_message(t('The field has been updated'));
  }

  $form_state['redirect'] = JANRAIN_CAPTURE_PATH_FIELDS;
  return;
}

/**
 * Process a Capture field delete form submission.
 *
 * @see janrain_capture_field_delete_form()
 */
function janrain_capture_field_delete_form_submit($form, &$form_state) {
  _janrain_capture_delete_mappable_capture_field($form_state['values']['fid']);

  drupal_set_message(t('The Janrain Capture field %field has been deleted.', array('%field' => $form_state['values']['title'])));

  $form_state['redirect'] = JANRAIN_CAPTURE_PATH_FIELDS;

  return;
}


/**
 * Validate janrain_capture_field_form and janrain_capture_field_edit_form submissions.
 *
 * @see janrain_capture_field_form(), janrain_capture_field_edit_form()
 */
function _janrain_capture_field_forms_validate($form, &$form_state) {
  $field = $form_state['values']['_edit_field'];

  // Missing title.
  if (!$field['title']) {
    form_set_error('_edit_field][title', t('You need to provide a title.'));
  }

  // Missing data path.
  if (!$field['path']) {
    form_set_error('_edit_field][path', t('You need to provide a data path.'));
  }

  // Validate the data path by parsing it, also save parsed path to values.
  $parsed_path = _janrain_capture_parse_path($field['path']);
  if ($parsed_path) {
    $form_state['values']['_edit_field']['parsed_path'] = $parsed_path;
  }
  else {
    _janrain_capture_path_validate_error();
  }

  $fid = (isset($field['fid'])) ? $field['fid'] : 0;

  $title = _janrain_capture_get_mappable_capture_fields(array('fid'), array('title' => $field['title']));
  unset($title[$fid]);

  if (!empty($title)) {
    form_set_error('_edit_field][title', t('The specified title is already in use.'));
  }

  $path = _janrain_capture_get_mappable_capture_fields(array('fid'), array('path' => $field['path']));
  unset($path[$fid]);

  if (!empty($path)) {
    form_set_error('_edit_field][path', t('The specified path is already in use.'));
  }
}

/**
 * Create a generic error for validation ... TODO: Why is this here?
 */
function _janrain_capture_path_validate_error() {
  form_set_error('_edit_field][path', t('The specified path is not valid. Please check that it does not contain spaces or any other special characters except brackets ([]) and apostrophe (\'), and it would make a valid PHP array reference if appended to an array variable.'));
}


function janrain_capture_settings($form, &$form_state) {

  $janrain_capture_main = variable_get('janrain_capture_main', array());

  $form['janrain_capture_main'] = array(
    '#type' => 'fieldset',
    '#title' => t('Janrain Capture Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_main']['capture_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Client ID'),
    '#default_value' => !empty($janrain_capture_main['capture_client_id'])
      ? $janrain_capture_main['capture_client_id']
      : '',
    '#description' => t('Your Janrain Capture Client ID')
  );
  $form['janrain_capture_main']['capture_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Client Secret'),
    '#default_value' => !empty($janrain_capture_main['capture_client_secret'])
      ? $janrain_capture_main['capture_client_secret']
      : '',
    '#description' => t('Your Janrain Capture Client Secret')
  );
  $form['janrain_capture_main']['capture_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture Address'),
    '#default_value' => !empty($janrain_capture_main['capture_address'])
      ? $janrain_capture_main['capture_address']
      : '',
    '#description' => t('Your Janrain Capture Address (e.g. myapp.janraincapture.com)')
  );

  $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  $form['janrain_capture_optional'] = array(
    '#type' => 'fieldset',
    '#title' => t('Optional Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['janrain_capture_optional']['capture_sso_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain Capture SSO Address'),
    '#default_value' => !empty($janrain_capture_optional['capture_sso_address'])
      ? $janrain_capture_optional['capture_sso_address']
      : '',
    '#description' => t('Your Janrain Capture SSO Address (e.g. myapp.janrainsso.com)')
  );
  $form['janrain_capture_optional']['captureui_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Janrain CaptureUI Address'),
    '#default_value' => !empty($janrain_capture_optional['captureui_address'])
      ? $janrain_capture_optional['captureui_address']
      : '',
    '#description' => t('Your Janrain CaptureUI Address (defaults to Janrain Capture Address)')
  );

  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $form['janrain_capture_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_fields']['capture_name_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile Name Field'),
    '#default_value' => !empty($janrain_capture_fields['capture_name_field'])
      ? $janrain_capture_fields['capture_name_field']
      : 'email',
    '#description' => t('The profile field to use for the local username. Must be unique.')
  );
  $form['janrain_capture_fields']['capture_match_email'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match Users on Email'),
    '#default_value' => !empty($janrain_capture_fields['capture_match_email'])
      ? $janrain_capture_fields['capture_match_email']
      : 0,
    '#description' => t('Match authenticated Capture users with a local record with the same email')
  );
  $form['janrain_capture_fields']['capture_enforce_verification'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enforce Email Verification'),
    '#default_value' => !empty($janrain_capture_fields['capture_enforce_verification'])
      ? $janrain_capture_fields['capture_enforce_verification']
      : 0,
    '#description' => t('Prompt users who have not verified their email address to do so before completing authentication')
  );

  $janrain_capture_dependencies = variable_get('janrain_capture_dependencies', array());
  $form['janrain_capture_dependencies'] = array(
    '#type' => 'fieldset',
    '#title' => t('Dependency Locations'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
  );
  $form['janrain_capture_dependencies']['capture_json2'] = array(
    '#type' => 'textfield',
    '#title' => t('JSON2.js Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_json2'])
      ? $janrain_capture_dependencies['capture_json2']
      : 'sites/all/libraries/json2/json2.js',
    '#description' => t('The path to json2.js releative to your Drupal installation root')
  );
  $form['janrain_capture_dependencies']['capture_fancy_js'] = array(
    '#type' => 'textfield',
    '#title' => t('Fancybox JS Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_fancy_js'])
      ? $janrain_capture_dependencies['capture_fancy_js']
      : 'sites/all/libraries/fancybox/jquery.fancybox-1.3.4.pack.js',
    '#description' => t('The path to the Fancybox javascript library releative to your Drupal installation root')
  );
  $form['janrain_capture_dependencies']['capture_fancy_css'] = array(
    '#type' => 'textfield',
    '#title' => t('Fancybox CSS Location'),
    '#default_value' => !empty($janrain_capture_dependencies['capture_fancy_css'])
      ? $janrain_capture_dependencies['capture_fancy_css']
      : 'sites/all/libraries/fancybox/jquery.fancybox-1.3.4.css',
    '#description' => t('The path to the Fancybox css stylesheet releative to your Drupal installation root')
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );


  return system_settings_form($form);
}
