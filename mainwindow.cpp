/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QMessageBox>
#include <QStringListModel>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QSslCipher>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslError>
#include <QUrlQuery>
#include <QProgressDialog>
#include <QSysInfo>

#include "util.h"
#include "progressdialog.h"

#include "cryptopp562/aes.h"
#include "cryptopp562/ccm.h"
#include "cryptopp562/filters.h"

#ifdef QT_DEBUG
#include <QtDebug>
#include "testpbkdf2.h"
#endif

static const QString APP_COMPANY_NAME = "c't";
static const QString APP_NAME = "ctpwdgen";
static const QString APP_VERSION = "1.0-ALPHA";
static const QString APP_URL = "https://github.com/ola-ct/ctpwdgen";
static const QString APP_AUTHOR = "Oliver Lau";
static const QString APP_AUTHOR_MAIL = "ola@ct.de";
static const QString APP_USER_AGENT = QString("%1/%2 (+%3) Qt/%4 (%5; %6)")
    .arg(APP_NAME)
    .arg(APP_VERSION)
    .arg(APP_URL)
    .arg(qVersion())
    .arg(QSysInfo::prettyProductName())
    .arg(QSysInfo::currentCpuArchitecture());

static const int DEFAULT_MASTER_PASSWORD_INVALIDATION_TIME_MINS = 5;
static const int AES_KEY_SIZE = 256 / 8;
static const unsigned char IV[16] = {0xb5, 0x4f, 0xcf, 0xb0, 0x88, 0x09, 0x55, 0xe5, 0xbf, 0x79, 0xaf, 0x37, 0x71, 0x1c, 0x28, 0xb6};
static const int NO_CRYPT_ERROR = -1;
static const bool COMPRESSION_ENABLED = true;

static const QString DEFAULT_SERVER_ROOT = "https://localhost/ctpwdgen-server";
static const QString DEFAULT_WRITE_URL = "/ajax/write.php";
static const QString DEFAULT_READ_URL = "/ajax/read.php";


class MainWindowPrivate {
public:
  MainWindowPrivate(QWidget *parent = nullptr)
    : settings(QSettings::IniFormat, QSettings::UserScope, APP_COMPANY_NAME, APP_NAME)
    , loaderIcon(":/images/loader.gif")
    , trayIcon(QIcon(":/images/ctpwdgen.ico"), parent)
    , customCharacterSetDirty(false)
    , parameterSetDirty(false)
    , autoIncreaseIterations(true)
    , completer(nullptr)
    , masterPasswordDialog(new CredentialsDialog(parent))
    , optionsDialog(new OptionsDialog(parent))
    , progressDialog(nullptr)
    , sslConf(QSslConfiguration::defaultConfiguration())
    , readNAM(new QNetworkAccessManager(parent))
    , readReq(nullptr)
    , writeNAM(new QNetworkAccessManager(parent))
    , writeReq(nullptr)
    , counter(0)
    , maxCounter(0)
  { /* ... */  }
  ~MainWindowPrivate()
  {
    safeDelete(completer);
  }
  CredentialsDialog *masterPasswordDialog;
  OptionsDialog *optionsDialog;
  QSettings settings;
  QVariantMap domains;
  QMovie loaderIcon;
  bool customCharacterSetDirty;
  bool parameterSetDirty;
  bool autoIncreaseIterations;
  QCompleter *completer;
  Password password;
  Password cryptPassword;
  QDateTime createdDate;
  QDateTime modifiedDate;
  QSystemTrayIcon trayIcon;
  QString masterPassword;
  QTimer masterPasswordInvalidationTimer;
  unsigned char AESKey[AES_KEY_SIZE];
  ProgressDialog *progressDialog;
  QList<QSslCertificate> cert;
  QList<QSslError> expectedSslErrors;
  QSslConfiguration sslConf;
  QNetworkAccessManager *readNAM;
  QNetworkReply *readReq;
  QNetworkAccessManager *writeNAM;
  QNetworkReply *writeReq;
  int counter;
  int maxCounter;
};


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , d_ptr(new MainWindowPrivate)
{
  Q_D(MainWindow);

  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctpwdgen.ico"));
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->userLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->forceRegexPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->useDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->avoidAmbiguousCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->forceLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceRegexCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(updatePassword()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(customCharacterSetChanged()));
  QObject::connect(ui->forceRegexPlainTextEdit, SIGNAL(textChanged()), SLOT(updateValidator()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->useDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(customCharacterSetCheckBoxToggled(bool)));
  QObject::connect(ui->avoidAmbiguousCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->copyPasswordToClipboardPushButton, SIGNAL(pressed()), SLOT(copyPasswordToClipboard()));
  QObject::connect(ui->savePushButton, SIGNAL(pressed()), SLOT(saveCurrentSettings()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(stopPasswordGeneration()));
  QObject::connect(&d->password, SIGNAL(generated()), SLOT(onPasswordGenerated()));
  QObject::connect(&d->password, SIGNAL(generationAborted()), SLOT(onPasswordGenerationAborted()), Qt::ConnectionType::QueuedConnection);
  QObject::connect(&d->password, SIGNAL(generationStarted()), SLOT(onPasswordGenerationStarted()), Qt::ConnectionType::QueuedConnection);
  QObject::connect(ui->actionNewDomain, SIGNAL(triggered(bool)), SLOT(newDomain()));
  QObject::connect(ui->actionSyncNow, SIGNAL(triggered(bool)), SLOT(sync()));
  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));
  QObject::connect(this, SIGNAL(reenterCredentials()), SLOT(enterMasterPassword()));
  QObject::connect(ui->actionOptions, SIGNAL(triggered(bool)), d->optionsDialog, SLOT(show()));
  QObject::connect(d->optionsDialog, SIGNAL(accepted()), SLOT(saveSettings()));
  QObject::connect(d->optionsDialog, SIGNAL(certificatesUpdated()), SLOT(loadCertificate()));
  QObject::connect(d->masterPasswordDialog, SIGNAL(accepted()), SLOT(masterPasswordEntered()));
  QObject::connect(&d->masterPasswordInvalidationTimer, SIGNAL(timeout()), SLOT(invalidatePassword()));

  d->progressDialog = new ProgressDialog(this);
  QObject::connect(d->progressDialog, SIGNAL(cancelled()), SLOT(cancelServerOperation()));

  QObject::connect(&d->loaderIcon, SIGNAL(frameChanged(int)), SLOT(updateSaveButtonIcon(int)));
  QObject::connect(d->readNAM, SIGNAL(finished(QNetworkReply*)), SLOT(readFinished(QNetworkReply*)));
  QObject::connect(d->readNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));
  QObject::connect(d->writeNAM, SIGNAL(finished(QNetworkReply*)), SLOT(writeFinished(QNetworkReply*)));
  QObject::connect(d->writeNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));

#ifdef QT_DEBUG
  QObject::connect(ui->actionInvalidatePassword, SIGNAL(triggered(bool)), SLOT(invalidatePassword()));
#endif

#ifdef QT_DEBUG
  ui->avoidAmbiguousCheckBox->setChecked(true);
#endif

#ifndef QT_DEBUG
  ui->hashPlainTextEdit->setVisible(false);
#endif

  d->optionsDialog->setSalt(PasswordParam::DefaultSalt);
  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&d->loaderIcon);
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  d->masterPasswordInvalidationTimer.setSingleShot(true);
  d->masterPasswordInvalidationTimer.setTimerType(Qt::VeryCoarseTimer);
  updateUsedCharacters();
  updateValidator();
  setDirty(false);


#ifdef QT_DEBUG
  TestPBKDF2 tc;
  QTest::qExec(&tc, 0, 0);
#endif

  emit reenterCredentials();

  d->trayIcon.show();
  QObject::connect(&d->trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
  QMenu *trayMenu = new QMenu(APP_NAME);
  QAction *actionShow = trayMenu->addAction(tr("Restore window"));
  QObject::connect(actionShow, SIGNAL(triggered(bool)), SLOT(showHide()));
  QAction *actionSync = trayMenu->addAction(tr("Sync"));
  QObject::connect(actionSync, SIGNAL(triggered(bool)), SLOT(sync()));
  QAction *actionClearClipboard = trayMenu->addAction(tr("Clear clipboard"));
  QObject::connect(actionClearClipboard, SIGNAL(triggered(bool)), SLOT(clearClipboard()));
  QAction *actionAbout = trayMenu->addAction(tr("About %1").arg(APP_NAME));
  QObject::connect(actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QAction *actionQuit = trayMenu->addAction(tr("Quit"));
  QObject::connect(actionQuit, SIGNAL(triggered(bool)), SLOT(close()));
  d->trayIcon.setContextMenu(trayMenu);
}


void MainWindow::showHide(void) {
  if (isVisible()) {
    hide();
  }
  else {
    show();
    raise();
    setFocus();
  }
}


void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::DoubleClick) {
    showHide();
  }
}


MainWindow::~MainWindow()
{
  d_ptr->trayIcon.hide();
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  Q_D(MainWindow);
  stopPasswordGeneration();
  int rc = (d->parameterSetDirty)
      ? QMessageBox::question(
          this,
          tr("Save before exit?"),
          tr("Your domain parameters have changed. Do you want to save the changes before exiting?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes)
      : QMessageBox::NoButton;
  if (rc == QMessageBox::Yes) {
    saveCurrentSettings();
  }
  else if (rc == QMessageBox::Cancel) {
    e->ignore();
  }
  else {
    d->masterPasswordDialog->close();
    saveSettings();
    invalidatePassword(false);
    QMainWindow::closeEvent(e);
    e->accept();
  }
}


void MainWindow::changeEvent(QEvent *e)
{
  switch (e->type()) {
  case QEvent::LanguageChange:
  {
    ui->retranslateUi(this);
    break;
  }
  case QEvent::WindowStateChange:
  {
    if (windowState() & Qt::WindowMinimized) {
      QTimer::singleShot(200, this, SLOT(hide()));
    }
    break;
  }
  default:
    break;
  }
}


void MainWindow::newDomain(void)
{
  DomainSettings domainSettings;
  ui->domainLineEdit->setText(QString());
  ui->domainLineEdit->setFocus();
  ui->useLowerCaseCheckBox->setChecked(domainSettings.useLowerCase);
  ui->useUpperCaseCheckBox->setChecked(domainSettings.useUpperCase);
  ui->useDigitsCheckBox->setChecked(domainSettings.useDigits);
  ui->useExtrasCheckBox->setChecked(domainSettings.useExtra);
  ui->iterationsSpinBox->setValue(domainSettings.iterations);
  ui->passwordLengthSpinBox->setValue(domainSettings.length);
  ui->forceRegexPlainTextEdit->setPlainText(domainSettings.validatorRegEx.pattern());
  ui->forceRegexCheckBox->setChecked(domainSettings.forceRegexValidation);
  updateValidator();
  updatePassword();
}


void MainWindow::setDirty(bool dirty)
{
  Q_D(MainWindow);
  d->parameterSetDirty = dirty;
  updateWindowTitle();
}


void MainWindow::restartInvalidationTimer(void)
{
  Q_D(MainWindow);
  d->masterPasswordInvalidationTimer.start(d->optionsDialog->masterPasswordInvalidationTimeMins() * 60 * 1000);
}


void MainWindow::onPasswordGenerationStarted(void)
{
  Q_D(MainWindow);
  ui->copyPasswordToClipboardPushButton->setEnabled(false);
  ui->processLabel->show();
  ui->cancelPushButton->show();
  d->loaderIcon.start();
}


void MainWindow::updatePassword(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::updatePassword()";
  bool validConfiguration = false;
  ui->statusBar->showMessage(QString());
  if (ui->customCharactersPlainTextEdit->toPlainText().count() > 0 && !d->masterPassword.isEmpty()) {
    validConfiguration = true;
    stopPasswordGeneration();
    generatePassword();
    restartInvalidationTimer();
  }
  if (!validConfiguration) {
    ui->generatedPasswordLineEdit->setText(QString());
    ui->hashPlainTextEdit->setPlainText(QString());
  }
}


void MainWindow::updateUsedCharacters(void)
{
  if (!ui->useCustomCheckBox->isChecked()) {
    stopPasswordGeneration();
    QString passwordCharacters;
    if (ui->useLowerCaseCheckBox->isChecked())
      passwordCharacters += PasswordParam::LowerChars;
    if (ui->useUpperCaseCheckBox->isChecked())
      passwordCharacters += ui->avoidAmbiguousCheckBox->isChecked() ? PasswordParam::UpperCharsNoAmbiguous : PasswordParam::UpperChars;
    if (ui->useDigitsCheckBox->isChecked())
      passwordCharacters += PasswordParam::Digits;
    if (ui->useExtrasCheckBox->isChecked())
      passwordCharacters += PasswordParam::ExtraChars;
    ui->customCharactersPlainTextEdit->blockSignals(true);
    ui->customCharactersPlainTextEdit->setPlainText(passwordCharacters);
    ui->customCharactersPlainTextEdit->blockSignals(false);
  }
  updateValidator();
}


void MainWindow::generatePassword(void)
{
  Q_D(MainWindow);
  d->password.generateAsync(
        PasswordParam(
          ui->domainLineEdit->text().toUtf8(),
          d->masterPassword.toUtf8(),
          ui->customCharactersPlainTextEdit->toPlainText(),
          ui->passwordLengthSpinBox->value(),
          ui->iterationsSpinBox->value()
          )
        );
}


void MainWindow::stopPasswordGeneration(void)
{
  Q_D(MainWindow);
  if (d->password.isRunning()) {
    d->password.abortGeneration();
    d->password.waitForFinished();
  }
}


void MainWindow::onPasswordGenerated(void)
{
  Q_D(MainWindow);
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  d->loaderIcon.stop();
  ui->copyPasswordToClipboardPushButton->setEnabled(true);
  bool setKey = true;
  if (ui->forceRegexCheckBox->isChecked() && !d->password.isValid())
    setKey = false;
  if (setKey) {
    ui->generatedPasswordLineEdit->setText(d->password.key());
    ui->hashPlainTextEdit->setPlainText(d->password.hexKey());
    ui->statusBar->showMessage(tr("generation time: %1 ms").arg(d->password.elapsedSeconds(), 0, 'f', 4), 3000);
  }
  else {
    ui->statusBar->showMessage(tr("Password does not match regular expression. %1").arg(d->autoIncreaseIterations ? tr("Increasing iteration count.") : QString()));
    if (d->autoIncreaseIterations)
      ui->iterationsSpinBox->setValue(ui->iterationsSpinBox->value() + 1);
  }
}


void MainWindow::onPasswordGenerationAborted(void)
{
  onPasswordGenerated();
}


void MainWindow::copyPasswordToClipboard(void)
{
  QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Password copied to clipboard."));
}


void MainWindow::customCharacterSetCheckBoxToggled(bool checked)
{
  Q_D(MainWindow);
  bool reallyToggle = true;
  if (!checked && d->customCharacterSetDirty) {
    int button = QMessageBox::warning(
          this,
          tr("Really uncheck this option?"),
          tr("Custom characters have changed. If you uncheck the checkbox you will lose your changes. Do you really want to continue?"),
          QMessageBox::Yes,
          QMessageBox::No);
    if (button != QMessageBox::Yes) {
      reallyToggle = false;
      ui->useCustomCheckBox->setChecked(true);
    }
  }
  if (reallyToggle) {
    ui->useLowerCaseCheckBox->setEnabled(!checked);
    ui->useUpperCaseCheckBox->setEnabled(!checked);
    ui->useDigitsCheckBox->setEnabled(!checked);
    ui->useExtrasCheckBox->setEnabled(!checked);
    updateUsedCharacters();
    if (!checked)
      d->customCharacterSetDirty = false;
  }
}


void MainWindow::customCharacterSetChanged(void)
{
  Q_D(MainWindow);
  ui->useCustomCheckBox->setChecked(true);
  d->customCharacterSetDirty = true;
}


void MainWindow::updateValidator(void)
{
  Q_D(MainWindow);
  bool ok = false;
  QStringList canContain;
  QStringList mustContain;
  ui->forceLowerCaseCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceUpperCaseCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceDigitsCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceExtrasCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  if (ui->forceRegexCheckBox->isChecked()) {
    ok = d->password.setValidator(QRegExp(ui->forceRegexPlainTextEdit->toPlainText()));
  }
  else {
    if (ui->useLowerCaseCheckBox->isChecked())
      canContain << "a-z";
    if (ui->useUpperCaseCheckBox->isChecked())
      canContain << "A-Z";
    if (ui->useDigitsCheckBox->isChecked())
      canContain << "0-9";
    if (ui->useExtrasCheckBox->isChecked())
      canContain << PasswordParam::ExtraChars;
    if (ui->forceLowerCaseCheckBox->isChecked())
      mustContain << "[a-z]";
    if (ui->forceUpperCaseCheckBox->isChecked())
      mustContain << "[A-Z]";
    if (ui->forceDigitsCheckBox->isChecked())
      mustContain << "[0-9]";
    if (ui->forceExtrasCheckBox->isChecked())
      mustContain << "[" + PasswordParam::ExtraChars + "]";
    ok = d->password.setValidCharacters(canContain, mustContain);
    if (ok) {
      ui->forceRegexPlainTextEdit->blockSignals(true);
      ui->forceRegexPlainTextEdit->setPlainText(d->password.validator().pattern());
      ui->forceRegexPlainTextEdit->blockSignals(false);
    }
  }
  if (ok) {
    updatePassword();
    ui->statusBar->showMessage(QString());
  }
  else {
    ui->statusBar->showMessage(tr("Bad regex: ") + d->password.errorString());
  }
}


void MainWindow::copyDomainSettingsToGUI(const QString &domain)
{
  Q_D(MainWindow);
  const QVariantMap &p = d->domains[domain].toMap();
  ui->domainLineEdit->setText(p[DomainSettings::DOMAIN_NAME].toString());
  ui->userLineEdit->setText(p[DomainSettings::USER_NAME].toString());
  ui->useLowerCaseCheckBox->setChecked(p[DomainSettings::USE_LOWERCASE].toBool());
  ui->useUpperCaseCheckBox->setChecked(p[DomainSettings::USE_UPPERCASE].toBool());
  ui->useDigitsCheckBox->setChecked(p[DomainSettings::USE_DIGITS].toBool());
  ui->useExtrasCheckBox->setChecked(p[DomainSettings::USE_EXTRA].toBool());
  ui->useCustomCheckBox->setChecked(p[DomainSettings::USE_CUSTOM].toBool());
  ui->avoidAmbiguousCheckBox->setChecked(p[DomainSettings::AVOID_AMBIGUOUS].toBool());
  ui->customCharactersPlainTextEdit->blockSignals(true);
  ui->customCharactersPlainTextEdit->setPlainText(p[DomainSettings::CUSTOM_CHARACTER_SET].toString());
  ui->customCharactersPlainTextEdit->blockSignals(false);
  ui->iterationsSpinBox->setValue(p[DomainSettings::ITERATIONS].toInt());
  ui->passwordLengthSpinBox->setValue(p[DomainSettings::LENGTH].toInt());
  ui->forceLowerCaseCheckBox->setChecked(p[DomainSettings::FORCE_LOWERCASE].toBool());
  ui->forceUpperCaseCheckBox->setChecked(p[DomainSettings::FORCE_UPPERCASE].toBool());
  ui->forceDigitsCheckBox->setChecked(p[DomainSettings::FORCE_DIGITS].toBool());
  ui->forceExtrasCheckBox->setChecked(p[DomainSettings::FORCE_EXTRA].toBool());
  ui->forceRegexCheckBox->setChecked(p[DomainSettings::FORCE_REGEX_VALIDATION].toBool());
  d->createdDate = QDateTime::fromString(p[DomainSettings::CDATE].toString(), Qt::ISODate);
  d->modifiedDate = QDateTime::fromString(p[DomainSettings::MDATE].toString(), Qt::ISODate);
  updateValidator();
  updatePassword();
}


void MainWindow::saveCurrentSettings(void)
{
  Q_D(MainWindow);
  DomainSettings ds;
  ds.domainName = ui->domainLineEdit->text();
  ds.username = ui->userLineEdit->text();
  ds.useLowerCase = ui->useLowerCaseCheckBox->isChecked();
  ds.useUpperCase = ui->useUpperCaseCheckBox->isChecked();
  ds.useDigits = ui->useDigitsCheckBox->isChecked();
  ds.useExtra = ui->useExtrasCheckBox->isChecked();
  ds.useCustom = ui->useCustomCheckBox->isChecked();
  ds.avoidAmbiguous = ui->avoidAmbiguousCheckBox->isChecked();
  ds.customCharacterSet = ui->customCharactersPlainTextEdit->toPlainText();
  ds.iterations = ui->iterationsSpinBox->value();
  ds.salt = d->optionsDialog->salt();
  ds.length = ui->passwordLengthSpinBox->value();
  ds.validatorRegEx = d->password.validator();
  ds.forceLowerCase = ui->forceLowerCaseCheckBox->isChecked();
  ds.forceUpperCase = ui->forceUpperCaseCheckBox->isChecked();
  ds.forceDigits = ui->forceDigitsCheckBox->isChecked();
  ds.forceExtra = ui->forceExtrasCheckBox->isChecked();
  ds.forceRegexValidation = ui->forceRegexCheckBox->isChecked();
  ds.cDate = d->createdDate.isValid() ? d->createdDate : QDateTime::currentDateTime();
  ds.mDate = d->modifiedDate.isValid() ? d->modifiedDate : QDateTime::currentDateTime();
  saveDomainDataToSettings(ds);
  d->settings.sync();
  setDirty(false);
  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
}


void MainWindow::saveDomainDataToSettings(DomainSettings domainSettings)
{
  Q_D(MainWindow);
  QStringListModel *model = reinterpret_cast<QStringListModel*>(ui->domainLineEdit->completer()->model());
  QStringList domains = model->stringList();
  if (domains.contains(domainSettings.domainName, Qt::CaseInsensitive)) {
    domainSettings.mDate = QDateTime::currentDateTime();
  }
  else {
    domainSettings.cDate = QDateTime::currentDateTime();
    domains << domainSettings.domainName;
    model->setStringList(domains);
  }
  d->domains[domainSettings.domainName] = domainSettings.toVariantMap();
  saveDomainDataToSettings();
}


void MainWindow::saveDomainDataToSettings(void)
{
  Q_D(MainWindow);
  int errCode;
  QString errMsg;
  const QByteArray &cipher = encode(QJsonDocument::fromVariant(d->domains).toJson(QJsonDocument::Compact), COMPRESSION_ENABLED, &errCode, &errMsg);
  if (errCode == NO_CRYPT_ERROR) {
    d->settings.setValue("data/domains", QString(cipher.toHex()));
    d->settings.sync();
  }
  else {
    // TODO
    qWarning() << errMsg;
  }
}


void MainWindow::restoreDomainDataFromSettings(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::restoreDomainDataFromSettings()";
  Q_ASSERT(!d->masterPassword.isEmpty());
  QJsonDocument json;
  QStringList domains;
  const QByteArray &baDomains = QByteArray::fromHex(d->settings.value("data/domains").toByteArray());
  if (!baDomains.isEmpty()) {
    qDebug() << "MainWindow::restoreDomainDataFromSettings() trying to decode ...";
    int errCode;
    QString errMsg;
    const QByteArray &recovered = decode(baDomains, COMPRESSION_ENABLED, &errCode, &errMsg);
    if (errCode != NO_CRYPT_ERROR) {
      wrongPasswordWarning(errCode, errMsg);
      return;
    }
    json = QJsonDocument::fromJson(recovered);
    domains = json.object().keys();
    qDebug() << "Password accepted. Restored" << d->domains.count() << "domains";
    ui->statusBar->showMessage(tr("Password accepted. Restored %1 domains.").arg(d->domains.count()), 5000);
  }
  d->domains = json.toVariant().toMap();
  if (d->completer) {
    QObject::disconnect(d->completer, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
    delete d->completer;
  }
  d->completer = new QCompleter(domains);
  QObject::connect(d->completer, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
  ui->domainLineEdit->setCompleter(d->completer);
}


void MainWindow::saveSettings(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::saveSettings()";
  int errCode;
  QString errMsg;
  d->settings.setValue("mainwindow/geometry", geometry());
  d->settings.setValue("mainwindow/masterPasswordInvalidationTimerMins", d->optionsDialog->masterPasswordInvalidationTimeMins());
  d->settings.setValue("sync/onStart", d->optionsDialog->syncOnStart());
  d->settings.setValue("sync/filename", d->optionsDialog->syncFilename());
  d->settings.setValue("sync/useFile", d->optionsDialog->useSyncFile());
  d->settings.setValue("sync/useServer", d->optionsDialog->useSyncServer());
  d->settings.setValue("sync/serverRoot", d->optionsDialog->serverRootUrl());
  d->settings.setValue("sync/serverCertificateFilename", d->optionsDialog->serverCertificateFilename());
  d->settings.setValue("sync/acceptSelfSignedCertificates", d->optionsDialog->selfSignedCertificatesAccepted());
  d->settings.setValue("sync/acceptUntrustedCertificates", d->optionsDialog->untrustedCertificatesAccepted());
  d->settings.setValue("sync/serverUsername", QString(encode(d->optionsDialog->serverUsername().toUtf8(), false, &errCode, &errMsg).toHex()));
  d->settings.setValue("sync/serverPassword", QString(encode(d->optionsDialog->serverPassword().toUtf8(), false, &errCode, &errMsg).toHex()));
  d->settings.setValue("sync/serverWriteUrl", d->optionsDialog->writeUrl());
  d->settings.setValue("sync/serverReadUrl", d->optionsDialog->readUrl());
  saveDomainDataToSettings();
  d->settings.sync();
}


void MainWindow::loadCertificate(void)
{
  Q_D(MainWindow);
  d->sslConf.setCiphers(QSslSocket::supportedCiphers());
  d->sslConf.setCaCertificates(d->optionsDialog->serverCertificates());
  d->expectedSslErrors.clear();
  if (d->optionsDialog->selfSignedCertificatesAccepted())
    d->expectedSslErrors.append(QSslError::SelfSignedCertificate);
  if (d->optionsDialog->untrustedCertificatesAccepted())
    d->expectedSslErrors.append(QSslError::CertificateUntrusted);
}


void MainWindow::restoreSettings(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::restoreSettings()";
  int errCode;
  QString errMsg;
  restoreGeometry(d->settings.value("mainwindow/geometry").toByteArray());
  d->optionsDialog->setMasterPasswordInvalidationTimeMins(d->settings.value("mainwindow/masterPasswordInvalidationTimerMins", DEFAULT_MASTER_PASSWORD_INVALIDATION_TIME_MINS).toInt());
  QString defaultSyncFilename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + APP_NAME + ".bin";
  d->optionsDialog->setSyncFilename(d->settings.value("sync/filename", defaultSyncFilename).toString());
  d->optionsDialog->setSyncOnStart(d->settings.value("sync/onStart", true).toBool());
  d->optionsDialog->setUseSyncFile(d->settings.value("sync/useFile", false).toBool());
  d->optionsDialog->setUseSyncServer(d->settings.value("sync/useServer", false).toBool());
  d->optionsDialog->setServerRootUrl(d->settings.value("sync/serverRoot", DEFAULT_SERVER_ROOT).toString());
  d->optionsDialog->setSelfSignedCertificatesAccepted(d->settings.value("sync/acceptSelfSignedCertificates", false).toBool());
  d->optionsDialog->setUntrustedCertificatesAccepted(d->settings.value("sync/acceptUntrustedCertificates", false).toBool());
  d->optionsDialog->setWriteUrl(d->settings.value("sync/serverWriteUrl", DEFAULT_WRITE_URL).toString());
  d->optionsDialog->setReadUrl(d->settings.value("sync/serverReadUrl", DEFAULT_READ_URL).toString());
  d->optionsDialog->setServerCertificateFilename(d->settings.value("sync/serverCertificateFilename").toString());
  const QByteArray &serverUsername = d->settings.value("sync/serverUsername").toByteArray();
  if (!serverUsername.isEmpty()) {
    const QByteArray &serverUsernameBin = QByteArray::fromHex(serverUsername);
    QByteArray serverUsernameDecoded;
    try {
      serverUsernameDecoded = decode(serverUsernameBin, false, &errCode, &errMsg);
    }
    catch (const CryptoPP::Exception &e) {
      qErrnoWarning(e.GetErrorType(), e.what());
    }
    if (errCode == NO_CRYPT_ERROR) {
      d->optionsDialog->setServerUsername(QString::fromUtf8(serverUsernameDecoded));
    }
    else {
      qWarning() << "decode() error:" << errMsg;
    }
  }

  const QByteArray &serverPassword = d->settings.value("sync/serverPassword").toByteArray();
  if (!serverPassword.isEmpty()) {
    const QByteArray &serverPasswordBin = QByteArray::fromHex(serverPassword);
    const QByteArray &password = decode(serverPasswordBin, false, &errCode, &errMsg);
    if (errCode == NO_CRYPT_ERROR) {
      d->optionsDialog->setServerPassword(QString::fromUtf8(password));
    }
    else {
      qWarning() << "decode() error:" << errMsg;
    }
  }
}


QByteArray MainWindow::encode(const QByteArray &baPlain, bool compress, int *errCode, QString *errMsg)
{
  Q_D(MainWindow);
  if (errCode != nullptr)
    *errCode = NO_CRYPT_ERROR;
  std::string plain = (compress) ? qCompress(baPlain, 9).toStdString() : baPlain.toStdString();
  std::string cipher;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKeyWithIV(d->AESKey, AES_KEY_SIZE, IV);
    CryptoPP::StringSource s(
          plain,
          true,
          new CryptoPP::StreamTransformationFilter(
            enc,
            new CryptoPP::StringSink(cipher),
            CryptoPP::StreamTransformationFilter::PKCS_PADDING
            )
          );
    Q_UNUSED(s);
  }
  catch(const CryptoPP::Exception &e)
  {
    if (errCode != nullptr)
      *errCode = (int)e.GetErrorType();
    if (errMsg != nullptr)
      *errMsg = e.what();
    if (e.GetErrorType() > NO_CRYPT_ERROR)
      qErrnoWarning(e.GetErrorType(), e.what());
  }
  return QByteArray::fromStdString(cipher);
}


QByteArray MainWindow::decode(const QByteArray &baCipher, bool uncompress, int *errCode, QString *errMsg)
{
  Q_D(MainWindow);
  if (errCode != nullptr)
    *errCode = NO_CRYPT_ERROR;
  std::string recovered;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(d->AESKey, AES_KEY_SIZE, IV);
    CryptoPP::StringSource s(
          baCipher.toStdString(),
          true,
          new CryptoPP::StreamTransformationFilter(
            dec,
            new CryptoPP::StringSink(recovered)
            )
          );
    Q_UNUSED(s);
  }
  catch(const CryptoPP::Exception &e)
  {
    if (errCode != nullptr)
      *errCode = (int)e.GetErrorType();
    if (errMsg != nullptr)
      *errMsg = e.what();
    if (e.GetErrorType() > NO_CRYPT_ERROR)
      qErrnoWarning(e.GetErrorType(), e.what());
  }
  const QByteArray &plain = QByteArray::fromStdString(recovered);
  return (uncompress) ? qUncompress(plain) : plain;
}


void MainWindow::writeFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  if (reply->error() == QNetworkReply::NoError) {
    ++d->counter;
    d->progressDialog->setValue(d->counter);
    if (d->counter == d->maxCounter) {
      d->loaderIcon.stop();
      updateSaveButtonIcon();
      ui->statusBar->showMessage(tr("Sync to server finished."), 5000);
    }
  }
  else {
    QMessageBox::warning(this, tr("Sync server error"),
                         tr("Writing to the server failed. Reason: %1")
                         .arg(reply->errorString()), QMessageBox::Ok);
  }
}


void MainWindow::cancelServerOperation(void)
{
  Q_D(MainWindow);
  if (d->readReq->isRunning()) {
    d->readReq->abort();
    ui->statusBar->showMessage(tr("Server read operation aborted."), 3000);
  }
  if (d->writeReq->isRunning()) {
    d->writeReq->abort();
    ui->statusBar->showMessage(tr("Sync to server aborted."), 3000);
  }
}


void MainWindow::sync(void)
{
  Q_D(MainWindow);
  if (d->masterPassword.isEmpty()) {
    emit reenterCredentials();
    return;
  }

  if (d->optionsDialog->useSyncFile()) {
    qDebug() << "Trying to sync with file ...";
    ui->statusBar->showMessage(tr("Syncing with file ..."));
    QByteArray baDomains;
    QFileInfo fi(d->optionsDialog->syncFilename());
    if (!fi.isFile()) {
      QFile syncFile(d->optionsDialog->syncFilename());
      bool ok = syncFile.open(QIODevice::WriteOnly);
      if (!ok) {
        QMessageBox::warning(this, tr("Sync file creation error"),
                             tr("The sync file %1 cannot be created. Reason: %2")
                             .arg(d->optionsDialog->syncFilename())
                             .arg(syncFile.errorString()), QMessageBox::Ok);
      }
      const QByteArray &baDomains = encode(QByteArray("{}"), COMPRESSION_ENABLED);
      syncFile.write(baDomains);
      syncFile.close();
    }
    if (fi.isFile() && fi.isReadable()) {
      QFile syncFile(d->optionsDialog->syncFilename());
      bool ok = syncFile.open(QIODevice::ReadOnly);
      if (!ok) {
        QMessageBox::warning(this, tr("Sync file read error"),
                             tr("The sync file %1 cannot be opened for reading. Reason: %2")
                             .arg(d->optionsDialog->syncFilename()).arg(syncFile.errorString()), QMessageBox::Ok);
      }
      baDomains = syncFile.readAll();
      syncFile.close();
      sync(FileSource, baDomains);
    }
    else {
      QMessageBox::warning(this, tr("Sync file read error"),
                           tr("The sync file %1 cannot be opened for reading.")
                           .arg(d->optionsDialog->syncFilename()), QMessageBox::Ok);
    }
  }

  if (d->optionsDialog->useSyncServer()) {
    qDebug() << "Trying to sync with server ...";
    ui->statusBar->showMessage(tr("Syncing with server ..."));
    d->progressDialog->show();
    d->progressDialog->raise();
    d->progressDialog->setText(tr("Reading from server ..."));
    d->counter = 0;
    d->progressDialog->setValue(d->counter);
    QNetworkRequest req(QUrl(d->optionsDialog->serverRootUrl() + d->optionsDialog->readUrl()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::UserAgentHeader, APP_USER_AGENT);
    req.setRawHeader("Authorization", d->optionsDialog->serverCredentials());
    req.setSslConfiguration(d->sslConf);
    QNetworkReply *reply = d->readNAM->post(req, QByteArray());
    reply->ignoreSslErrors(d->expectedSslErrors);
  }
}


void MainWindow::sync(SyncSource syncSource, const QByteArray &remoteDomainsEncoded)
{
  Q_D(MainWindow);
  bool updateRemote = false;
  bool updateLocal = false;
  int errCode;
  QString errMsg;
  QJsonDocument remoteJSON;
  if (!remoteDomainsEncoded.isEmpty()) {
    std::string sDomains = decode(remoteDomainsEncoded, COMPRESSION_ENABLED, &errCode, &errMsg);
    if (errCode == NO_CRYPT_ERROR && !sDomains.empty()) {
      remoteJSON = QJsonDocument::fromJson(QByteArray::fromStdString(sDomains));
    }
    else {
      wrongPasswordWarning(errCode, errMsg);
      return;
    }
  }
  QVariantMap remoteDomains = remoteJSON.toVariant().toMap();
  const QSet<QString> &allDomainNames = (remoteDomains.keys() + d->domains.keys()).toSet();
  foreach(QString domainName, allDomainNames) {
    QVariantMap remoteDomain;
    QVariantMap localDomain;
    if (remoteDomains.contains(domainName))
      remoteDomain = remoteDomains[domainName].toMap();
    if (d->domains.contains(domainName))
      localDomain = d->domains[domainName].toMap();
    if (!localDomain.isEmpty() && !remoteDomain.isEmpty()) {
      const QDateTime &remoteT = QDateTime::fromString(remoteDomain[DomainSettings::MDATE].toString(), Qt::ISODate);
      const QDateTime &localT = QDateTime::fromString(localDomain[DomainSettings::MDATE].toString(), Qt::ISODate);
      if (remoteT > localT) {
        d->domains[domainName] = remoteDomain;
        updateLocal = true;
      }
      else if (remoteT < localT){
        remoteDomains[domainName] = localDomain;
        updateRemote = true;
      }
      else {
        // timestamps are equal, do nothing
      }
    }
    else if (remoteDomain.isEmpty()) {
      remoteDomains[domainName] = localDomain;
      updateRemote = true;
    }
    else {
      d->domains[domainName] = remoteDomain;
      updateLocal = true;
    }
  }

  remoteJSON = QJsonDocument::fromVariant(remoteDomains);
  if (updateRemote) {
    int errCode;
    QString errMsg;
    const QByteArray &baCipher = encode(remoteJSON.toJson(QJsonDocument::Compact), COMPRESSION_ENABLED, &errCode, &errMsg);
    if (errCode == NO_CRYPT_ERROR) {
      if (syncSource == FileSource && d->optionsDialog->useSyncFile()) {
        QFile syncFile(d->optionsDialog->syncFilename());
        syncFile.open(QIODevice::WriteOnly);
        qint64 bytesWritten = syncFile.write(baCipher);
        if (bytesWritten < 0) {
          QMessageBox::warning(this, tr("Sync file write error"), tr("Writing to your sync file %1 failed: %2")
                               .arg(d->optionsDialog->syncFilename())
                               .arg(syncFile.errorString()), QMessageBox::Ok);
        }
        syncFile.close();
      }
      if (syncSource == ServerSource && d->optionsDialog->useSyncServer()) {
        ui->statusBar->showMessage(tr("Sending data to server ..."));
        d->counter = 0;
        d->maxCounter = 1;
        d->progressDialog->setText(tr("Sending data to server ..."));
        d->progressDialog->setRange(0, d->maxCounter);
        d->progressDialog->setValue(0);
        d->progressDialog->show();
        QUrlQuery params;
        params.addQueryItem("data", baCipher.toBase64()); // TODO: baCipher.toBase64().toPercentEncoding()
        const QByteArray &data = params.query().toUtf8();
        QNetworkRequest req(QUrl(d->optionsDialog->serverRootUrl() + d->optionsDialog->writeUrl()));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
        req.setHeader(QNetworkRequest::UserAgentHeader, APP_USER_AGENT);
        req.setRawHeader("Authorization", d->optionsDialog->serverCredentials());
        req.setSslConfiguration(d->sslConf);
        QNetworkReply *reply = d->writeNAM->post(req, data);
        reply->ignoreSslErrors(d->expectedSslErrors);
        d->loaderIcon.start();
        updateSaveButtonIcon();
      }
    }
    else {
      // TODO: catch encryption error
      throw std::string("encryption error:" );
    }
  }

  if (updateLocal) {
    saveDomainDataToSettings();
    restoreDomainDataFromSettings();
  }
}


void MainWindow::domainSelected(const QString &domain)
{
  copyDomainSettingsToGUI(domain);
  setDirty(false);
}


void MainWindow::updateWindowTitle(void)
{
  Q_D(MainWindow);
  setWindowTitle(QString("%1 %2%3")
                 .arg(APP_NAME)
                 .arg(APP_VERSION)
                 .arg(d->parameterSetDirty ? "*" : ""));
}


void MainWindow::clearClipboard(void)
{
  QApplication::clipboard()->clear();
}


void MainWindow::enterMasterPassword(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::enterCredentials()";
  ui->encryptionLabel->setPixmap(QPixmap());
  setEnabled(false);
  d->masterPasswordDialog->setRepeatPassword(d->settings.value("mainwindow/masterPasswordEntered", false).toBool() == false);
  d->masterPasswordDialog->show();
  d->masterPasswordDialog->raise();
}


void MainWindow::masterPasswordEntered(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::masterPasswordEntered()";
  QString masterPwd = d->masterPasswordDialog->masterPassword();
  if (!masterPwd.isEmpty()) {
    setEnabled(true);
    d->masterPasswordDialog->hide();
    ui->encryptionLabel->setPixmap(QPixmap(":/images/encrypted.png"));
    d->masterPassword = masterPwd;
    d->cryptPassword.generate(PasswordParam(d->masterPassword.toUtf8()));
    d->cryptPassword.extractAESKey((char*)d->AESKey, AES_KEY_SIZE);
    restoreSettings();
    restoreDomainDataFromSettings();
    updatePassword();
    d->settings.setValue("mainwindow/masterPasswordEntered", true);
    d->settings.sync();
    if (d->optionsDialog->syncOnStart())
      sync();
  }
  else {
    d->settings.setValue("mainwindow/masterPasswordEntered", false);
    d->settings.sync();
    emit reenterCredentials();
  }
}


void MainWindow::wrongPasswordWarning(int errCode, QString errMsg)
{
  int button = QMessageBox::critical(
        this,
        tr("Decryption error"),
        tr("An error occured while decrypting your data (#%1, %2). Maybe you entered a wrong password. Please enter the correct password!").arg(errCode).arg(errMsg),
        QMessageBox::Retry,
        QMessageBox::NoButton);
  if (button == QMessageBox::Retry)
    emit reenterCredentials();
}


void MainWindow::invalidatePassword(bool reenter)
{
  Q_D(MainWindow);
  CryptoPP::memset_z(d->masterPassword.data(), 0, d->masterPassword.size());
  d->masterPassword = QByteArray();
  d->masterPasswordDialog->invalidatePassword();
  ui->statusBar->showMessage(tr("Master password cleared for security"));
  if (reenter)
    emit reenterCredentials();
}


void MainWindow::sslErrorsOccured(QNetworkReply *reply, QList<QSslError> errors)
{
  Q_UNUSED(reply);
  Q_UNUSED(errors);
  // ...
}


void MainWindow::updateSaveButtonIcon(int)
{
  Q_D(MainWindow);
  if (d->readReq != nullptr && d->readReq->isRunning()) {
    ui->savePushButton->setIcon(QIcon(d->loaderIcon.currentPixmap()));
  }
  else {
    ui->savePushButton->setIcon(QIcon());
  }
}


void MainWindow::readFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  d->loaderIcon.stop();
  updateSaveButtonIcon();
  d->progressDialog->hide();
  if (reply->error() == QNetworkReply::NoError) {
    const QByteArray &res = reply->readAll();
    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(res, &error);
    QVariantMap map = json.toVariant().toMap();
    if (map["status"].toString() == "ok") {
      const QByteArray &domainData = map["result"].toByteArray();
      sync(ServerSource, QByteArray::fromBase64(domainData));
    }
    else {
      QMessageBox::warning(this, tr("Sync server error"),
                           tr("Reading from the sync server failed. status: %1, error: %2")
                           .arg(map["status"].toString()).arg(map["error"].toString()), QMessageBox::Ok);
    }
  }
  else {
    QMessageBox::critical(this, "Critical Network Error", reply->errorString(), QMessageBox::Ok);
  }
}


void MainWindow::deleteFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  d->loaderIcon.stop();
  updateSaveButtonIcon();
  // TODO
}


void MainWindow::about(void)
{
  QMessageBox::about(
        this, tr("About %1 %2").arg(APP_NAME).arg(APP_VERSION),
        tr("<p><b>%1</b> is a domain specific password generator. "
           "See <a href=\"%2\" title=\"%1 project homepage\">%2</a> for more info.</p>"
           "<p>This program is free software: you can redistribute it and/or modify "
           "it under the terms of the GNU General Public License as published by "
           "the Free Software Foundation, either version 3 of the License, or "
           "(at your option) any later version.</p>"
           "<p>This program is distributed in the hope that it will be useful, "
           "but WITHOUT ANY WARRANTY; without even the implied warranty of "
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
           "GNU General Public License for more details.</p>"
           "You should have received a copy of the GNU General Public License "
           "along with this program. "
           "If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0\">http://www.gnu.org/licenses</a>.</p>"
           "<p>No animals were harmed during the development of <i>%1</i>. "
           "The software was programmed with CO<sub>2</sub> neutrality in mind and without the use of genetic engineering. "
           "It's vegan, free of antibiotics and hypoallergenic.</p>"
           "<p>Copyright &copy; 2015 %3 &lt;%4&gt;, Heise Medien GmbH &amp; Co. KG.</p>"
           )
        .arg(APP_NAME).arg(APP_URL).arg(APP_AUTHOR).arg(APP_AUTHOR_MAIL));
}


void MainWindow::aboutQt(void)
{
  QMessageBox::aboutQt(this);
}


