<?php

namespace Drupal\janrain_capture\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Janrain Screens settings form.
 */
class JanrainScreensSettingsForm extends ConfigFormBase {

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
    return 'janrain_capture.settings.screens';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildForm($form, $form_state);
    $config = $this->config('janrain_capture.settings');
    $form['screens_folder'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Screens Folder'),
      '#description' => $this->t('URL of the Capture screens folder<br/>(examples: file:///sites/all/themes/janrain-capture-screens/, http://example.com/capture-screens/)'),
      '#default_value' => $config->get('screens.folder'),
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    $this->configFactory->getEditable('janrain_capture.settings')
      ->set('screens.folder', $form_state->getValue('screens_folder'))
      ->save();
  }
}
