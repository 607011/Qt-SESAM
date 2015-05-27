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

#include "util.h"
#include "cryptopp562/aes.h"
#include "cryptopp562/ccm.h"
#include "cryptopp562/filters.h"

#ifdef QT_DEBUG
#include <QtDebug>
#include "testpbkdf2.h"
#endif

static const QString CompanyName = "c't";
static const QString AppName = "ctpwdgen";
static const QString AppVersion = "1.0 ALPHA";
static const QString AppUrl = "https://github.com/ola-ct/ctpwdgen";
static const QString AppAuthor = "Oliver Lau";
static const QString AppAuthorMail = "ola@ct.de";

static const int DefaultMasterPasswordInvalidationTimerIntervalMs = 5 * 60 * 1000;
static const int AESKeySize = 256 / 8;
static const unsigned char IV[16] = {0xb5, 0x4f, 0xcf, 0xb0, 0x88, 0x09, 0x55, 0xe5, 0xbf, 0x79, 0xaf, 0x37, 0x71, 0x1c, 0x28, 0xb6};


class MainWindowPrivate {
public:
  MainWindowPrivate(QWidget *parent = nullptr)
    : settings(QSettings::IniFormat, QSettings::UserScope, CompanyName, AppName)
    , loaderIcon(":/images/loader.gif")
    , trayIcon(QIcon(":/images/ctpwdgen.ico"), parent)
    , customCharacterSetDirty(false)
    , parameterSetDirty(false)
    , autoIncreaseIterations(true)
    , completer(nullptr)
    , credentialsDialog(new CredentialsDialog(parent))
    , optionsDialog(new OptionsDialog(parent))
    , masterPasswordValid(false)
  { /* ... */ }
  ~MainWindowPrivate()
  {
    safeDelete(completer);
  }
  CredentialsDialog *credentialsDialog;
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
  bool masterPasswordValid;
  QTimer masterPasswordInvalidationTimer;
  unsigned char AESKey[AESKeySize];
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
  QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
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
  QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
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
  QObject::connect(&d->password, SIGNAL(generationAborted()), SLOT(onPasswordGenerationAborted()));
  QObject::connect(&d->password, SIGNAL(generationStarted()), SLOT(onPasswordGenerationStarted()));
  QObject::connect(ui->actionNewDomain, SIGNAL(triggered(bool)), SLOT(newDomain()));
  QObject::connect(ui->actionSyncNow, SIGNAL(triggered(bool)), SLOT(sync()));
  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));
  QObject::connect(ui->actionReenterCredentials, SIGNAL(triggered(bool)), SLOT(enterCredentials()));
  QObject::connect(this, SIGNAL(reenterCredentials()), SLOT(enterCredentials()));
  QObject::connect(ui->actionOptions, SIGNAL(triggered(bool)), d->optionsDialog, SLOT(show()));
  QObject::connect(d->credentialsDialog, SIGNAL(accepted()), SLOT(credentialsEntered()));
  QObject::connect(&d->masterPasswordInvalidationTimer, SIGNAL(timeout()), SLOT(invalidatePassword()));

#ifdef QT_DEBUG
  QObject::connect(ui->actionInvalidatePassword, SIGNAL(triggered(bool)), SLOT(invalidatePassword()));
  QObject::connect(ui->actionSaveSettings, SIGNAL(triggered(bool)), SLOT(saveSettings()));
#endif

#ifdef QT_DEBUG
  ui->saltLineEdit->setEnabled(true);
  ui->domainLineEdit->setText("foo bar");
  ui->avoidAmbiguousCheckBox->setChecked(true);
#endif
  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&d->loaderIcon);
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  d->masterPasswordInvalidationTimer.setSingleShot(true);
  d->masterPasswordInvalidationTimer.setTimerType(Qt::VeryCoarseTimer);
  restoreSettings();
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
  QMenu *trayMenu = new QMenu(AppName);
  QAction *actionSync = trayMenu->addAction(tr("Sync"));
  QObject::connect(actionSync, SIGNAL(triggered(bool)), SLOT(sync()));
  QAction *actionClearClipboard = trayMenu->addAction(tr("Clear clipboard"));
  QObject::connect(actionClearClipboard, SIGNAL(triggered(bool)), SLOT(clearClipboard()));
  QAction *actionAbout = trayMenu->addAction(tr("About %1").arg(AppName));
  QObject::connect(actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QAction *actionQuit = trayMenu->addAction(tr("Quit"));
  QObject::connect(actionQuit, SIGNAL(triggered(bool)), SLOT(close()));
  d->trayIcon.setContextMenu(trayMenu);
}


void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::DoubleClick) {
    if (isVisible()) {
      hide();
    }
    else {
      show();
      raise();
      setFocus();
    }
  }
}


MainWindow::~MainWindow()
{
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
    QMainWindow::closeEvent(e);
    e->accept();
    saveSettings();
    invalidatePassword();
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
  ui->saltLineEdit->setText(domainSettings.salt);
  ui->forceRegexPlainTextEdit->setPlainText(domainSettings.validatorRegEx.pattern());
  ui->forceRegexCheckBox->setChecked(domainSettings.forceValidation);
  updateValidator();
  updatePassword();

}

void MainWindow::setDirty(bool dirty)
{
  Q_D(MainWindow);
  d->parameterSetDirty = dirty;
  updateWindowTitle();
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
    d->masterPasswordInvalidationTimer.start(DefaultMasterPasswordInvalidationTimerIntervalMs);
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
          ui->saltLineEdit->text().toUtf8(),
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


void MainWindow::saveCurrentSettings(void)
{
  Q_D(MainWindow);
  DomainSettings domainSettings;
  domainSettings.domain = ui->domainLineEdit->text();
  domainSettings.username = ui->userLineEdit->text();
  domainSettings.useLowerCase = ui->useLowerCaseCheckBox->isChecked();
  domainSettings.useUpperCase = ui->useUpperCaseCheckBox->isChecked();
  domainSettings.useDigits = ui->useDigitsCheckBox->isChecked();
  domainSettings.useExtra = ui->useExtrasCheckBox->isChecked();
  domainSettings.useCustom = ui->useCustomCheckBox->isChecked();
  domainSettings.avoidAmbiguous = ui->avoidAmbiguousCheckBox->isChecked();
  domainSettings.customCharacters = ui->customCharactersPlainTextEdit->toPlainText();
  domainSettings.iterations = ui->iterationsSpinBox->value();
  domainSettings.salt = ui->saltLineEdit->text();
  domainSettings.length = ui->passwordLengthSpinBox->value();
  domainSettings.validatorRegEx = d->password.validator();
  domainSettings.forceValidation = ui->forceRegexCheckBox->isChecked();
  domainSettings.cDate = d->createdDate.isValid() ? d->createdDate : QDateTime::currentDateTime();
  domainSettings.mDate = d->modifiedDate.isValid() ? d->modifiedDate : QDateTime::currentDateTime();
  saveDomainDataToSettings(domainSettings);
  d->settings.sync();
  setDirty(false);
  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
}


void MainWindow::saveDomainDataToSettings(DomainSettings domainSettings)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::saveDomainSettings() for domain" << domainSettings.domain;
  QStringListModel *model = reinterpret_cast<QStringListModel*>(ui->domainLineEdit->completer()->model());
  QStringList domains = model->stringList();
  if (domains.contains(domainSettings.domain, Qt::CaseInsensitive)) {
    domainSettings.mDate = QDateTime::currentDateTime();
  }
  else {
    domainSettings.cDate = QDateTime::currentDateTime();
    domains << domainSettings.domain;
    model->setStringList(domains);
  }
  d->domains[domainSettings.domain] = domainSettings.toVariant();
  sync();
}


void MainWindow::saveDomainDataToSettings(void)
{
  Q_D(MainWindow);
#ifdef QT_DEBUG
  std::string plain = QJsonDocument::fromVariant(d->domains).toJson(QJsonDocument::Compact).toStdString();
#endif
  QByteArray cipher = encode(QJsonDocument::fromVariant(d->domains).toJson(QJsonDocument::Compact));

#ifdef QT_DEBUG
  std::string recovered;
  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
  dec.SetKeyWithIV(d->AESKey, AESKeySize, IV);
  CryptoPP::StringSource s(
        cipher.toStdString(),
        true,
        new CryptoPP::StreamTransformationFilter(
          dec,
          new CryptoPP::StringSink(recovered)
          )
        );
  Q_UNUSED(s);
  Q_ASSERT(recovered == plain);
#endif

  d->settings.setValue("data/domains", cipher);
  sync();
}


void MainWindow::saveSettings(void)
{
  Q_D(MainWindow);
  d->settings.setValue("mainwindow/geometry", geometry());
  d->settings.setValue("sync/onStart", ui->actionSyncOnStart->isChecked());
  d->settings.setValue("sync/filename", d->optionsDialog->syncFilename());
  saveDomainDataToSettings();
  d->settings.sync();
}


QByteArray MainWindow::encode(const QByteArray &baPlain, int *errCode, QString *errMsg)
{
  Q_D(MainWindow);
  std::string cipher;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKeyWithIV(d->AESKey, AESKeySize, IV);
    CryptoPP::StringSource s(
          baPlain.toStdString(),
          true,
          new CryptoPP::StreamTransformationFilter(
            enc,
            new CryptoPP::StringSink(cipher)
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
  }

  return QByteArray::fromStdString(cipher);
}


QByteArray MainWindow::decode(const QByteArray &baCipher, int *errCode, QString *errMsg)
{
  Q_D(MainWindow);
  Q_ASSERT(!baCipher.isEmpty());
  std::string recovered;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(d->AESKey, AESKeySize, IV);
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
  }
  return QByteArray::fromStdString(recovered);
}


void MainWindow::restoreDomainDataFromSettings(void)
{
  Q_D(MainWindow);
  Q_ASSERT(!d->masterPassword.isEmpty());
  qDebug() << "MainWindow::restoreDomainDataFromSettings()";
  QJsonDocument json;
  QStringList domains;
  const QByteArray &baDomains = d->settings.value("data/domains").toByteArray();
  if (!baDomains.isEmpty()) {
    int errCode = -1;
    QString errMsg;
    QByteArray recovered = decode(baDomains, &errCode, &errMsg);
    d->masterPasswordValid = (errCode == -1);
    if (!d->masterPasswordValid) {
      wrongPasswordWarning(errCode, errMsg);
      return;
    }
    json = QJsonDocument::fromJson(recovered);
    domains = json.object().keys();
  }
  d->domains = json.toVariant().toMap();
  ui->statusBar->showMessage(tr("Restored %1 domains.").arg(d->domains.count()), 5000);
  if (d->completer) {
    QObject::disconnect(d->completer, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
    delete d->completer;
  }
  d->completer = new QCompleter(domains);
  QObject::connect(d->completer, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
  ui->domainLineEdit->setCompleter(d->completer);
}


void MainWindow::restoreSettings(void)
{
  Q_D(MainWindow);
  restoreGeometry(d->settings.value("mainwindow/geometry").toByteArray());
  QString defaultSyncFilename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + AppName + ".bin";
  d->optionsDialog->setSyncFilename(d->settings.value("sync/filename", defaultSyncFilename).toString());
  ui->actionSyncOnStart->setChecked(d->settings.value("sync/onStart", true).toBool());
}


void MainWindow::copyDomainSettingsToGUI(const QString &domain)
{
  Q_D(MainWindow);
  const QVariantMap &p = d->domains[domain].toMap();
  ui->domainLineEdit->setText(p["domain"].toString());
  ui->userLineEdit->setText(p["username"].toString());
  ui->useLowerCaseCheckBox->setChecked(p["useLowerCase"].toBool());
  ui->useUpperCaseCheckBox->setChecked(p["useUpperCase"].toBool());
  ui->useDigitsCheckBox->setChecked(p["useDigits"].toBool());
  ui->useExtrasCheckBox->setChecked(p["useExtra"].toBool());
  ui->useCustomCheckBox->setChecked(p["useCustom"].toBool());
  ui->avoidAmbiguousCheckBox->setChecked(p["avoidAmbiguous"].toBool());
  ui->customCharactersPlainTextEdit->blockSignals(true);
  ui->customCharactersPlainTextEdit->setPlainText(p["customCharacters"].toString());
  ui->customCharactersPlainTextEdit->blockSignals(false);
  ui->iterationsSpinBox->setValue(p["iterations"].toInt());
  ui->passwordLengthSpinBox->setValue(p["length"].toInt());
  ui->saltLineEdit->setText(p["salt"].toString());
  d->createdDate = QDateTime::fromString(p["cDate"].toString(), Qt::ISODate);
  d->modifiedDate = QDateTime::fromString(p["mDate"].toString(), Qt::ISODate);
  updateValidator();
  updatePassword();
}


void MainWindow::sync(void)
{
  Q_D(MainWindow);
  qDebug() << "MainWindow::sync()";
  if (d->masterPassword.isEmpty() || !d->masterPasswordValid) {
    emit reenterCredentials();
  }
  else {
    // TODO: handle bad credentials
    bool rewriteSyncFile = false;
    QByteArray baDomains;
    QFileInfo fi(d->optionsDialog->syncFilename());
    qDebug() << "mOptionsDialog->syncFilename() =" << d->optionsDialog->syncFilename();
    if (!fi.isFile()) {
      qDebug() << "Sync file missing. Creating ...";
      QFile syncFile(d->optionsDialog->syncFilename());
      bool ok = syncFile.open(QIODevice::WriteOnly);
      if (!ok) {
        // TODO: handle error
      }
      const QByteArray &baDomains = encode(QByteArray("{}"));
      syncFile.write(baDomains);
      syncFile.close();
    }
    if (fi.isFile() && fi.isReadable()) {
      QFile syncFile(d->optionsDialog->syncFilename());
      bool ok = syncFile.open(QIODevice::ReadOnly);
      if (!ok) {
        // TODO: handle error
      }
      baDomains = syncFile.readAll();
      syncFile.close();
    }
    else {
      // TODO: handle sync file not readable error
    }
    QJsonDocument remoteJSON;
    int errCode = -1;
    QString errMsg;
    if (!baDomains.isEmpty()) {
      std::string sDomains = decode(baDomains, &errCode, &errMsg);
      if (errCode == -1 && !sDomains.empty()) {
        remoteJSON = QJsonDocument::fromJson(QByteArray::fromStdString(sDomains));
      }
      else {
        d->masterPasswordValid = false;
        wrongPasswordWarning(errCode, errMsg);
        return;
      }
    }
    QVariantMap remoteDomains = remoteJSON.toVariant().toMap();
    qDebug() << "remoteDomains.keys() =" << remoteDomains.keys();
    qDebug() << "mDomains.keys() =" << d->domains.keys();
    const QSet<QString> &allDomainNames = (remoteDomains.keys() + d->domains.keys()).toSet();
    foreach(QString domainName, allDomainNames) {
      qDebug() << "Checking domain" << domainName;
      const QVariantMap &remoteDomain = remoteDomains.contains(domainName) ? remoteDomains[domainName].toMap() : QVariantMap();
      const QVariantMap &localDomain = d->domains.contains(domainName) ? d->domains[domainName].toMap() : QVariantMap();
      if (!localDomain.isEmpty() && !remoteDomain.isEmpty()) {
        if (remoteDomain["mDate"].toDateTime() > localDomain["mDate"].toDateTime()) {
          d->domains[domainName] = remoteDomain;
          qDebug() << "Updating local:" << domainName;
        }
        else {
          remoteDomains[domainName] = localDomain;
          rewriteSyncFile = true;
          qDebug() << "Updating remote:" << domainName;
        }
      }
      else if (remoteDomain.isEmpty()) {
        remoteDomains[domainName] = localDomain;
        qDebug() << "Adding to remote:" << domainName;
        rewriteSyncFile = true;
      }
      else {
        d->domains[domainName] = remoteDomain;
        qDebug() << "Adding to local:" << domainName;
      }
    }
    remoteJSON = QJsonDocument::fromVariant(remoteDomains);
    qDebug() << "REMOTE:" << remoteJSON.toJson(QJsonDocument::Compact);
    QJsonDocument localJSON = QJsonDocument::fromVariant(d->domains);
    qDebug() << "LOCAL:" << localJSON.toJson(QJsonDocument::Compact);
    if (rewriteSyncFile) {
      qDebug() << "rewriting sync file ..." << remoteJSON.toJson();
      const QByteArray &baCipher = encode(remoteJSON.toJson(QJsonDocument::Compact));
      QFile syncFile(d->optionsDialog->syncFilename());
      syncFile.open(QIODevice::WriteOnly);
      qint64 bytesWritten = syncFile.write(baCipher);
      // TODO: handle bytesWritten < 0
      qDebug() << "bytesWritten: " << bytesWritten;
      syncFile.close();
    }
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
  setWindowTitle(AppName + " " + AppVersion + (d->parameterSetDirty ? "*" : ""));
}


void MainWindow::clearClipboard(void)
{
  QApplication::clipboard()->clear();
}


void MainWindow::enterCredentials(void)
{
  Q_D(MainWindow);
  d->masterPasswordValid = false;
  d->credentialsDialog->show();
}


void MainWindow::credentialsEntered(void)
{
  Q_D(MainWindow);
  const QString &credentials = d->credentialsDialog->password();
  if (!credentials.isEmpty()) {
    d->masterPassword = credentials;
    d->cryptPassword.generate(PasswordParam(d->masterPassword.toLocal8Bit()));
#ifdef WIN32
    memcpy_s(d->AESKey, AESKeySize, d->cryptPassword.derivedKey().data(), AESKeySize);
#else
    memcpy(d->mAESKey, d->mCryptPassword.derivedKey().data(), AESKeySize);
#endif
    restoreDomainDataFromSettings();
    updatePassword();
  }
  else {
    emit reenterCredentials();
  }
}

template <class T>
void MainWindow::zeroize(T *pC, int len)
{
  Q_ASSERT(pC != nullptr);
  Q_ASSERT(len > 0);
  while (len--)
    *pC++ = T('\0');
}


void MainWindow::zeroize(QLineEdit *lineEdit)
{
  Q_ASSERT(lineEdit != nullptr);
  if (lineEdit->text().isEmpty())
    return;
  zeroize(lineEdit->text().data(), lineEdit->text().size());
}


void MainWindow::wrongPasswordWarning(int errCode, QString errMsg)
{
  int button = QMessageBox::critical(
        this,
        tr("Decryption error"),
        tr("An error occured while decrypting your data (#%1, %2). Maybe you entered a wrong password. Retry entering the correct password!").arg(errCode).arg(errMsg),
        QMessageBox::Retry,
        QMessageBox::NoButton);
  if (button == QMessageBox::Retry)
    emit reenterCredentials();
}


void MainWindow::invalidatePassword(QLineEdit *lineEdit) {
  zeroize(lineEdit);
  lineEdit->clear();
}


void MainWindow::invalidatePassword(void)
{
  Q_D(MainWindow);
  zeroize(d->masterPassword.data(), d->masterPassword.size());
  d->masterPassword = QByteArray();
  ui->statusBar->showMessage(tr("Password cleared for security"), 5000);
}


void MainWindow::about(void)
{
  QMessageBox::about(
        this, tr("About %1 %2").arg(AppName).arg(AppVersion),
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
        .arg(AppName).arg(AppUrl).arg(AppAuthor).arg(AppAuthorMail));
}


void MainWindow::aboutQt(void)
{
  QMessageBox::aboutQt(this);
}


