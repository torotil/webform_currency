<?php

/**
 * @file
 * Implementation using the webform API to define a webform component.
 */

/**
 * Implements _webform_defaults_[component]().
 */
function _webform_defaults_country_list() {
  $element = array(
    'name'     => 'Country List',
    'form_key' => NULL,
    'value'    => '',
    'extra'    => array(
      'available_countries' => array(),
      'private'             => FALSE,
    ),
  );

  return $element;
}

/**
 * Implements _webform_edit_[component]().
 */
function _webform_edit_country_list($component) {
  include_once DRUPAL_ROOT . '/includes/locale.inc';
  $all_countries    = country_get_list();
  $delta            = 0;
  $nr_all_countries = count($all_countries);
  $form             = array(
    'country_list' => array(
      '#type'     => 'container',
      '#attached' => array(
        'css' => array(drupal_get_path('module', 'webform_country_list') . '/webform_country_list.css'),
      ),
    ),
  );
  $avail_countries = $component['extra']['available_countries'];
  foreach ($avail_countries as $country_code) {
    $avail_countries[$country_code] = &$all_countries[$country_code];
    unset($all_countries[$country_code]);
  }
  $all_countries = $avail_countries + $all_countries;
  foreach ($all_countries as $country_code => $country) {
    $form_entry = &$form['country_list'][$delta];
    $form_entry = array(
      '#type' => 'container',
    );
    $form_entry['available'] = array(
      '#type'          => 'checkbox',
      '#return_value'  => $country_code,
      '#default_value' => isset($avail_countries[$country_code]) ? $country_code : NULL,
      '#attributes'    => array('class' => array('country-list-available')),
    );
    $form_entry['country_code'] = array(
      '#markup' => $country_code,
      '#prefix' => '<span class="country-list-code">',
      '#suffix' => '</span>',
    );
    $form_entry['label'] = array(
      '#markup' => $country,
      '#prefix' => '<span class="country-list-country">',
      '#suffix' => '</span>',
    );

    $form_entry['#delta']  = $delta;
    $form_entry['#weight'] = $delta;
    $form_entry['_weight'] = array(
      '#type'          => 'weight',
      '#title'         => t('Weight for row @number', array('@number' => $delta + 1)),
      '#title_display' => 'invisible',
      '#delta'         => $nr_all_countries,
      '#default_value' => $delta,
      '#weight'        => 100,
    );
    $delta++;
  }
  $form['country_list']['#theme'] = 'country_list_form';
  $form['country_list']['#element_validate'] = array('_webform_country_list_edit_validate');

  return $form;
}

/**
 * Helper function for usort.
 */
function _webform_country_list_cmp_weights($a, $b) {
  return ($a['_weight'] - $b['_weight']);
}

/**
 * Validation function set in _webform_edit_country_list.
 */
function _webform_country_list_edit_validate(&$element, &$form_state) {
  usort($form_state['values']['country_list'], '_webform_country_list_cmp_weights');
  foreach ($form_state['values']['country_list'] as $value) {
    if ($value['available'] !== 0) {
      $form_state['values']['extra']['available_countries'][$value['available']] = $value['available'];
    }
  }
}

/**
 * Implements _webform_render_[component]().
 */
function _webform_render_country_list($component, $value = NULL, $filter = TRUE) {
  // Necessary for country_get_list()
  include_once DRUPAL_ROOT . '/includes/locale.inc';
  $all_countries       = country_get_list();
  $available_countries = array();

  foreach ($component['extra']['available_countries'] as $country_code) {
    $available_countries[$country_code] = $all_countries[$country_code];
  }
  if (empty($available_countries)) {
    $available_countries = $all_countries;
  }
  reset($available_countries);
  $default_ctry = key($available_countries);
  // If the PHP PECL geoip package is available set the default country to that
  // matching the geoip country.
  if (function_exists('geoip_country_code_by_name')) {
    // We use the PHP PECL geoip package to look up the IP address of the
    // user in the geoip IP database in order to find a matching country.
    // We get back the ISO country code for the user.
    $geoip_ctry_code = @geoip_country_code_by_name(ip_address());
    if (isset($available_countries[$geoip_ctry_code])) {
      $default_ctry = $geoip_ctry_code;
    }
  }
  $element = array(
    '#type'          => 'select',
    '#title'         => $component['name'],
    '#description'   => isset($component['extra']['description']) ? check_plain($component['extra']['description']) : NULL,
    '#options'       => $available_countries,
    '#default_value' => $default_ctry,
    '#weight'        => isset($component['weight']) ? $component['weight'] : 0,
  );

  return $element;
}

/**
 * Implements _webform_display_[component]().
 */
function _webform_display_country_list($component, $value, $format = 'html') {
  return array(
    '#title'          => $component['name'],
    '#weight'         => $component['weight'],
    '#theme'          => 'webform_display_country_list',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render'    => array('webform_element_wrapper'),
    '#component'      => $component,
    '#format'         => $format,
    '#value'          => isset($value) ? $value[0] : '',
  );
}

/**
 * Implements _webform_theme_[component]().
 */
function _webform_theme_country_list() {
  return array(
    'country_list_form' => array(
      'render element' => 'element',
    ),
    'webform_display_country_list' => array(
      'render element' => 'element',
    ),
  );
}

/**
 * Implements _webform_table_[component]().
 */
function _webform_table_country_list($component, $value) {
  return !empty($value[0]) ? check_plain($value[0]) : '';
}

/**
 * Implements _webform_csv_headers_[component]().
 */
function _webform_csv_headers_country_list($component, $export_options) {
  $header = array();

  $header[0] = array('');
  $header[1] = array('');
  $header[2] = $component['name'];

  return $header;
}

/**
 * Implements _webform_csv_data_[component]().
 */
function _webform_csv_data_country_list($component, $export_options, $value) {
  return isset($value[0]) ? $value[0] : '';
}