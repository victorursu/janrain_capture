<?php

namespace Drupal\janrain_capture\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Entity\Entity\EntityViewMode;
use Drupal\Core\Entity\EntityTypeManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Janrain Capture settings form for Drupal config.
 */
class JanrainCaptureDrupalSettingsForm extends ConfigFormBase {

  /**
   * Entity type manager.
   *
   * @var Drupal\Core\Entity\EntityTypeManager
   */
  protected $entityTypeManager;

  /**
   * JanrainCaptureDrupalSettingsForm constructor.
   *
   * @param \Drupal\Core\Entity\EntityTypeManager $entityTypeManager
   *   Entity type manager.
   */
  public function __construct(EntityTypeManager $entityTypeManager) {
    $this->entityTypeManager = $entityTypeManager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity_type.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return ['janrain_capture.drupal_settings'];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'janrain_capture.drupal_settings.capture';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildForm($form, $form_state);
    $config = $this->config('janrain_capture.drupal_settings');

    $form['capture'] = [
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => $this->t('Janrain Capture Drupal settings'),
    ];
    $view_modes = $this->entityTypeManager->getStorage('entity_view_mode')->getQuery()
      ->condition('targetEntityType', 'user')
      ->execute();
    $view_modes = EntityViewMode::loadMultiple($view_modes);
    $options = [];
    foreach ($view_modes as $id => $view_mode) {
      $id = str_replace('user.', '', $id);
      $options[$id] = $view_mode->label();
    }

    $form['capture']['view_modes'] = [
      '#type' => 'select',
      '#title' => $this->t('User view modes'),
      '#options' => $options,
      '#multiple' => TRUE,
      '#description' => $this->t('Select which view modes to activate Janrain user profile.'),
      '#default_value' => $config->get('capture.view_modes'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);

    $capture = $form_state->getValue('capture');

    $this->configFactory->getEditable('janrain_capture.drupal_settings')
      ->set('capture.view_modes', $capture['view_modes'])
      ->save();
  }

}
