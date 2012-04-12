<?php

/**
 * Returns HTML for the Engage fields overview page.
 *
 * @ingroup themeable
 * @see janrain_capture_field_form()
 */
function theme_janrain_capture_field_form($variables) {
  drupal_add_css(drupal_get_path('module', 'janrain_capture_ui') . '/janrain_capture_ui.css');
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $key) {
    // Skip form control elements.
    if (array_key_exists('path', $form[$key])) {
      $field = &$form[$key];

      // Add the row
      $row = array();
      $row[] = drupal_render($field['title']);
      $row[] = drupal_render($field['path']);
      $row[] = drupal_render($field['edit']);
      $row[] = drupal_render($field['delete']);
      $rows[] = array('data' => $row);
    }
  }

  $header = array(t('Title'), t('Path'));
  $header[] = array('data' => t('Operations'), 'colspan' => 2);

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;

}

/**
 * Theme Engage field mapping form.
 *
 * @ingroup themeable
 */
function theme_janrain_capture_mapping_form($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $key) {
    // Skip form control elements.
    if (array_key_exists('separator', $form[$key])) {
      $field = &$form[$key];

      // Add the row
      $row = array();
      $row[] = drupal_render($field['fid']);
      $row[] = drupal_render($field['separator']);
      $row[] = drupal_render($field['field_set']);
      $row[] = drupal_render($field['field_bundle']);
      $row[] = drupal_render($field['field']);
      $row[] = drupal_render($field['edit']);
      $rows[] = array('data' => $row);
    }
  }
  $header = array(t('Capture Data Field'), '');
  $header[] = array('data' => t('Drupal Profile Field'), 'colspan' => 3);
  $header[] = array('data' => t('Operations'));

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);

  return $output;
}
