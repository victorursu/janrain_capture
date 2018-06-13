<?php

namespace Drupal\janrain_capture\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Janrain Capture settings form.
 */
class JanrainCaptureSettingsForm extends ConfigFormBase {

  /**
   * The list of identity providers for social login.
   */
  protected const SOCIAL_IDENTITY_PROVIDERS = [
    'amazon' => 'Amazon',
    'aol' => 'AOL',
    'blogger' => 'Blogger',
    'disqus' => 'Disqus',
    'doccheck' => 'DocCheck',
    'doximity' => 'Doximity',
    'facebook' => 'Facebook',
    'flickr' => 'Flickr',
    'fimnet' => 'Fimnet',
    'foursquare' => 'Foursquare',
    'googleplus' => 'Google+',
    'instagram' => 'Instagram',
    'linkedin' => 'LinkedIn',
    'livejournal' => 'LiveJournal',
    'medikey' => 'MediKey',
    'medy' => 'Medy',
    'microsoftaccount' => 'Microsoft Account',
    'mixi' => 'Mixi',
    'mydigipass' => 'MYDIGIPASS.COM',
    'odnoklassniki' => 'Odnoklassniki',
    'onekey' => 'OneKey',
    'openid' => 'OpenID',
    'paypal' => 'PayPal',
    'qq' => 'QQ',
    'renren' => 'Renren',
    'salesforce' => 'Salesforce',
    'sina weibo' => 'Sina Weibo',
    'soundcloud' => 'SoundCloud',
    'tumblr' => 'Tumblr',
    'twitter' => 'Twitter',
    'vk' => 'VK',
    'wechat' => 'WeChat',
    'xing' => 'Xing',
    'yahoo' => 'Yahoo!',
  ];

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
    $form['capture']['providers'] = [
      '#type' => 'details',
      '#open' => FALSE,
      '#title' => $this->t('Social login'),
      '#collapsible' => TRUE,
      '#description' => $this->t('Social login enables users to register on your web site by using an account created with a third-party identity provider (IDP). Read more in the <a href="@documentation">official documentation</a>.', [
        '@documentation' => 'https://docs.janrain.com/social/identity-providers',
      ]),
    ];
    $form['capture']['providers']['list'] = [
      '#type' => 'checkboxes',
      '#title' => $this->t('Identity providers'),
      '#options' => static::SOCIAL_IDENTITY_PROVIDERS,
      '#default_value' => $config->get('capture.providers'),
    ];
    $form['capture']['engage'] = [
      '#type' => 'details',
      '#open' => FALSE,
      '#title' => $this->t('Engage Settings (optional)'),
      '#collapsible' => TRUE,
    ];
    $form['capture']['engage']['app_url'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Engage App URL'),
      '#description' => $this->t('The URL of the Janrain Engage Single Sign-on server. For example, myapp.rpxnow.com.'),
      '#default_value' => $config->get('capture.app_url'),
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

    $capture = $form_state->getValue('capture');

    $this->configFactory->getEditable('janrain_capture.settings')
      ->set('capture.app_id', $capture['app_id'])
      ->set('capture.client_id', $capture['client_id'])
      ->set('capture.client_secret', $capture['client_secret'])
      ->set('capture.load_js_url', $capture['load_js_url'])
      ->set('capture.providers', array_keys(array_filter($capture['providers']['list'])))
      ->set('capture.capture_server', $capture['capture_server'])
      ->set('capture.app_url', $capture['engage']['app_url'])
      ->set('capture.enable_sso', $capture['federate']['enable_sso'])
      ->set('capture.federate_server', $capture['federate']['federate_server'])
      ->set('capture.federate_segment', $capture['federate']['federate_segment'])
      ->set('capture.federate_supported_segments', $capture['federate']['federate_supported_segments'])
      ->save();
  }

}
