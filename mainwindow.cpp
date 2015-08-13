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
#include <QElapsedTimer>

#include <random>

#include "global.h"
#include "util.h"
#include "progressdialog.h"
#include "newdomainwizard.h"
#include "credentialsdialog.h"
#include "optionsdialog.h"

#include "cryptopp562/sha.h"
#include "cryptopp562/aes.h"
#include "cryptopp562/ccm.h"
#include "cryptopp562/filters.h"
#include "cryptopp562/misc.h"

#include <QtDebug>

#ifdef QT_DEBUG
#include "testpbkdf2.h"
#endif

#include "dump.h"



static const int DEFAULT_MASTER_PASSWORD_INVALIDATION_TIME_MINS = 5;
static const int DEFAULT_SALT_LENGTH = 12;
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
    , autoIncrementIterations(true)
    , updatePasswordBlocked(false)
    , newDomainWizard(new NewDomainWizard)
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
  { /* ... */ }
  ~MainWindowPrivate()
  {
    SecureErase(masterPassword);
    SecureErase(AESKey, AES_KEY_SIZE);
  }
  NewDomainWizard *newDomainWizard;
  CredentialsDialog *masterPasswordDialog;
  OptionsDialog *optionsDialog;
  QSettings settings;
  DomainSettingsList domains;
  QMovie loaderIcon;
  bool customCharacterSetDirty;
  bool parameterSetDirty;
  bool autoIncrementIterations;
  bool updatePasswordBlocked;
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
  std::random_device randomDevice;
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
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->userLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->userLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->legacyPasswordLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->notesPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(updatePassword()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->saltBase64LineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->copyLegacyPasswordToClipboardPushButton, SIGNAL(clicked()), SLOT(copyLegacyPasswordToClipboard()));
  QObject::connect(ui->copyGeneratedPasswordToClipboardPushButton, SIGNAL(clicked()), SLOT(copyGeneratedPasswordToClipboard()));
  QObject::connect(ui->renewSaltPushButton, SIGNAL(clicked()), SLOT(onRenewSalt()));
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
  QObject::connect(ui->actionOptions, SIGNAL(triggered(bool)), d->optionsDialog, SLOT(show()));
  QObject::connect(d->optionsDialog, SIGNAL(accepted()), SLOT(saveSettings()));
  QObject::connect(d->optionsDialog, SIGNAL(certificatesUpdated()), SLOT(loadCertificate()));
  QObject::connect(d->masterPasswordDialog, SIGNAL(accepted()), SLOT(masterPasswordEntered()));
  QObject::connect(&d->masterPasswordInvalidationTimer, SIGNAL(timeout()), SLOT(invalidatePassword()));
  QObject::connect(ui->domainsComboBox, SIGNAL(activated(QString)), SLOT(domainSelected(QString)));

  d->progressDialog = new ProgressDialog(this);
  QObject::connect(d->progressDialog, SIGNAL(cancelled()), SLOT(cancelServerOperation()));

  QObject::connect(&d->loaderIcon, SIGNAL(frameChanged(int)), SLOT(updateSaveButtonIcon(int)));
  QObject::connect(d->readNAM, SIGNAL(finished(QNetworkReply*)), SLOT(readFinished(QNetworkReply*)));
  QObject::connect(d->readNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));
  QObject::connect(d->writeNAM, SIGNAL(finished(QNetworkReply*)), SLOT(writeFinished(QNetworkReply*)));
  QObject::connect(d->writeNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));

  QObject::connect(this, SIGNAL(badMasterPassword()), SLOT(enterMasterPassword()), Qt::QueuedConnection);

  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&d->loaderIcon);
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  d->masterPasswordInvalidationTimer.setSingleShot(true);
  d->masterPasswordInvalidationTimer.setTimerType(Qt::VeryCoarseTimer);

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

#ifndef QT_DEBUG
  ui->hashPlainTextEdit->setVisible(false);
#endif

#if defined(QT_DEBUG)
#if defined(WIN32)
  ui->menuExtras->addAction(tr("[DEBUG] Create Mini Dump"), this, SLOT(createFullDump()), QKeySequence(tr("Alt+Shift+D")));
#endif
  ui->menuExtras->addAction(tr("[DEBUG] Invalidate password"), this, SLOT(invalidatePassword()), QKeySequence(tr("Alt+Shift+I")));
#endif

#ifdef QT_DEBUG
  TestPBKDF2 tc;
  QTest::qExec(&tc, 0, 0);
#endif

  enterMasterPassword();
}


void MainWindow::showHide(void)
{
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
  if (reason == QSystemTrayIcon::DoubleClick)
    showHide();
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


void MainWindow::blockUpdatePassword(void)
{
  Q_D(MainWindow);
  d->updatePasswordBlocked = true;
}


void MainWindow::unblockUpdatePassword(void)
{
  Q_D(MainWindow);
  d->updatePasswordBlocked = false;
}


void MainWindow::newDomain(void)
{
  Q_D(MainWindow);
  int rc = d->newDomainWizard->exec();
  if (rc == QDialog::Accepted) {
    setDirty(false);
    ui->domainLineEdit->setText(d->newDomainWizard->domain());
    ui->userLineEdit->setText(d->newDomainWizard->username());
    ui->legacyPasswordLineEdit->setText(d->newDomainWizard->legacyPassword());
    ui->saltBase64LineEdit->setText(d->newDomainWizard->salt_base64());
    ui->iterationsSpinBox->setValue(d->newDomainWizard->iterations());
    ui->passwordLengthSpinBox->setValue(d->newDomainWizard->passwordLength());
    ui->notesPlainTextEdit->setPlainText(d->newDomainWizard->notes());
    ui->usedCharactersPlainTextEdit->setPlainText(d->newDomainWizard->usedCharacters());
    ui->createdLabel->setText(QDateTime::currentDateTime().toString(Qt::ISODate));
    ui->modifiedLabel->setText(QString());
    d->autoIncrementIterations = true;
    updatePassword();
  }
}


void MainWindow::renewSalt(void)
{
  Q_D(MainWindow);
  QByteArray salt(d->optionsDialog->saltLength(), '\0');
  for (int i = 0; i < salt.size(); ++i)
    salt[i] = (char)d->randomDevice();
  ui->saltBase64LineEdit->setText(salt.toBase64());
  updatePassword();
}


void MainWindow::onRenewSalt(void)
{
  int button = QMessageBox::question(
        this,
        tr("Really renew salt?"),
        tr("Renewing the salt will invalidate your current generated password. Are you sure you want to generate a new salt?"),
        QMessageBox::Yes,
        QMessageBox::No);
  if (button == QMessageBox::Yes)
    renewSalt();
}


void MainWindow::setDirty(bool dirty)
{
  Q_D(MainWindow);
  d->parameterSetDirty = dirty && !ui->domainLineEdit->text().isEmpty();
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
  d->autoIncrementIterations = false;
  ui->copyGeneratedPasswordToClipboardPushButton->hide();
  ui->processLabel->show();
  ui->cancelPushButton->show();
  d->loaderIcon.start();
}


void MainWindow::updatePassword(void)
{
  Q_D(MainWindow);
  if (!d->updatePasswordBlocked && !d->masterPassword.isEmpty() && !ui->domainLineEdit->text().isEmpty()) {
    ui->generatedPasswordLineEdit->setText(QString());
    ui->hashPlainTextEdit->setPlainText(QString());
    ui->statusBar->showMessage(QString());
    stopPasswordGeneration();
    generatePassword();
    restartInvalidationTimer();
  }
}


DomainSettings MainWindow::collectedDomainSettings(void) const
{
  DomainSettings ds;
  ds.domainName = ui->domainLineEdit->text();
  ds.userName = ui->userLineEdit->text();
  ds.notes = ui->notesPlainTextEdit->toPlainText();
  ds.salt_base64 = ui->saltBase64LineEdit->text();
  ds.legacyPassword = ui->legacyPasswordLineEdit->text();
  ds.iterations = ui->iterationsSpinBox->value();
  ds.length = ui->passwordLengthSpinBox->value();
  ds.usedCharacters = ui->usedCharactersPlainTextEdit->toPlainText();
  ds.createdDate = d_ptr->createdDate.isValid() ? d_ptr->createdDate : QDateTime::currentDateTime();
  ds.modifiedDate = d_ptr->modifiedDate.isValid() ? d_ptr->modifiedDate : QDateTime::currentDateTime();
  ds.deleted = ui->deleteCheckBox->isChecked();
  ds.canBeDeletedByRemote = ds.deleted;
  return ds;
}


void MainWindow::generatePassword(void)
{
  Q_D(MainWindow);
  const DomainSettings ds = collectedDomainSettings();
  d->password.setDomainSettings(ds);
  d->password.generateAsync(d->masterPassword);
}


void MainWindow::stopPasswordGeneration(void)
{
  Q_D(MainWindow);
  if (d->password.isRunning()) {
    d->password.abortGeneration();
    d->password.waitForFinished();
  }
}


bool MainWindow::keyContainsAnyOf(const QString &forcedCharacters)
{
  foreach (QChar c, forcedCharacters) {
    if (d_ptr->password.key().contains(c))
      return true;
  }
  return false;
}



void MainWindow::analyzeGeneratedPassword(void)
{
  Q_D(MainWindow);
  if (keyContainsAnyOf(Password::LowerChars))
    d->newDomainWizard->setForceLowercase(true);
  if (keyContainsAnyOf(Password::UpperChars))
    d->newDomainWizard->setForceUppercase(true);
  if (keyContainsAnyOf(Password::Digits))
    d->newDomainWizard->setForceDigits(true);
  if (keyContainsAnyOf(Password::ExtraChars))
    d->newDomainWizard->setForceExtra(true);
}


bool MainWindow::generatedPasswordIsValid(void)
{
  Q_D(MainWindow);
  if (d_ptr->newDomainWizard->forceLowercase() && !keyContainsAnyOf(Password::LowerChars))
    return false;
  if (d_ptr->newDomainWizard->forceUppercase() && !keyContainsAnyOf(Password::UpperChars))
    return false;
  if (d_ptr->newDomainWizard->forceDigits() && !keyContainsAnyOf(Password::Digits))
    return false;
  if (d_ptr->newDomainWizard->forceExtra() && !keyContainsAnyOf(Password::ExtraChars))
    return false;
  return true;
}

void MainWindow::onPasswordGenerated(void)
{
  Q_D(MainWindow);
  if (!d->autoIncrementIterations || generatedPasswordIsValid()) {
    ui->generatedPasswordLineEdit->setText(d->password.key());
    ui->hashPlainTextEdit->setPlainText(d->password.hexKey());
    if (!d->password.isAborted())
      ui->statusBar->showMessage(tr("generation time: %1 ms").arg(d->password.elapsedSeconds(), 0, 'f', 4), 3000);
    ui->processLabel->hide();
    ui->cancelPushButton->hide();
    d->loaderIcon.stop();
    ui->copyGeneratedPasswordToClipboardPushButton->show();
  }
  else if (d->autoIncrementIterations) {
    const int nIterations = ui->iterationsSpinBox->value() + 1;
    ui->statusBar->showMessage(tr("Password does not follow the required rules. Increasing iteration count."));
    ui->iterationsSpinBox->blockSignals(true);
    ui->iterationsSpinBox->setValue(nIterations);
    ui->iterationsSpinBox->blockSignals(false);
    updatePassword();
  }
}


void MainWindow::onPasswordGenerationAborted(void)
{
  onPasswordGenerated();
}


void MainWindow::copyLegacyPasswordToClipboard(void)
{
  QApplication::clipboard()->setText(ui->legacyPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Legacy password copied to clipboard."));
}


void MainWindow::copyGeneratedPasswordToClipboard(void)
{
  QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Generated password copied to clipboard."), 3000);
}


void MainWindow::copyDomainSettingsToGUI(const QString &domain)
{
  Q_D(MainWindow);
  blockUpdatePassword();
  const DomainSettings &p = d->domains.at(domain);
  ui->domainLineEdit->setText(p.domainName);
  ui->userLineEdit->setText(p.userName);
  ui->legacyPasswordLineEdit->setText(p.legacyPassword);
  ui->saltBase64LineEdit->setText(p.salt_base64);
  ui->notesPlainTextEdit->setPlainText(p.notes);
  ui->usedCharactersPlainTextEdit->setPlainText(p.usedCharacters);
  ui->iterationsSpinBox->setValue(p.iterations);
  ui->passwordLengthSpinBox->setValue(p.length);
  ui->createdLabel->setText(p.createdDate.toString(Qt::ISODate));
  ui->modifiedLabel->setText(p.modifiedDate.toString(Qt::ISODate));
  d->createdDate = p.createdDate;
  d->modifiedDate = p.modifiedDate;
  ui->deleteCheckBox->setChecked(false);
  unblockUpdatePassword();
  updatePassword();
}


void MainWindow::setDomainComboBox(QStringList domainList)
{
  domainList.sort();
  ui->domainsComboBox->clear();
  ui->domainsComboBox->addItem(tr("<Choose domain ...>"));
  ui->domainsComboBox->addItems(domainList);
}


void MainWindow::saveCurrentSettings(void)
{
  Q_D(MainWindow);

  qDebug() << "MainWindow::saveCurrentSetting()";
  DomainSettings ds = collectedDomainSettings();

  ui->createdLabel->setText(ds.createdDate.toString(Qt::ISODate));
  ui->modifiedLabel->setText(ds.modifiedDate.toString(Qt::ISODate));

  QStringList domainList;
  for (int i = 1; i < ui->domainsComboBox->count(); ++i)
    domainList.append(ui->domainsComboBox->itemText(i));

  if (domainList.contains(ds.domainName, Qt::CaseInsensitive)) {
    ds.modifiedDate = QDateTime::currentDateTime();
    if (ds.deleted)
      domainList.removeOne(ds.domainName);
  }
  else {
    ds.createdDate = QDateTime::currentDateTime();
    ds.modifiedDate = ds.createdDate;
    if (!ds.deleted)
      domainList << ds.domainName;
  }

  setDomainComboBox(domainList);

  d->domains.updateWith(ds);
  saveAllDomainDataToSettings();

  setDirty(false);

  if (ds.deleted)
    newDomain();

  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
}


void MainWindow::saveAllDomainDataToSettings(void)
{
  Q_D(MainWindow);
  int errCode;
  QString errMsg;
  const QByteArray &cipher = encode(d->domains.toJson(), COMPRESSION_ENABLED, &errCode, &errMsg);
  if (errCode == NO_CRYPT_ERROR) {
    d->settings.setValue("data/domains", QString(cipher.toHex()));
    d->settings.sync();
  }
  else {
    // TODO
    qWarning() << errMsg;
  }
}


bool MainWindow::restoreDomainDataFromSettings(void)
{
  Q_D(MainWindow);
  Q_ASSERT_X(!d->masterPassword.isEmpty(), "MainWindow::restoreDomainDataFromSettings()", "d->masterPassword must not be empty");
  QJsonDocument json;
  QStringList domainList;
  const QByteArray &baDomains = QByteArray::fromHex(d->settings.value("data/domains").toByteArray());
  if (!baDomains.isEmpty()) {
    qDebug() << "MainWindow::restoreDomainDataFromSettings() trying to decode ...";
    int errCode;
    QString errMsg;
    const QByteArray &recovered = decode(baDomains, COMPRESSION_ENABLED, &errCode, &errMsg);
    if (errCode != NO_CRYPT_ERROR) {
      wrongPasswordWarning(errCode, errMsg);
      return false;
    }
    QJsonParseError parseError;
    json = QJsonDocument::fromJson(recovered, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
      domainList = json.object().keys();
      qDebug() << "Password accepted. Restored" << domainList.count() << "domains";
      ui->statusBar->showMessage(tr("Password accepted. Restored %1 domains.").arg(domainList.count()), 5000);
    }
    else {
      QMessageBox::warning(this, tr("Bad data from sync server"),
                           tr("Decoding the data from the sync server failed: %1")
                           .arg(parseError.errorString()), QMessageBox::Ok);
    }
  }
  d->domains = DomainSettingsList::fromQJsonDocument(json);
  setDomainComboBox(domainList);
  return true;
}


void MainWindow::saveSettings(void)
{
  Q_D(MainWindow);
  int errCode;
  QString errMsg;
  d->settings.setValue("mainwindow/geometry", geometry());
  d->settings.setValue("misc/masterPasswordInvalidationTimeMins", d->optionsDialog->masterPasswordInvalidationTimeMins());
  d->settings.setValue("misc/saltLength", d->optionsDialog->saltLength());
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
  saveAllDomainDataToSettings();
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


bool MainWindow::restoreSettings(void)
{
  Q_D(MainWindow);
  int errCode = NO_CRYPT_ERROR;
  QString errMsg;
  restoreGeometry(d->settings.value("mainwindow/geometry").toByteArray());
  d->optionsDialog->setMasterPasswordInvalidationTimeMins(
        d->settings.value("misc/masterPasswordInvalidationTimeMins", DEFAULT_MASTER_PASSWORD_INVALIDATION_TIME_MINS).toInt());
  d->optionsDialog->setSaltLength(
        d->settings.value("misc/saltLength", DEFAULT_SALT_LENGTH).toInt());
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
    serverUsernameDecoded = decode(serverUsernameBin, false, &errCode, &errMsg);
    if (errCode == NO_CRYPT_ERROR) {
      d->optionsDialog->setServerUsername(QString::fromUtf8(serverUsernameDecoded));
    }
    else {
      qWarning() << "ERROR: decode() of server user name failed:" << errMsg;
    }
  }

  if (errCode != NO_CRYPT_ERROR)
    return false;

  const QByteArray &serverPassword = d->settings.value("sync/serverPassword").toByteArray();
  if (!serverPassword.isEmpty()) {
    const QByteArray &serverPasswordBin = QByteArray::fromHex(serverPassword);
    QByteArray password;
      password = decode(serverPasswordBin, false, &errCode, &errMsg);
    if (errCode == NO_CRYPT_ERROR) {
      d->optionsDialog->setServerPassword(QString::fromUtf8(password));
    }
    else {
      qWarning() << "ERROR: decode() of server password failed:" << errMsg;
    }
  }
  return errCode == NO_CRYPT_ERROR;
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
  if (d->readReq != nullptr && d->readReq->isRunning()) {
    d->readReq->abort();
    ui->statusBar->showMessage(tr("Server read operation aborted."), 3000);
  }
  if (d->writeReq != nullptr && d->writeReq->isRunning()) {
    d->writeReq->abort();
    ui->statusBar->showMessage(tr("Sync to server aborted."), 3000);
  }
}


void MainWindow::sync(void)
{
  Q_D(MainWindow);
  Q_ASSERT_X(!d->masterPassword.isEmpty(), "MainWindow::sync()", "d->masterPassword must not be empty");
  if (d->optionsDialog->useSyncFile()) {
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
  int errCode;
  QString errMsg;
  QJsonDocument remoteJSON;
  if (!remoteDomainsEncoded.isEmpty()) {
    std::string sDomains = decode(remoteDomainsEncoded, COMPRESSION_ENABLED, &errCode, &errMsg);
    QJsonParseError parseError;
    if (errCode == NO_CRYPT_ERROR && !sDomains.empty()) {
      remoteJSON = QJsonDocument::fromJson(QByteArray::fromStdString(sDomains), &parseError);
      if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("Bad data from sync server"),
                             tr("Decoding the data from the sync server failed: %1")
                             .arg(parseError.errorString()), QMessageBox::Ok);
      }
    }
    else {
      wrongPasswordWarning(errCode, errMsg);
      return;
    }
  }

  // merge local and remote domain data
  d->domains.setDirty(false);
  DomainSettingsList remoteDomains = DomainSettingsList::fromQJsonDocument(remoteJSON);
  QStringList allDomainNames = remoteDomains.keys() + d->domains.keys();
  allDomainNames.removeDuplicates();
  foreach(QString domainName, allDomainNames) {
    const DomainSettings &remote = remoteDomains.at(domainName);
    const DomainSettings &local = d->domains.at(domainName);
    if (!local.isEmpty() && !remote.isEmpty()) {
      if (remote.modifiedDate > local.modifiedDate) {
        d->domains.updateWith(remote);
      }
      else if (remote.modifiedDate < local.modifiedDate) {
        if (local.deleted) {
          remoteDomains.remove(domainName);
          d->domains.remove(domainName);
        }
        else {
          remoteDomains.updateWith(local);
        }
      }
      else {
        // timestamps are identical, do nothing
      }
    }
    else if (remote.isEmpty()) {
      if (local.canBeDeletedByRemote) {
        d->domains.remove(domainName);
      }
      else {
        DomainSettings ds = local;
        ds.canBeDeletedByRemote = true;
        d->domains.updateWith(ds);
        remoteDomains.updateWith(local);
      }
    }
    else {
      d->domains.updateWith(remote);
    }
  }

  if (remoteDomains.isDirty()) {
    int errCode;
    QString errMsg;
    const QByteArray &baCipher = encode(remoteDomains.toJson(), COMPRESSION_ENABLED, &errCode, &errMsg);
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
      throw std::string("encryption error");
    }
  }

  if (d->domains.isDirty()) {
    saveAllDomainDataToSettings();
    restoreDomainDataFromSettings();
    d->domains.setDirty(false);
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
  hide();
  d->masterPasswordDialog->setRepeatPassword(d->settings.value("mainwindow/masterPasswordEntered", false).toBool() == false);
  d->masterPasswordDialog->show();
  d->masterPasswordDialog->raise();
}


void MainWindow::masterPasswordEntered(void)
{
  Q_D(MainWindow);
  bool ok = true;
  QString masterPwd = d->masterPasswordDialog->masterPassword();
  if (!masterPwd.isEmpty()) {
    this->show();
    setEnabled(true);
    d->masterPasswordDialog->hide();
    d->masterPassword = masterPwd;
    d->cryptPassword.generate(d->masterPassword);
    d->cryptPassword.extractAESKey((char*)d->AESKey, AES_KEY_SIZE);
    ok = restoreSettings();
    if (ok) {
      ok = restoreDomainDataFromSettings();
      if (ok) {
        // updatePassword();
        d->settings.setValue("mainwindow/masterPasswordEntered", true);
        d->settings.sync();
        if (d->optionsDialog->syncOnStart())
          sync();
      }
    }
  }
  if (!ok ) {
    d->settings.setValue("mainwindow/masterPasswordEntered", false);
    emit badMasterPassword();
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
    emit badMasterPassword();
}


void MainWindow::invalidatePassword(bool reenter)
{
  Q_D(MainWindow);
  CryptoPP::memset_z(d->masterPassword.data(), 0, d->masterPassword.size());
  d->masterPassword = QByteArray();
  d->masterPasswordDialog->invalidatePassword();
  ui->statusBar->showMessage(tr("Master password cleared for security"));
  if (reenter)
    emit badMasterPassword();
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
    QJsonParseError parseError;
    const QJsonDocument &json = QJsonDocument::fromJson(res, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
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
      QMessageBox::warning(this, tr("Bad data from sync server"),
                           tr("Decoding the data from the sync server failed: %1")
                           .arg(parseError.errorString()), QMessageBox::Ok);
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


void MainWindow::createFullDump(void)
{
#if defined(QT_DEBUG)
#if defined(WIN32)
   make_minidump();
   ui->statusBar->showMessage(tr("Dump created."), 4000);
   qDebug() << "Mini dump created.";
#else
   ui->statusBar->showMessage(tr("Dump not implemented."), 4000);
   qDebug() << "Dump not implemented.";
#endif
#endif
}
