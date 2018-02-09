<?php

namespace Drupal\janrain_capture\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Janrain Capture settings form.
 */
class JanrainCaptureSettingsForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return ['janrain_capture.settings'];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'janrain_capture.settings.capture';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildForm($form, $form_state);
    $config = $this->config('janrain_capture.settings');
    $form['capture'] = [
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => $this->t('Janrain Capture settings'),
    ];
    $form['capture']['app_id'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Application ID'),
      '#description' => $this->t('The Capture application ID that represents a specific capture app.'),
      '#default_value' => $config->get('capture.app_id'),
    ];
    $form['capture']['client_id'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Client ID'),
      '#description' => $this->t('A client ID from your Registration application.'),
      '#default_value' => $config->get('capture.client_id'),
    ];
    $form['capture']['client_secret'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Client Secret'),
      '#description' => $this->t('A client secret from your Registration application'),
      '#default_value' => $config->get('capture.client_secret'),
    ];
    $form['capture']['load_js_url'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Load.js file URL'),
      '#description' => $this->t('Application load file (load.js)'),
      '#default_value' => $config->get('capture.load_js_url'),
    ];
    $form['capture']['capture_server'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Capture server URL'),
      '#description' => $this->t('The URL of the server hosting the Registration application.For example, https://myapp.janraincapture.com.'),
      '#default_value' => $config->get('capture.capture_server'),
    ];

    $form['capture']['federate'] = [
      '#type' => 'details',
      '#title' => $this->t('Federate Settings (optional)'),
      '#collapsible' => TRUE,
      '#open' => $config->get('capture.enable_sso'),
    ];
    $form['capture']['federate']['enable_sso'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Enable SSO'),
      '#description' => $this->t('Enable Single Sign On.'),
      '#default_value' => $config->get('capture.enable_sso'),
    ];
    $form['capture']['federate']['federate_server'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Federate Server'),
      '#description' => $this->t('The URL of the Janrain Single Sign-on server. For example, myapp.janrainsso.com.'),
      '#default_value' => $config->get('capture.federate_server'),
    ];
    $form['capture']['federate']['federate_segment'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Federate segment'),
      '#description' => $this->t('Specifies which SSO segment the current property belongs in.'),
      '#default_value' => $config->get('capture.federate_segment'),
    ];
    $form['capture']['federate']['federate_supported_segments'] = [
      '#type' => 'textarea',
      '#title' => $this->t('SSO Supported Segment Names'),
      '#default_value' => $config->get('capture.federate_supported_segments'),
      '#description' => $this->t('Segments that the current site is allowed to federate with using Single Sign-on.'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    $this->configFactory->getEditable('janrain_capture.settings')
      ->set('capture.app_id', $form_state->getValue('capture')['app_id'])
      ->set('capture.client_id', $form_state->getValue('capture')['client_id'])
      ->set('capture.client_secret', $form_state->getValue('capture')['client_secret'])
      ->set('capture.load_js_url', $form_state->getValue('capture')['load_js_url'])
      ->set('capture.capture_server', $form_state->getValue('capture')['capture_server'])
      ->set('capture.enable_sso', $form_state->getValue('capture')['federate']['enable_sso'])
      ->set('capture.federate_server', $form_state->getValue('capture')['federate']['federate_server'])
      ->set('capture.federate_segment', $form_state->getValue('capture')['federate']['federate_segment'])
      ->set('capture.federate_supported_segments', $form_state->getValue('capture')['federate']['federate_supported_segments'])
      ->save();
  }
}
