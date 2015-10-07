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

#include <QDebug>
#include <QObject>
#include <QClipboard>
#include <QMessageBox>
#include <QStringListModel>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkSession>
#include <QSslCipher>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslError>
#include <QSslKey>
#include <QUrlQuery>
#include <QProgressDialog>
#include <QSysInfo>
#include <QElapsedTimer>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QCompleter>
#include <QShortcut>

#include "singleinstancedetector.h"
#include "global.h"
#include "util.h"
#include "presets.h"
#include "progressdialog.h"
#include "masterpassworddialog.h"
#include "changemasterpassworddialog.h"
#include "optionsdialog.h"
#include "easyselectorwidget.h"
#include "countdownwidget.h"
#if HACKING_MODE_ENABLED
#include "hackhelper.h"
#endif
#include "pbkdf2.h"
#include "password.h"
#include "crypter.h"
#include "securebytearray.h"
#include "passwordchecker.h"

#include "dump.h"

#ifdef WIN32
#include "clipboardmonitor.h"
static const int SmartLoginNotActive = -1;
#endif

static const int DefaultMasterPasswordInvalidationTimeMins = 5;
static const bool CompressionEnabled = true;
static const int NotFound = -1;

static const QString DefaultSyncServerRoot = "https://syncserver.net/ctSESAM";
static const QString DefaultSyncServerUsername = "inter";
static const QString DefaultSyncServerPassword = "op";
static const QString DefaultSyncServerWriteUrl = "/ajax/write.php";
static const QString DefaultSyncServerReadUrl = "/ajax/read.php";
static const QString DefaultSyncServerDeleteUrl = "/ajax/delete.php";


class MainWindowPrivate {
public:
  MainWindowPrivate(QWidget *parent)
    : masterPasswordDialog(new MasterPasswordDialog(parent))
    , changeMasterPasswordDialog(new ChangeMasterPasswordDialog(parent))
    , optionsDialog(new OptionsDialog(parent))
    , progressDialog(new ProgressDialog(parent))
    , easySelector(new EasySelectorWidget)
    , countdownWidget(new CountdownWidget)
    , actionShow(Q_NULLPTR)
    , settings(QSettings::IniFormat, QSettings::UserScope, AppCompanyName, AppName)
    , loaderIcon(":/images/loader.gif")
    , customCharacterSetDirty(false)
    , parameterSetDirty(false)
#if HACKING_MODE_ENABLED
    , hackIterationDurationMs(0)
    , hackSalt(4, 0)
    , hackPermutations(1)
    , hackingMode(false)
#endif
    , trayIcon(QIcon(":/images/ctSESAM.ico"))
    , salt(Crypter::generateSalt())
    , masterKey(Crypter::AESKeySize, '\0')
    , IV(Crypter::AESBlockSize, '\0')
    , deleteReply(Q_NULLPTR)
    , readReply(Q_NULLPTR)
    , writeReply(Q_NULLPTR)
    , completer(Q_NULLPTR)
    , counter(0)
    , maxCounter(0)
    , masterPasswordChangeStep(0)
  #ifdef WIN32
    , smartLoginStep(SmartLoginNotActive)
  #endif
  {
    resetSSLConf();
  }
  ~MainWindowPrivate()
  {
    SecureErase(masterPassword);
  }
  void resetSSLConf(void)
  {
    sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setCiphers(QSslSocket::supportedCiphers());
  }
  MasterPasswordDialog *masterPasswordDialog;
  ChangeMasterPasswordDialog *changeMasterPasswordDialog;
  OptionsDialog *optionsDialog;
  ProgressDialog *progressDialog;
  EasySelectorWidget *easySelector;
  CountdownWidget *countdownWidget;
  QAction *actionShow;
  QString lastDomainBeforeLock;
  QString lastDomain;
  QSettings settings;
  DomainSettingsList domains;
  DomainSettingsList remoteDomains;
  DomainSettings lastDomainSettings;
  QMovie loaderIcon;
  bool customCharacterSetDirty;
  bool parameterSetDirty;
#if HACKING_MODE_ENABLED
  qint64 hackIterationDurationMs;
  QElapsedTimer hackClock;
  QElapsedTimer hackIterationClock;
  QByteArray hackSalt;
  PositionTable hackPos;
  qint64 hackPermutations;
  bool hackingMode;
#endif
  Password password;
  QDateTime createdDate;
  QDateTime modifiedDate;
  QSystemTrayIcon trayIcon;
  QByteArray salt;
  SecureByteArray masterKey;
  SecureByteArray IV;
  SecureByteArray KGK;
  QFuture<void> keyGenerationFuture;
  QMutex keyGenerationMutex;
  QString masterPassword;
  QSslConfiguration sslConf;
  QNetworkAccessManager deleteNAM;
  QNetworkAccessManager readNAM;
  QNetworkAccessManager writeNAM;
  QNetworkReply *deleteReply;
  QNetworkReply *readReply;
  QNetworkReply *writeReply;
  QString currentDomain;
  QCompleter *completer;
  int counter;
  int maxCounter;
  int masterPasswordChangeStep;
  QAction *undoAction;
#ifdef WIN32
  int smartLoginStep;
#endif
};


MainWindow::MainWindow(bool forceStart, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , d_ptr(new MainWindowPrivate(this))
{
  Q_D(MainWindow);

  if (!forceStart && SingleInstanceDetector::instance().alreadyRunning()) {
    QMessageBox::information(Q_NULLPTR, QObject::tr("%1 can run only once").arg(AppName), QObject::tr("Only one instance of %1 can run at a time.").arg(AppName));
    SingleInstanceDetector::instance().detach();
    close();
    ::exit(1);
  }
  SingleInstanceDetector::instance().attach();

  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctSESAM.ico"));

  ui->selectorGridLayout->addWidget(d->easySelector, 0, 1);
  QObject::connect(d->easySelector, SIGNAL(valuesChanged(int, int)), SLOT(onEasySelectorValuesChanged(int, int)));
  QObject::connect(d->optionsDialog, SIGNAL(maxPasswordLengthChanged(int)), d->easySelector, SLOT(setMaxLength(int)));
  resetAllFields();

  new QShortcut(QKeySequence("Esc"), this, SLOT(onEscPressed()));

  QObject::connect(ui->domainsComboBox, SIGNAL(editTextChanged(QString)), SLOT(onDomainTextChanged(QString)));
  QObject::connect(ui->domainsComboBox, SIGNAL(activated(QString)), SLOT(onDomainSelected(QString)));
  ui->domainsComboBox->installEventFilter(this);
  QObject::connect(ui->userLineEdit, SIGNAL(textChanged(QString)), SLOT(onUserChanged(QString)));
  ui->userLineEdit->installEventFilter(this);
  QObject::connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), SLOT(onURLChanged(QString)));
  ui->urlLineEdit->installEventFilter(this);
  QObject::connect(ui->openURLPushButton, SIGNAL(pressed()), SLOT(openURL()));
  QObject::connect(ui->legacyPasswordLineEdit, SIGNAL(textEdited(QString)), SLOT(onLegacyPasswordChanged(QString)));
  ui->legacyPasswordLineEdit->installEventFilter(this);
  QObject::connect(ui->notesPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  ui->notesPlainTextEdit->installEventFilter(this);
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(onUsedCharactersChanged()));
  ui->usedCharactersPlainTextEdit->installEventFilter(this);
  QObject::connect(ui->extraLineEdit, SIGNAL(textChanged(QString)), SLOT(onExtraCharactersChanged(QString)));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(onPasswordLengthChanged(int)));
  ui->passwordLengthSpinBox->installEventFilter(this);
  QObject::connect(ui->deleteCheckBox, SIGNAL(toggled(bool)), SLOT(onDeleteChanged(bool)));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(onIterationsChanged(int)));
  ui->iterationsSpinBox->installEventFilter(this);
  QObject::connect(ui->saltBase64LineEdit, SIGNAL(textChanged(QString)), SLOT(onSaltChanged(QString)));
  ui->saltBase64LineEdit->installEventFilter(this);
  ui->generatedPasswordLineEdit->installEventFilter(this);
  QObject::connect(ui->passwordTemplateLineEdit, SIGNAL(textChanged(QString)), SLOT(onPasswordTemplateChanged(QString)));
  QObject::connect(ui->copyGeneratedPasswordToClipboardPushButton, SIGNAL(clicked()), SLOT(copyGeneratedPasswordToClipboard()));
  QObject::connect(ui->copyLegacyPasswordToClipboardPushButton, SIGNAL(clicked()), SLOT(copyLegacyPasswordToClipboard()));
  QObject::connect(ui->copyUsernameToClipboardPushButton, SIGNAL(clicked()), SLOT(copyUsernameToClipboard()));
  QObject::connect(ui->renewSaltPushButton, SIGNAL(clicked()), SLOT(onRenewSalt()));
  QObject::connect(ui->actionSave, SIGNAL(triggered(bool)), SLOT(saveCurrentDomainSettings()));
  QObject::connect(ui->actionClearAllSettings, SIGNAL(triggered(bool)), SLOT(clearAllSettings()));
  QObject::connect(ui->actionSyncNow, SIGNAL(triggered(bool)), SLOT(sync()));
  QObject::connect(ui->actionLockApplication, SIGNAL(triggered(bool)), SLOT(lockApplication()));
  QObject::connect(ui->actionClearClipboard, SIGNAL(triggered(bool)), SLOT(clearClipboard()));
  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));
  QObject::connect(ui->actionOptions, SIGNAL(triggered(bool)), SLOT(showOptionsDialog()));
  QObject::connect(d->optionsDialog, SIGNAL(serverCertificatesUpdated(QList<QSslCertificate>)), SLOT(onServerCertificatesUpdated(QList<QSslCertificate>)));
  QObject::connect(d->masterPasswordDialog, SIGNAL(accepted()), SLOT(onMasterPasswordEntered()));
  QObject::connect(d->countdownWidget, SIGNAL(timeout()), SLOT(lockApplication()));
  QObject::connect(ui->actionChangeMasterPassword, SIGNAL(triggered(bool)), SLOT(changeMasterPassword()));
#if HACKING_MODE_ENABLED
  QObject::connect(ui->actionHackLegacyPassword, SIGNAL(triggered(bool)), SLOT(hackLegacyPassword()));
#else
  ui->actionHackLegacyPassword->setVisible(false);
#endif
  QObject::connect(ui->actionRegenerateSaltKeyIV, SIGNAL(triggered(bool)), SLOT(generateSaltKeyIV()));
  QObject::connect(this, SIGNAL(saltKeyIVGenerated()), SLOT(onGenerateSaltKeyIV()), Qt::ConnectionType::QueuedConnection);
  QObject::connect(d->progressDialog, SIGNAL(cancelled()), SLOT(cancelServerOperation()));

  QObject::connect(&d->password, SIGNAL(generated()), SLOT(onPasswordGenerated()));
  QObject::connect(&d->password, SIGNAL(generationAborted()), SLOT(onPasswordGenerationAborted()));
  QObject::connect(&d->password, SIGNAL(generationStarted()), SLOT(onPasswordGenerationStarted()));

  QObject::connect(&d->deleteNAM, SIGNAL(finished(QNetworkReply*)), SLOT(onDeleteFinished(QNetworkReply*)));
  QObject::connect(&d->deleteNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));
  QObject::connect(&d->readNAM, SIGNAL(finished(QNetworkReply*)), SLOT(onReadFinished(QNetworkReply*)));
  QObject::connect(&d->readNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));
  QObject::connect(&d->writeNAM, SIGNAL(finished(QNetworkReply*)), SLOT(onWriteFinished(QNetworkReply*)));
  QObject::connect(&d->writeNAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));

  QObject::connect(&d->trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
  QMenu *trayMenu = new QMenu(AppName);
  d->actionShow = trayMenu->addAction(tr("Minimize window"));
  QObject::connect(d->actionShow, SIGNAL(triggered(bool)), SLOT(showHide()));
  QAction *actionSync = trayMenu->addAction(tr("Sync"));
  QObject::connect(actionSync, SIGNAL(triggered(bool)), SLOT(sync()));
  QAction *actionClearClipboard = trayMenu->addAction(tr("Clear clipboard"));
  QObject::connect(actionClearClipboard, SIGNAL(triggered(bool)), SLOT(clearClipboard()));
  QAction *actionLockApplication = trayMenu->addAction(tr("Lock application ..."));
  QObject::connect(actionLockApplication, SIGNAL(triggered(bool)), SLOT(lockApplication()));
  QAction *actionAbout = trayMenu->addAction(tr("About %1").arg(AppName));
  QObject::connect(actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QAction *actionQuit = trayMenu->addAction(tr("Quit"));
  QObject::connect(actionQuit, SIGNAL(triggered(bool)), SLOT(close()));
  d->trayIcon.setContextMenu(trayMenu);
  d->trayIcon.show();



#ifdef WIN32
  QObject::connect(ClipboardMonitor::instance(), SIGNAL(pasted()), SLOT(onPasted()));
#ifdef QT_DEBUG
  ui->menuExtras->addAction(tr("[DEBUG] Create Mini Dump"), this, SLOT(createFullDump()), QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_D));
#endif
#endif

  ui->statusBar->addPermanentWidget(d->countdownWidget);
  setDirty(false);
  ui->tabWidget->setCurrentIndex(0);
  ui->tabWidgetVersions->setCurrentIndex(1);
  enterMasterPassword();
}


void MainWindow::showHide(void)
{
  Q_D(MainWindow);
  if (d->masterPasswordDialog->isVisible()) {
    d->masterPasswordDialog->raise();
    d->masterPasswordDialog->activateWindow();
    d->masterPasswordDialog->setFocus();
    return;
  }
  if (isMinimized()) {
    show();
    showNormal();
    raise();
    activateWindow();
    setFocus();
    d->actionShow->setText(tr("Minimize window"));
  }
  else {
    showMinimized();
    d->actionShow->setText(tr("Restore window"));
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
  d_ptr->optionsDialog->close();
  d_ptr->changeMasterPasswordDialog->close();
  d_ptr->masterPasswordDialog->close();
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent *e)
{
  Q_D(MainWindow);

  if (d->masterPasswordDialog->masterPassword().isEmpty()) {
    e->ignore();
    return;
  }

  auto prepareExit = [this]() {
    d_ptr->masterPasswordDialog->close();
    d_ptr->optionsDialog->close();
    d_ptr->trayIcon.hide();
    saveSettings();
    invalidatePassword(false);
    d_ptr->keyGenerationFuture.waitForFinished();
  };

  cancelPasswordGeneration();

  QMessageBox::StandardButton rc = (d->parameterSetDirty && !ui->domainsComboBox->currentText().isEmpty())
      ? QMessageBox::question(
          this,
          tr("Save before exit?"),
          tr("Your domain parameters have changed. Do you want to save the changes before exiting?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes)
      : QMessageBox::No;
  switch (rc) {
  case QMessageBox::Yes:
    saveCurrentDomainSettings();
    prepareExit();
    e->accept();
    break;
  case QMessageBox::Cancel:
    e->ignore();
    break;
  case QMessageBox::No:
    prepareExit();
    e->accept();
    break;
  default:
    qWarning() << "Oops! Should never have come here. rc =" << rc;
    break;
  }

  SingleInstanceDetector::instance().detach();
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
      QTimer::singleShot(200, this, SLOT(showMinimized()));
    }
    break;
  }
  default:
    break;
  }
}


void MainWindow::resizeEvent(QResizeEvent *)
{
  restartInvalidationTimer();
}


void MainWindow::moveEvent(QMoveEvent *)
{
  restartInvalidationTimer();
}


void MainWindow::resetAllFieldsExceptDomainComboBox(void)
{
  Q_D(MainWindow);
  ui->userLineEdit->setText(QString());
  ui->urlLineEdit->blockSignals(true);
  ui->urlLineEdit->setText(QString());
  ui->urlLineEdit->blockSignals(false);
  ui->generatedPasswordLineEdit->setText(QString());
  ui->legacyPasswordLineEdit->setText(QString());
  ui->saltBase64LineEdit->blockSignals(true);
  renewSalt();
  ui->saltBase64LineEdit->blockSignals(false);
  ui->iterationsSpinBox->blockSignals(true);
  ui->iterationsSpinBox->setValue(DomainSettings::DefaultIterations);
  ui->iterationsSpinBox->blockSignals(false);
  ui->passwordLengthSpinBox->blockSignals(true);
  ui->passwordLengthSpinBox->setValue(DomainSettings::DefaultPasswordLength);
  ui->passwordLengthSpinBox->blockSignals(false);
  ui->notesPlainTextEdit->setPlainText(QString());
  ui->usedCharactersPlainTextEdit->blockSignals(true);
  ui->usedCharactersPlainTextEdit->setPlainText(Password::AllChars);
  ui->usedCharactersPlainTextEdit->blockSignals(false);
  ui->createdLabel->setText(QString());
  ui->modifiedLabel->setText(QString());
  ui->deleteCheckBox->setChecked(false);
  // v3
  ui->extraLineEdit->blockSignals(true);
  ui->extraLineEdit->setText(Password::ExtraChars);
  ui->extraLineEdit->blockSignals(false);
  d->easySelector->blockSignals(true);
  d->easySelector->setLength(d->optionsDialog->maxPasswordLength() / 2);
  d->easySelector->setComplexity(Password::DefaultComplexity);
  d->easySelector->blockSignals(false);
  ui->useLowerCaseCheckBox->setChecked(true);
  ui->useUpperCaseCheckBox->setChecked(true);
  ui->useDigitsCheckBox->setChecked(true);
  ui->useExtraCheckBox->setChecked(true);
  setDirty(false);
}


void MainWindow::resetAllFields(void)
{
  Q_D(MainWindow);
  resetAllFieldsExceptDomainComboBox();
  ui->domainsComboBox->setCurrentIndex(-1);
  ui->domainsComboBox->setFocus();
}


int MainWindow::findDomainInComboBox(const QString &domain, int lo, int hi) const {
  if (hi < lo)
    return NotFound;
  const int idx = (lo + hi) / 2;
  const int c = ui->domainsComboBox->itemText(idx).compare(domain, Qt::CaseInsensitive);
  if (c > 0)
    return findDomainInComboBox(domain, lo, idx - 1);
  else if (c < 0)
    return findDomainInComboBox(domain, idx + 1, hi);
  return idx;
}


int MainWindow::findDomainInComboBox(const QString &domain) const {
  return findDomainInComboBox(domain, 0, ui->domainsComboBox->count());
}


bool MainWindow::domainComboboxContains(const QString &domain) const {
  return findDomainInComboBox(domain, 0, ui->domainsComboBox->count()) != NotFound;
}


void MainWindow::renewSalt(void)
{
  Q_D(MainWindow);
  const QByteArray &salt = Crypter::randomBytes(d->optionsDialog->saltLength());
  ui->saltBase64LineEdit->setText(salt.toBase64());
}


void MainWindow::onRenewSalt(void)
{
  int button = QMessageBox::Yes;
  if (domainComboboxContains(ui->domainsComboBox->currentText())) {
    button = QMessageBox::question(
          this,
          tr("Really renew salt?"),
          tr("Renewing the salt will invalidate your current generated password. Are you sure you want to generate a new salt?"),
          QMessageBox::Yes,
          QMessageBox::No);
  }
  if (button == QMessageBox::Yes)
    renewSalt();
}


QMessageBox::StandardButton MainWindow::saveOrDiscard(void)
{
  return QMessageBox::question(
        this,
        tr("Save changes?"),
        tr("You have changed the current domain settings. "
           "Do you want to save or discard the changes before proceeding?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
}


QMessageBox::StandardButton MainWindow::checkSaveOnDirty(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::checkSaveOnDirty()";
  QMessageBox::StandardButton rc = QMessageBox::NoButton;
  if (d->parameterSetDirty) {
    rc = saveOrDiscard();
  }
  return rc;
}


void MainWindow::cancelPasswordGeneration(void)
{
  Q_D(MainWindow);
#if HACKING_MODE_ENABLED
  if (d->hackingMode) {
    d->hackingMode = false;
    ui->renewSaltPushButton->setEnabled(true);
    ui->legacyPasswordLineEdit->setReadOnly(false);
  }
#endif
  stopPasswordGeneration();
}


void MainWindow::setDirty(bool dirty)
{
  Q_D(MainWindow);
  d->parameterSetDirty = dirty;
  updateWindowTitle();
}


void MainWindow::openURL(void)
{
  Q_D(MainWindow);
  if (!ui->urlLineEdit->text().isEmpty()) {
    QDesktopServices::openUrl(QUrl(ui->urlLineEdit->text()));
    copyUsernameToClipboard();
  }
}


void MainWindow::onURLChanged(QString)
{
  setDirty();
  restartInvalidationTimer();
  ui->openURLPushButton->setEnabled(!ui->urlLineEdit->text().isEmpty());
}


void MainWindow::onUserChanged(QString)
{
  setDirty();
  updatePassword();
}


void MainWindow::onUsedCharactersChanged(void)
{
  setDirty();
  updatePassword();
}


void MainWindow::onExtraCharactersChanged(QString)
{
  setDirty();
  createTemplate_v3();
  updatePassword();
}


void MainWindow::onPasswordLengthChanged(int len)
{
  Q_D(MainWindow);
  setDirty();
  d->easySelector->setLength(len);
  updatePassword();
}


void MainWindow::onIterationsChanged(int)
{
  setDirty();
  updatePassword();
}


void MainWindow::onSaltChanged(QString)
{
  setDirty();
  restartInvalidationTimer();
  updatePassword();
}


void MainWindow::onDeleteChanged(bool)
{
  setDirty();
  restartInvalidationTimer();
}


void MainWindow::restartInvalidationTimer(void)
{
  Q_D(MainWindow);
  const int timeout = d->optionsDialog->masterPasswordInvalidationTimeMins();
  if (timeout > 0) {
    d->countdownWidget->start(timeout * 60 * 1000);
  }
  else {
    d->countdownWidget->stop();
  }
}


void MainWindow::onPasswordGenerationStarted(void)
{
  // do nothing
}


void MainWindow::updatePassword(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::updatePassword() triggered by" << (sender() ? sender()->objectName() : "NONE");
  if (!d->masterPassword.isEmpty()) {
    if (ui->legacyPasswordLineEdit->text().isEmpty()) {
      stopPasswordGeneration();
#if HACKING_MODE_ENABLED
      if (!d->hackingMode) {
        ui->generatedPasswordLineEdit->setText(QString());
        ui->statusBar->showMessage(QString());
      }
#endif
      generatePassword();
    }
    else {
      ui->generatedPasswordLineEdit->setText(QString());
    }
    restartInvalidationTimer();
  }
}


DomainSettings MainWindow::collectedDomainSettings(void) const
{
  DomainSettings ds;
  ds.domainName = ui->domainsComboBox->currentText();
  ds.url = ui->urlLineEdit->text();
  ds.deleted = ui->deleteCheckBox->isChecked();
  ds.createdDate = d_ptr->createdDate.isValid() ? d_ptr->createdDate : QDateTime::currentDateTime();
  ds.modifiedDate = d_ptr->modifiedDate.isValid() ? d_ptr->modifiedDate : QDateTime::currentDateTime();
  ds.userName = ui->userLineEdit->text();
  ds.notes = ui->notesPlainTextEdit->toPlainText();
  ds.salt_base64 = ui->saltBase64LineEdit->text();
  ds.legacyPassword = ui->legacyPasswordLineEdit->text();
  ds.iterations = ui->iterationsSpinBox->value();
  ds.passwordLength = ui->passwordLengthSpinBox->value();
  ds.usedCharacters = ui->usedCharactersPlainTextEdit->toPlainText();
  // v3
  ds.extraCharacters = ui->extraLineEdit->text();
  ds.passwordTemplate = ui->passwordTemplateLineEdit->text().toUtf8();
  return ds;
}


void MainWindow::analyzeTemplate_v3(const QByteArray &templ)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::analyzeTemplate_v3(" << templ << ")";
  const QList<QByteArray> &templateParts = templ.split(';');
  if (templateParts.count() != 2)
    return;
  int complexity = templateParts.at(0).toInt();
  int length = templateParts.at(1).length();

  d->easySelector->blockSignals(true);
  d->easySelector->setComplexity(complexity);
  d->easySelector->setLength(length);
  d->easySelector->blockSignals(false);

  const QBitArray &ba = Password::deconstructedComplexity(complexity);
  ui->useDigitsCheckBox->setChecked(ba.at(0));
  ui->useLowerCaseCheckBox->setChecked(ba.at(1));
  ui->useUpperCaseCheckBox->setChecked(ba.at(2));
  ui->useExtraCheckBox->setChecked(ba.at(3) && !ui->extraLineEdit->text().isEmpty());

  ui->passwordLengthSpinBox->blockSignals(true);
  ui->passwordLengthSpinBox->setValue(templateParts.at(1).length());
  ui->passwordLengthSpinBox->blockSignals(false);

  usedCharacters_v3();
}


QString MainWindow::usedCharacters_v3(void)
{
  QString used;
  if (ui->useLowerCaseCheckBox->isChecked())
    used += Password::LowerChars;
  if (ui->useUpperCaseCheckBox->isChecked())
    used += Password::UpperChars;
  if (ui->useDigitsCheckBox->isChecked())
    used += Password::Digits;
  if (ui->useExtraCheckBox->isChecked())
    used += ui->extraLineEdit->text();
  return used;
}


void MainWindow::createTemplate_v3(void)
{
  Q_D(MainWindow);
  QByteArray used;
  if (ui->useLowerCaseCheckBox->isChecked())
    used += 'a';
  if (ui->useUpperCaseCheckBox->isChecked())
    used += 'A';
  if (ui->useDigitsCheckBox->isChecked())
    used += 'n';
  if (ui->useExtraCheckBox->isChecked())
    used += 'o';
  QByteArray pwdTemplate = used + QByteArray(d->easySelector->length() - used.count(), 'x');
  ui->passwordTemplateLineEdit->setText(QString("%1").arg(d->easySelector->complexity()).toUtf8() + ';' + shuffled(QString::fromUtf8(pwdTemplate)));
  ui->usedCharactersPlainTextEdit->blockSignals(true);
  ui->usedCharactersPlainTextEdit->setPlainText(usedCharacters_v3());
  ui->usedCharactersPlainTextEdit->blockSignals(false);
}


void MainWindow::generatePassword(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::generatePassword()";
  if (ui->usedCharactersPlainTextEdit->toPlainText().isEmpty()) {
    ui->generatedPasswordLineEdit->setText(QString());
  }
  else {
    d->password.generateAsync(d->KGK, collectedDomainSettings());
  }
}


void MainWindow::hideActivityIcons(void)
{
  Q_D(MainWindow);
  d->loaderIcon.stop();
  ui->copyGeneratedPasswordToClipboardPushButton->show();
}


void MainWindow::stopPasswordGeneration(void)
{
  Q_D(MainWindow);
  if (d->password.isRunning()) {
    d->password.abortGeneration();
    d->password.waitForFinished();
  }
}


void MainWindow::changeMasterPassword(void)
{
  Q_D(MainWindow);
  d->changeMasterPasswordDialog->setPasswordFilename(d->optionsDialog->passwordFilename());
  const int rc = d->changeMasterPasswordDialog->exec();
  if ((rc == QDialog::Accepted) && (d->changeMasterPasswordDialog->oldPassword() == d->masterPassword)) {
    if (syncToServerEnabled() || syncToFileEnabled()) {
      d->masterPasswordChangeStep = 1;
      nextChangeMasterPasswordStep();
    }
    else {
      saveAllDomainDataToSettings();
      d->masterPassword = d->changeMasterPasswordDialog->newPassword();
      d->keyGenerationFuture.waitForFinished();
      generateSaltKeyIV().waitForFinished();
    }
  }
}


void MainWindow::nextChangeMasterPasswordStep(void)
{
  Q_D(MainWindow);
  switch (d->masterPasswordChangeStep++) {
  case 1:
    d->progressDialog->show();
    d->progressDialog->raise();
    d->progressDialog->setText(tr("Starting synchronisation ..."));
    d->progressDialog->setRange(1, 3);
    d->progressDialog->setValue(1);
    saveAllDomainDataToSettings();
    sync();
    if (!syncToServerEnabled())
      nextChangeMasterPasswordStep();
    break;
  case 2:
    d->progressDialog->setValue(2);
    d->masterPassword = d->changeMasterPasswordDialog->newPassword();
    generateSaltKeyIV().waitForFinished();
    d->progressDialog->setText(tr("Writing to sync peers ..."));
    if (d->optionsDialog->useSyncFile()) {
      writeToRemote(SyncPeerFile);
      if (!syncToServerEnabled())
        nextChangeMasterPasswordStep();
    }
    if (syncToServerEnabled()) {
      writeToRemote(SyncPeerServer);
    }
    break;
  case 3:
    d->masterPasswordChangeStep = 0;
    d->progressDialog->setText(tr("Password changed."));
    d->progressDialog->setValue(3);
    break;
  default:
    // ignore
    break;
  }
}


auto makeHMS = [](qint64 ms) {
  QString sign;
  if (ms < 0) {
    sign = "-";
    ms = -ms;
  }
  else {
    sign = "";
  }
  qint64 secs = ms / 1000;
  qint64 hrs = secs / 60 / 60;
  qint64 mins = (secs / 60 - hrs * 60);
  secs -= 60 * (hrs * 60 + mins);
  return QString("%1%2h%3'%4\"").arg(sign).arg(hrs).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
};


void MainWindow::onPasswordGenerated(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::onPasswordGenerated()";
#if HACKING_MODE_ENABLED
  if (!d->hackingMode) {
#endif
    ui->generatedPasswordLineEdit->setText(d->password());
    if (!d->password.isAborted())
      ui->statusBar->showMessage(tr("generation time: %1 ms")
                                 .arg(1e3 * d->password.elapsedSeconds(), 0, 'f', 4), 3000);
#if HACKING_MODE_ENABLED
  }
  else { // in hacking mode
    ui->generatedPasswordLineEdit->setText(d->password());
    PositionTable st(d->password());
    if (d->hackPos == st) {
      const QString &newCharTable = d->hackPos.substitute(st, usedCharacters());
      ui->usedCharactersPlainTextEdit->setPlainText(newCharTable);
      d->hackingMode = false;
      ui->renewSaltPushButton->setEnabled(true);
      ui->legacyPasswordLineEdit->setReadOnly(false);
      hideActivityIcons();
      int button = QMessageBox::question(
            this,
            tr("Finished \"hacking\""),
            tr("Found a salt in %1 that allows to calculate the legacy password from the domain settings :-) "
               "The legacy password is no longer needed. "
               "Do you want to clear the legacy password and save the new domain settings?").arg(makeHMS(d->hackClock.elapsed())));
      if (button == QMessageBox::Yes) {
        ui->legacyPasswordLineEdit->setText(QString());
        ui->tabWidget->setCurrentIndex(0);
        saveCurrentDomainSettings();
      }
      restartInvalidationTimer();
    }
    else {
      const qint64 dt = d->hackIterationClock.restart();
      d->hackIterationDurationMs = (d->hackIterationDurationMs > 0)
          ? (d->hackIterationDurationMs + dt) / 2
          : dt;
      ui->statusBar->showMessage(
            tr("Hacking ... t%1 (%2ms) t: %3")
            .arg(makeHMS(d->hackClock.elapsed() - 3 * d->hackPermutations * d->hackIterationDurationMs / 2))
            .arg(dt)
            .arg(makeHMS(d->hackClock.elapsed()))
            );
      incrementEndianless(d->hackSalt);
      ui->saltBase64LineEdit->setText(d->hackSalt.toBase64());
    }
  }
#endif
}


void MainWindow::onPasswordGenerationAborted(void)
{
  onPasswordGenerated();
}


void MainWindow::onOptionsAccepted(void)
{
  Q_D(MainWindow);
  restartInvalidationTimer();
  saveSettings();
}


void MainWindow::onServerCertificatesUpdated(const QList<QSslCertificate> &certs)
{
  Q_D(MainWindow);
  d->deleteNAM.clearAccessCache();
  d->readNAM.clearAccessCache();
  d->writeNAM.clearAccessCache();
  d->resetSSLConf();
  if (!certs.isEmpty())
    d->sslConf.setCaCertificates(certs);
}


void MainWindow::showOptionsDialog(void)
{
  Q_D(MainWindow);
  int button = d->optionsDialog->exec();
  if (button == QDialog::Accepted)
    onOptionsAccepted();
}


QFuture<void> &MainWindow::generateSaltKeyIV(void)
{
  Q_D(MainWindow);
  d->keyGenerationFuture = QtConcurrent::run(this, &MainWindow::generateSaltKeyIVThread);
  return d->keyGenerationFuture;
}


void MainWindow::generateSaltKeyIVThread(void)
{
  Q_D(MainWindow);
  QMutexLocker(&d->keyGenerationMutex);
  d->salt = Crypter::generateSalt();
  Crypter::makeKeyAndIVFromPassword(d->masterPassword.toUtf8(), d->salt, d->masterKey, d->IV);
  emit saltKeyIVGenerated();
}


void MainWindow::onGenerateSaltKeyIV(void)
{
  Q_D(MainWindow);
  ui->statusBar->showMessage(tr("Auto-generated new salt (%1) and key.").arg(QString::fromLatin1(d->salt.mid(0, 4).toHex())), 2000);
}


#ifdef WIN32
void MainWindow::onPasted(void)
{
  Q_D(MainWindow);
  if (!d->optionsDialog->smartLogin())
    return;
  switch (d->smartLoginStep) {
  case 0:
  {
    if (ui->legacyPasswordLineEdit->text().isEmpty()) {
      copyGeneratedPasswordToClipboard();
    }
    else {
      copyLegacyPasswordToClipboard();
    }
    break;
  }
  case 1:
  {
    clearClipboard();
    d->smartLoginStep = SmartLoginNotActive;
    break;
  }
  default:
    break;
  }
}
#endif


void MainWindow::copyUsernameToClipboard(void)
{
#ifdef WIN32
  Q_D(MainWindow);
  d->smartLoginStep = 0;
#endif
  QApplication::clipboard()->setText(ui->userLineEdit->text());
  ui->statusBar->showMessage(tr("Username copied to clipboard."), 5000);
}


void MainWindow::copyGeneratedPasswordToClipboard(void)
{
#ifdef WIN32
  Q_D(MainWindow);
  d->smartLoginStep = 1;
#endif
  QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Generated password copied to clipboard."), 3000);
}


void MainWindow::copyLegacyPasswordToClipboard(void)
{
#ifdef WIN32
  Q_D(MainWindow);
  d->smartLoginStep = 1;
#endif
  QApplication::clipboard()->setText(ui->legacyPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Legacy password copied to clipboard."), 5000);
}


void MainWindow::copyDomainSettingsToGUI(const QString &domain)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::copyDomainSettingsToGUI(" << domain << ")";
  const DomainSettings &p = d->domains.at(domain);
  ui->domainsComboBox->blockSignals(true);
  ui->domainsComboBox->setCurrentText(p.domainName);
  ui->domainsComboBox->blockSignals(false);
  ui->urlLineEdit->setText(p.url);
  ui->userLineEdit->blockSignals(true);
  ui->userLineEdit->setText(p.userName);
  ui->userLineEdit->blockSignals(false);
  ui->legacyPasswordLineEdit->setText(p.legacyPassword);
  ui->saltBase64LineEdit->blockSignals(true);
  ui->saltBase64LineEdit->setText(p.salt_base64);
  ui->saltBase64LineEdit->blockSignals(false);
  ui->notesPlainTextEdit->setPlainText(p.notes);
  ui->usedCharactersPlainTextEdit->blockSignals(true);
  ui->usedCharactersPlainTextEdit->setPlainText(p.usedCharacters);
  ui->usedCharactersPlainTextEdit->blockSignals(false);
  ui->iterationsSpinBox->blockSignals(true);
  ui->iterationsSpinBox->setValue(p.iterations);
  ui->iterationsSpinBox->blockSignals(false);
  ui->passwordLengthSpinBox->blockSignals(true);
  ui->passwordLengthSpinBox->setValue(p.passwordLength);
  ui->passwordLengthSpinBox->blockSignals(false);
  ui->createdLabel->setText(p.createdDate.toString(Qt::ISODate));
  ui->modifiedLabel->setText(p.modifiedDate.toString(Qt::ISODate));
  d->createdDate = p.createdDate;
  d->modifiedDate = p.modifiedDate;
  ui->deleteCheckBox->setChecked(false);
  // v3
  ui->extraLineEdit->blockSignals(true);
  ui->extraLineEdit->setText(p.extraCharacters);
  ui->extraLineEdit->blockSignals(false);

  analyzeTemplate_v3(p.passwordTemplate);
  ui->passwordTemplateLineEdit->blockSignals(true);
  ui->passwordTemplateLineEdit->setText(p.passwordTemplate);
  ui->passwordTemplateLineEdit->blockSignals(false);

  updatePassword();
}


void MainWindow::makeDomainComboBox(void)
{
  Q_D(MainWindow);
  QStringList domainNames;
  ui->domainsComboBox->blockSignals(true);
  ui->domainsComboBox->clear();
  foreach(DomainSettings ds, d->domains)
    if (!ds.deleted)
      domainNames.append(ds.domainName);
  domainNames.sort(Qt::CaseInsensitive);
  ui->domainsComboBox->addItems(domainNames);
  if (d->completer != Q_NULLPTR) {
    QObject::disconnect(d->completer, SIGNAL(activated(QString)), this, SLOT(onDomainSelected(QString)));
    delete d->completer;
  }
  d->completer = new QCompleter(domainNames);
  d->completer->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(d->completer, SIGNAL(activated(QString)), this, SLOT(onDomainSelected(QString)));
  ui->domainsComboBox->setCompleter(d->completer);
  ui->domainsComboBox->setCurrentIndex(-1);
  ui->domainsComboBox->blockSignals(false);
}


void MainWindow::saveCurrentDomainSettings(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::saveCurrentDomainSettings() called by" << (sender() ? sender()->objectName() : "NONE");

  DomainSettings ds = collectedDomainSettings();

  if (ds.usedCharacters.isEmpty()) {
    QMessageBox::warning(this, tr("Empty character table"), tr("You forgot to fill in some characters into the field \"used characters\""));
    return;
  }

  ui->createdLabel->setText(ds.createdDate.toString(Qt::ISODate));
  ui->modifiedLabel->setText(ds.modifiedDate.toString(Qt::ISODate));

  const QString currentDomain = ui->domainsComboBox->currentText();

  QStringList domainList;
  for (int i = 0; i < ui->domainsComboBox->count(); ++i)
    domainList.append(ui->domainsComboBox->itemText(i));

  if (domainList.contains(ds.domainName, Qt::CaseInsensitive)) {
    ds.modifiedDate = QDateTime::currentDateTime();
    if (ds.deleted) {
      domainList.removeOne(ds.domainName);
      resetAllFields();
    }
  }
  else {
    ds.createdDate = QDateTime::currentDateTime();
    ds.modifiedDate = ds.createdDate;
    if (!ds.deleted)
      domainList << ds.domainName;
  }
  d->domains.updateWith(ds);

  makeDomainComboBox();
  ui->domainsComboBox->blockSignals(true);
  ui->domainsComboBox->setCurrentText(currentDomain);
  ui->domainsComboBox->blockSignals(false);

  saveAllDomainDataToSettings();

  setDirty(false);

  if (ds.deleted)
    resetAllFields();

  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
}


void MainWindow::onLegacyPasswordChanged(QString legacyPassword)
{
  setDirty();
  ui->actionHackLegacyPassword->setEnabled(!legacyPassword.isEmpty());
}


void MainWindow::writeBackupFile(const QByteArray &binaryDomainData)
{
  const QString &backupFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  const QString &backupFilename = QString("%1/%2-%3-domaindata-backup.txt")
      .arg(backupFilePath)
      .arg(QDateTime::currentDateTime().toString("yyyyMMddThhmmss"))
      .arg(AppName);
  if (QDir().mkpath(backupFilePath)) {
    QFile backupFile(backupFilename);
    backupFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    backupFile.write(binaryDomainData);
    backupFile.close();
  }
}


void MainWindow::saveAllDomainDataToSettings(void)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::saveAllDomainDataToSettings()";
  d->keyGenerationMutex.lock();
  QByteArray cipher;
  try {
    cipher = Crypter::encode(d->masterKey, d->IV, d->salt, d->KGK, d->domains.toJson(), CompressionEnabled);
  }
  catch (CryptoPP::Exception &e) {
    qErrnoWarning((int)e.GetErrorType(), e.what());
  }
  d->keyGenerationMutex.unlock();

  const QByteArray &binaryDomainData = cipher.toBase64();
  d->settings.setValue("sync/domains", QString::fromUtf8(binaryDomainData));
  d->settings.sync();

  if (d->masterPasswordChangeStep == 0) {
    generateSaltKeyIV();
    if (d->optionsDialog->writeBackups())
      writeBackupFile(binaryDomainData);
  }
}


bool MainWindow::restoreDomainDataFromSettings(void)
{
  Q_D(MainWindow);
  Q_ASSERT_X(!d->masterPassword.isEmpty(), "MainWindow::restoreDomainDataFromSettings()", "d->masterPassword must not be empty");
  QJsonDocument json;
  QStringList domainList;
  const QByteArray &domains = QByteArray::fromBase64(d->settings.value("sync/domains").toByteArray());
  if (!domains.isEmpty()) {
    QByteArray recovered;
    try {
      recovered = Crypter::decode(d->masterPassword.toUtf8(), domains, CompressionEnabled, d->KGK);
    }
    catch (CryptoPP::Exception &e) {
      wrongPasswordWarning((int)e.GetErrorType(), e.what());
      return false;
    }

    QJsonParseError parseError;
    json = QJsonDocument::fromJson(recovered, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
      domainList = json.object().keys();
      ui->statusBar->showMessage(tr("Password accepted. Restored %1 domains.").arg(domainList.count()), 5000);
    }
    else {
      QMessageBox::warning(this, tr("Bad data from sync server"),
                           tr("Decoding the data from the sync server failed: %1")
                           .arg(parseError.errorString()), QMessageBox::Ok);
    }
  }
  d->domains = DomainSettingsList::fromQJsonDocument(json);
  makeDomainComboBox();
  return true;
}


void MainWindow::saveSettings(void)
{
  Q_D(MainWindow);

  QVariantMap syncData;
  syncData["sync/server/root"] = d->optionsDialog->serverRootUrl();
  syncData["sync/server/username"] = d->optionsDialog->serverUsername();
  syncData["sync/server/password"] = d->optionsDialog->serverPassword();
  syncData["sync/server/rootCertificates"] = QString(d->optionsDialog->serverRootCertificate().toPem());
  syncData["sync/server/secure"] = d->optionsDialog->secure();
  syncData["sync/server/writeUrl"] = d->optionsDialog->writeUrl();
  syncData["sync/server/readUrl"] = d->optionsDialog->readUrl();
  syncData["sync/server/deleteUrl"] = d->optionsDialog->deleteUrl();
  syncData["sync/onStart"] = d->optionsDialog->syncOnStart();
  syncData["sync/filename"] = d->optionsDialog->syncFilename();
  syncData["sync/useFile"] = d->optionsDialog->useSyncFile();
  syncData["sync/useServer"] = d->optionsDialog->useSyncServer();

  d->keyGenerationMutex.lock();
  QByteArray baCryptedData;
  try {
    baCryptedData = Crypter::encode(d->masterKey, d->IV, d->salt, d->KGK, QJsonDocument::fromVariant(syncData).toJson(QJsonDocument::Compact), CompressionEnabled);
  }
  catch (CryptoPP::Exception &e) {
    wrongPasswordWarning((int)e.GetErrorType(), e.what());
    return;
  }
  d->keyGenerationMutex.unlock();

  d->settings.setValue("sync/param", QString::fromUtf8(baCryptedData.toBase64()));

  d->settings.setValue("mainwindow/geometry", saveGeometry());
  d->settings.setValue("misc/masterPasswordInvalidationTimeMins", d->optionsDialog->masterPasswordInvalidationTimeMins());
  d->settings.setValue("misc/maxPasswordLength", d->optionsDialog->maxPasswordLength());
  d->settings.setValue("misc/saltLength", d->optionsDialog->saltLength());
  d->settings.setValue("misc/writeBackups", d->optionsDialog->writeBackups());
  d->settings.setValue("misc/passwordFile", d->optionsDialog->passwordFilename());
#ifdef WIN32
  d->settings.setValue("misc/smartLogin", d->optionsDialog->smartLogin());
#endif

  saveAllDomainDataToSettings();
  d->settings.sync();
}


#if HACKING_MODE_ENABLED
void MainWindow::hackLegacyPassword(void)
{
  Q_D(MainWindow);
  const QString &pwd = ui->legacyPasswordLineEdit->text();
  if (pwd.isEmpty()) {
    QMessageBox::information(this, tr("Cannot hack"), tr("No legacy password given. Cannot hack!"));
  }
  else {
    ui->tabWidget->setCurrentIndex(0);
    blockUpdatePassword();
    d->masterPasswordInvalidationTimer.stop();
    d->hackingMode = true;
    d->hackSalt.fill(0);
    d->hackPos = PositionTable(pwd);
    d->hackPermutations = d->hackPos.permutations();
    d->hackIterationDurationMs = 0;
    const QStringList &chrs = pwd.split("", QString::SkipEmptyParts).toSet().toList(); // keep this for backwards compatibility (Qt < 5.5)
    ui->usedCharactersPlainTextEdit->setPlainText(chrs.join(""));
    ui->legacyPasswordLineEdit->setReadOnly(true);
    ui->usedCharactersPlainTextEdit->setReadOnly(true);
    ui->renewSaltPushButton->setEnabled(false);
    ui->passwordLengthSpinBox->setValue(pwd.size());
    d->hackClock.restart();
    d->hackIterationClock.restart();
    unblockUpdatePassword();
    ui->saltBase64LineEdit->setText(d->hackSalt.toBase64());
  }
}
#endif


bool MainWindow::restoreSettings(void)
{
  Q_D(MainWindow);
  restoreGeometry(d->settings.value("mainwindow/geometry").toByteArray());
  d->optionsDialog->setMasterPasswordInvalidationTimeMins(d->settings.value("misc/masterPasswordInvalidationTimeMins", DefaultMasterPasswordInvalidationTimeMins).toInt());
  d->optionsDialog->setSaltLength(d->settings.value("misc/saltLength", DomainSettings::DefaultSaltLength).toInt());
  d->optionsDialog->setWriteBackups(d->settings.value("misc/writeBackups", false).toBool());
  d->optionsDialog->setPasswordFilename(d->settings.value("misc/passwordFile").toString());
  d->optionsDialog->setMaxPasswordLength(d->settings.value("misc/maxPasswordLength", Password::DefaultMaxLength).toInt());
#ifdef WIN32
  d->optionsDialog->setSmartLogin(d->settings.value("misc/smartLogin").toBool());
#endif
  d->optionsDialog->setSyncFilename(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + AppName + ".bin");
  d->optionsDialog->setServerRootUrl(DefaultSyncServerRoot);
  d->optionsDialog->setServerUsername(DefaultSyncServerUsername);
  d->optionsDialog->setServerPassword(DefaultSyncServerPassword);
  d->optionsDialog->setReadUrl(DefaultSyncServerReadUrl);
  d->optionsDialog->setWriteUrl(DefaultSyncServerWriteUrl);
  d->optionsDialog->setDeleteUrl(DefaultSyncServerDeleteUrl);

  QByteArray baCryptedData = QByteArray::fromBase64(d->settings.value("sync/param").toByteArray());
  if (!baCryptedData.isEmpty()) {
    QByteArray baSyncData;
    try {
      baSyncData = Crypter::decode(d->masterPassword.toUtf8(), baCryptedData, CompressionEnabled, d->KGK);
    }
    catch (CryptoPP::Exception &e) {
      wrongPasswordWarning((int)e.GetErrorType(), e.what());
      return false;
    }

    const QJsonDocument &jsonSyncData = QJsonDocument::fromJson(baSyncData);
    QVariantMap syncData = jsonSyncData.toVariant().toMap();
    d->optionsDialog->setSyncFilename(syncData["sync/filename"].toString());
    d->optionsDialog->setSyncOnStart(syncData["sync/onStart"].toBool());
    d->optionsDialog->setUseSyncFile(syncData["sync/useFile"].toBool());
    d->optionsDialog->setUseSyncServer(syncData["sync/useServer"].toBool());
    d->optionsDialog->setServerRootUrl(syncData["sync/server/root"].toString());
    d->optionsDialog->setWriteUrl(syncData["sync/server/writeUrl"].toString());
    d->optionsDialog->setReadUrl(syncData["sync/server/readUrl"].toString());
    d->optionsDialog->setDeleteUrl(syncData["sync/server/deleteUrl"].toString());
    d->optionsDialog->setServerCertificates(QSslCertificate::fromData(syncData["sync/server/rootCertificates"].toByteArray(), QSsl::Pem));
    d->optionsDialog->setSecure(syncData["sync/server/secure"].toBool());
    d->optionsDialog->setServerUsername(syncData["sync/server/username"].toString());
    d->optionsDialog->setServerPassword(syncData["sync/server/password"].toString());
  }

  return true;
}


void MainWindow::onWriteFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  ++d->counter;
  d->progressDialog->setValue(d->counter);
  if (reply->error() == QNetworkReply::NoError) {
    if (d->masterPasswordChangeStep > 0) {
      nextChangeMasterPasswordStep();
    }
    else {
      if (d->counter == d->maxCounter) {
        d->loaderIcon.stop();
        d->progressDialog->setText(tr("Sync to server finished."));
      }
    }
  }
  else {
    d->progressDialog->setText(tr("Writing to the server failed. Reason: %1").arg(reply->errorString()));
  }
  reply->close();
}


void MainWindow::cancelServerOperation(void)
{
  Q_D(MainWindow);
  if (d->readReply != Q_NULLPTR && d->readReply->isRunning()) {
    d->readReply->abort();
    ui->statusBar->showMessage(tr("Server read operation aborted."), 3000);
  }
  if (d->writeReply != Q_NULLPTR && d->writeReply->isRunning()) {
    d->writeReply->abort();
    ui->statusBar->showMessage(tr("Sync to server aborted."), 3000);
  }
}


void MainWindow::sync(void)
{
  Q_D(MainWindow);
  Q_ASSERT_X(!d->masterPassword.isEmpty(), "MainWindow::sync()", "d->masterPassword must not be empty");
  if (d->optionsDialog->useSyncFile() && !d->optionsDialog->syncFilename().isEmpty()) {
    ui->statusBar->showMessage(tr("Syncing with file ..."));
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
      d->keyGenerationMutex.lock();
      const QByteArray &domains = Crypter::encode(d->masterKey, d->IV, d->salt, d->KGK, QByteArray("{}"), CompressionEnabled);
      d->keyGenerationMutex.unlock();
      syncFile.write(domains);
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
      QByteArray domains = syncFile.readAll();
      syncFile.close();
      syncWith(SyncPeerFile, domains);
    }
    else {
      QMessageBox::warning(this, tr("Sync file read error"),
                           tr("The sync file %1 cannot be opened for reading.")
                           .arg(d->optionsDialog->syncFilename()), QMessageBox::Ok);
    }
  }

  if (syncToServerEnabled()) {
    if (d->masterPasswordChangeStep == 0) {
      d->progressDialog->show();
      d->progressDialog->raise();
      d->counter = 0;
      d->maxCounter = 1;
      d->progressDialog->setRange(0, d->maxCounter);
      d->progressDialog->setValue(d->counter);
    }
    d->progressDialog->setText(tr("Reading from server ..."));
    QNetworkRequest req(QUrl(d->optionsDialog->serverRootUrl() + d->optionsDialog->readUrl()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::UserAgentHeader, AppUserAgent);
    req.setRawHeader("Authorization", d->optionsDialog->httpBasicAuthenticationString());
    req.setSslConfiguration(d->sslConf);
    d->readReply = d->readNAM.post(req, QByteArray());
  }
}


QByteArray MainWindow::cryptedRemoteDomains(void)
{
  Q_D(MainWindow);
  QMutexLocker(&d->keyGenerationMutex);
  QByteArray cipher;
  try {
    cipher = Crypter::encode(d->masterKey, d->IV, d->salt, d->KGK, d->remoteDomains.toJson(), CompressionEnabled);
  }
  catch (CryptoPP::Exception &e) {
    wrongPasswordWarning((int)e.GetErrorType(), e.what());
  }
  return cipher;
}


void MainWindow::syncWith(SyncPeer syncPeer, const QByteArray &remoteDomainsEncoded)
{
  Q_D(MainWindow);
  QJsonDocument remoteJSON;
  if (!remoteDomainsEncoded.isEmpty()) {
    QByteArray baDomains;
    bool ok = true;
    try {
      baDomains = Crypter::decode(d->masterPassword.toUtf8(), remoteDomainsEncoded, CompressionEnabled, d->KGK);
    }
    catch (CryptoPP::Exception &e) {
      ok = false;
      if (d->masterPasswordChangeStep == 0) {
        wrongPasswordWarning((int)e.GetErrorType(), e.what());
        return;
      }
    }
    if (!ok) { // fall back to new password
      try {
        baDomains = Crypter::decode(d->changeMasterPasswordDialog->newPassword().toUtf8(), remoteDomainsEncoded, CompressionEnabled, d->KGK);
      }
      catch (CryptoPP::Exception &e) {
        wrongPasswordWarning((int)e.GetErrorType(), e.what());
        return;
      }
    }
    if (!baDomains.isEmpty()) {
      QJsonParseError parseError;
      remoteJSON = QJsonDocument::fromJson(baDomains, &parseError);
      if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("Bad data from sync peer"),
                             tr("Decoding the data from the sync peer failed: %1")
                             .arg(parseError.errorString()), QMessageBox::Ok);
      }
    }
  }

  d->domains.setDirty(false);
  d->remoteDomains = DomainSettingsList::fromQJsonDocument(remoteJSON);
  mergeLocalAndRemoteData();

  if (d->remoteDomains.isDirty()) {
    writeToRemote(syncPeer);
  }

  if (d->domains.isDirty()) {
    saveAllDomainDataToSettings();
    restoreDomainDataFromSettings();
    d->domains.setDirty(false);
  }
}


void MainWindow::mergeLocalAndRemoteData(void)
{
  Q_D(MainWindow);
  QStringList allDomainNames = d->remoteDomains.keys() + d->domains.keys();
  allDomainNames.removeDuplicates();
  foreach(QString domainName, allDomainNames) {
    const DomainSettings &remoteDomainSetting = d->remoteDomains.at(domainName);
    const DomainSettings &localDomainSetting = d->domains.at(domainName);
    if (!localDomainSetting.isEmpty() && !remoteDomainSetting.isEmpty()) {
      if (remoteDomainSetting.modifiedDate > localDomainSetting.modifiedDate) {
        d->domains.updateWith(remoteDomainSetting);
      }
      else if (remoteDomainSetting.modifiedDate < localDomainSetting.modifiedDate) {
        d->remoteDomains.updateWith(localDomainSetting);
      }
    }
    else if (remoteDomainSetting.isEmpty()) {
      if (!localDomainSetting.deleted)
        d->remoteDomains.updateWith(localDomainSetting);
      else
        d->domains.remove(domainName);
    }
    else {
      d->domains.updateWith(remoteDomainSetting);
    }
  }
}


void MainWindow::writeToRemote(SyncPeer syncPeer)
{
  Q_D(MainWindow);
  const QByteArray &cipher = cryptedRemoteDomains();
  if (!cipher.isEmpty()) {
    if ((syncPeer & SyncPeerFile) == SyncPeerFile && syncToFileEnabled()) {
      writeToSyncFile(cipher);
    }
    if ((syncPeer & SyncPeerServer) == SyncPeerServer && syncToServerEnabled()) {
      sendToSyncServer(cipher);
    }
  }
  else {
    // TODO: catch encryption error
  }
}


bool MainWindow::syncToServerEnabled(void) const {
  return d_ptr->optionsDialog->useSyncServer();
}


bool MainWindow::syncToFileEnabled(void) const {
  return d_ptr->optionsDialog->useSyncFile() && !d_ptr->optionsDialog->syncFilename().isEmpty();
}


void MainWindow::writeToSyncFile(const QByteArray &cipher)
{
  Q_D(MainWindow);
  if (syncToFileEnabled()) {
    QFile syncFile(d->optionsDialog->syncFilename());
    syncFile.open(QIODevice::WriteOnly);
    const qint64 bytesWritten = syncFile.write(cipher);
    syncFile.close();
    if (bytesWritten < 0) {
      QMessageBox::warning(this, tr("Sync file write error"), tr("Writing to your sync file %1 failed: %2")
                           .arg(d->optionsDialog->syncFilename())
                           .arg(syncFile.errorString()), QMessageBox::Ok);
    }
  }
}


void MainWindow::sendToSyncServer(const QByteArray &cipher)
{
  Q_D(MainWindow);
  if (d->masterPasswordChangeStep == 0) {
    d->counter = 0;
    d->maxCounter = 1;
    d->progressDialog->setText(tr("Sending data to server ..."));
    d->progressDialog->setRange(0, d->maxCounter);
    d->progressDialog->setValue(0);
    d->progressDialog->show();
  }
  QUrlQuery params;
  // XXX: Wouldn't QByteArray::Base64UrlEncoding be better?
  params.addQueryItem("data", cipher.toBase64(QByteArray::Base64Encoding));
  const QByteArray &data = params.query().toUtf8();
  QNetworkRequest req(QUrl(d->optionsDialog->serverRootUrl() + d->optionsDialog->writeUrl()));
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  req.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
  req.setHeader(QNetworkRequest::UserAgentHeader, AppUserAgent);
  req.setRawHeader("Authorization", d->optionsDialog->httpBasicAuthenticationString());
  req.setSslConfiguration(d->sslConf);
  d->writeReply = d->writeNAM.post(req, data);
}


void MainWindow::onDomainSelected(QString domain)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::onDomainSelected(" << domain << ")" << "d->lastDomain =" << d->lastDomain << " SENDER: " << (sender() != Q_NULLPTR ? sender()->objectName() : "NONE");
  if (!domainComboboxContains(domain))
    return;
  QMessageBox::StandardButton button = checkSaveOnDirty();
  switch (button) {
  case QMessageBox::Cancel:
    ui->domainsComboBox->blockSignals(true);
    ui->domainsComboBox->setCurrentText(d->lastDomain);
    ui->domainsComboBox->blockSignals(false);
    return;
    break;
  case QMessageBox::Discard:
    break;
  case QMessageBox::Save:
    ui->domainsComboBox->blockSignals(true);
    ui->domainsComboBox->setCurrentText(d->lastDomain);
    ui->domainsComboBox->blockSignals(false);
    saveCurrentDomainSettings();
    domain = d->lastDomain;
    break;
  default:
    break;
  }
  ui->generatedPasswordLineEdit->setEchoMode(QLineEdit::Password);
  copyDomainSettingsToGUI(domain);
  d->lastDomainSettings = collectedDomainSettings();
  setDirty(false);
  if (d->domains.at(domain).legacyPassword.isEmpty()) {
    ui->tabWidget->setCurrentIndex(0);
    ui->actionHackLegacyPassword->setEnabled(false);
    if (d->domains.at(domain).passwordTemplate.isEmpty()) {
      ui->tabWidgetVersions->setCurrentIndex(0);
    }
    else {
      ui->tabWidgetVersions->setCurrentIndex(1);
    }
  }
  else {
    ui->tabWidget->setCurrentIndex(1);
    ui->actionHackLegacyPassword->setEnabled(true);
  }
  d->currentDomain = ui->domainsComboBox->currentText();
}


void MainWindow::onDomainTextChanged(const QString &domain)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::onDomainTextChanged(" << domain << ")" << d->lastDomainSettings.domainName;
  int idx = findDomainInComboBox(domain);
  if (idx == NotFound) {
    if (!d->lastDomainSettings.isEmpty()) {
      resetAllFieldsExceptDomainComboBox();
      ui->tabWidgetVersions->setCurrentIndex(1);
      ui->tabWidget->setCurrentIndex(0);
    }
    ui->generatedPasswordLineEdit->setEchoMode(QLineEdit::Normal);
    d->lastDomainSettings.clear();
    d->lastDomain = domain;
    createTemplate_v3();
    updatePassword();
  }
}


void MainWindow::onEasySelectorValuesChanged(int length, int complexity)
{
  Q_D(MainWindow);
  Q_UNUSED(length);
  const QBitArray &ba = Password::deconstructedComplexity(complexity);
  ui->useDigitsCheckBox->setChecked(ba.at(0));
  ui->useLowerCaseCheckBox->setChecked(ba.at(1));
  ui->useUpperCaseCheckBox->setChecked(ba.at(2));
  ui->useExtraCheckBox->setChecked(ba.at(3) && !ui->extraLineEdit->text().isEmpty());
  createTemplate_v3();
  d->password.setDomainSettings(collectedDomainSettings());
  ui->generatedPasswordLineEdit->setText(d->password.remixed());
  setDirty();
  restartInvalidationTimer();
}


void MainWindow::onPasswordTemplateChanged(const QString &templ)
{
  Q_D(MainWindow);
//  qDebug() << "MainWindow::onPasswordTemplateChanged(" << templ << ")";
  analyzeTemplate_v3(templ.toUtf8());
}


void MainWindow::onEscPressed(void)
{
  Q_D(MainWindow);
  if (d->parameterSetDirty) {
    QMessageBox::StandardButton button = saveOrDiscard();
    switch (button) {
    case QMessageBox::Save:
      saveCurrentDomainSettings();
      break;
    default:
      break;
    }
  }
  else if (!ui->domainsComboBox->currentText().isEmpty()) {
    resetAllFields();
  }
}


void MainWindow::updateWindowTitle(void)
{
  Q_D(MainWindow);
  bool dirty = d->parameterSetDirty && !ui->domainsComboBox->currentText().isEmpty();
  setWindowTitle(QString("%1 %2%3 (%4)%5")
                 .arg(AppName)
                 .arg(AppVersion)
                 .arg(dirty ? "*" : "")
#if PLATFORM == 64
                 .arg("x64")
#else
                 .arg("x86")
#endif
                 .arg(isPortable() ? " - PORTABLE " : "")
                 );
}


void MainWindow::clearClipboard(void)
{
  QApplication::clipboard()->clear();
  ui->statusBar->showMessage(tr("Clipboard cleared."), 3000);
}


void MainWindow::enterMasterPassword(void)
{
  Q_D(MainWindow);
  hide();
  d->optionsDialog->hide();
  d->masterPasswordDialog->setRepeatPassword(d->settings.value("mainwindow/masterPasswordEntered", false).toBool() == false);
  d->masterPasswordDialog->show();
  d->masterPasswordDialog->raise();
}


void MainWindow::onMasterPasswordEntered(void)
{
  Q_D(MainWindow);
  bool ok = true;
  QString masterPwd = d->masterPasswordDialog->masterPassword();
  if (!masterPwd.isEmpty()) {
    d->masterPassword = masterPwd;
    if (d->KGK.isEmpty()) {
      d->KGK = Crypter::generateKGK();
    }
    ok = restoreSettings();
    if (ok) {
      ok = restoreDomainDataFromSettings();
      if (ok) {
        generateSaltKeyIV().waitForFinished();
        d->settings.setValue("mainwindow/masterPasswordEntered", true);
        d->settings.sync();
        ui->domainsComboBox->setCurrentText(d->lastDomainBeforeLock);
        ui->domainsComboBox->setFocus();
        d->masterPasswordDialog->hide();
        show();
        if (d->optionsDialog->syncOnStart())
          sync();
        restartInvalidationTimer();
      }
    }
  }
  if (!ok ) {
    enterMasterPassword();
  }
}


void MainWindow::clearAllSettings(void)
{
  Q_D(MainWindow);
  int button = QMessageBox::warning(
        this,
        tr("%1 - Really clear all settings?").arg(AppName),
        tr("You have chosen to delete all of your settings, "
           "i.e. your application settings and all of your domain settings. "
           "After deletion you'll have to start from scratch. "
           "Do you really want to do that?"), QMessageBox::Yes, QMessageBox::No);
  if (button != QMessageBox::Yes)
    return;

  resetAllFields();
  d->masterPasswordDialog->setRepeatPassword(true);
  ui->domainsComboBox->clear();
  d->settings.setValue("mainwindow/masterPasswordEntered", false);
  d->settings.remove("sync");
  d->settings.sync();
  if (d->optionsDialog->useSyncFile() && !d->optionsDialog->syncFilename().isEmpty()) {
    QFileInfo fi(d->optionsDialog->syncFilename());
    if (fi.isWritable())
      QFile(d->optionsDialog->syncFilename()).remove();
  }
  if (d->optionsDialog->useSyncServer() && !d->optionsDialog->deleteUrl().isEmpty()) {
    QNetworkRequest req(QUrl(d->optionsDialog->serverRootUrl() + d->optionsDialog->deleteUrl()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::UserAgentHeader, AppUserAgent);
    req.setRawHeader("Authorization", d->optionsDialog->httpBasicAuthenticationString());
    req.setSslConfiguration(d->sslConf);
    d->deleteReply = d->deleteNAM.post(req, QByteArray());
  }
  d->lastDomainBeforeLock.clear();
  invalidatePassword(true);
}


void MainWindow::wrongPasswordWarning(int errCode, QString errMsg)
{
  QMessageBox::critical(
        this,
        tr("%1 - Decryption error").arg(AppName),
        tr("An error occured while decrypting your data (#%1, %2). "
           "Maybe you entered a wrong password. "
           "Please enter the correct password!").arg(errCode).arg(errMsg),
        QMessageBox::Retry);
  enterMasterPassword();
}


void MainWindow::invalidatePassword(bool reenter)
{
  Q_D(MainWindow);
  SecureErase(d->masterPassword);
  d->masterPasswordDialog->invalidatePassword();
  ui->statusBar->showMessage(tr("Master password cleared for security"));
  if (reenter)
    enterMasterPassword();
}


void MainWindow::lockApplication(void)
{
  Q_D(MainWindow);
  d->lastDomainBeforeLock = ui->domainsComboBox->currentText();
  saveSettings();
  invalidatePassword(true);
}


void MainWindow::sslErrorsOccured(QNetworkReply *reply, const QList<QSslError> &errors)
{
  foreach (QSslError error, errors)
    qWarning() << "SSL error occured: " << int(error.error()) << error.errorString();
}


void MainWindow::onDeleteFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  if (reply->error() == QNetworkReply::NoError) {
    const QByteArray &res = reply->readAll();
    QJsonParseError parseError;
    const QJsonDocument &json = QJsonDocument::fromJson(res, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
      QVariantMap map = json.toVariant().toMap();
      if (map["status"].toString() == "ok") {
        QMessageBox::information(
              this,
              tr("Deletion on server finished"),
              tr("Your domain settings have been successfully deleted on the sync server"));
      }
      else {
        QMessageBox::warning(
              this,
              tr("Deletion on server failed"),
              tr("The deletion of your domain settings on the server failed: %1").arg(map["error"].toString()));
      }
    }
  }
  else {
    QMessageBox::warning(
          this,
          tr("Deletion on server failed"),
          tr("The deletion of your domain settings on the server failed: %1").arg(reply->errorString()));
  }
  reply->close();
}


void MainWindow::onReadFinished(QNetworkReply *reply)
{
  Q_D(MainWindow);
  d->loaderIcon.stop();
  ++d->counter;
  d->progressDialog->setValue(d->counter);

  if (reply->error() == QNetworkReply::NoError) {
    const QByteArray &res = reply->readAll();
    d->progressDialog->setText(tr("Reading from server finished."));
    QJsonParseError parseError;
    const QJsonDocument &json = QJsonDocument::fromJson(res, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
      QVariantMap map = json.toVariant().toMap();
      if (map["status"].toString() == "ok") {
        QByteArray baDomains = QByteArray::fromBase64(map["result"].toByteArray());
        syncWith(SyncPeerServer, baDomains);
      }
      else {
        d->progressDialog->setText(tr("Reading from the sync server failed. Status: %1 - Error: %2").arg(map["status"].toString()).arg(map["error"].toString()));
      }
      if (d->masterPasswordChangeStep > 0)
        nextChangeMasterPasswordStep();
    }
    else {
      d->progressDialog->setText(tr("Decoding the data from the sync server failed: %1").arg(parseError.errorString()));
    }
  }
  else {
    d->progressDialog->setText(tr("Critical Network Error: %1").arg(reply->errorString()));
  }
  reply->close();
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
           "<p>Copyright &copy; 2015 %3 &lt;%4&gt;, Heise Medien GmbH &amp; Co. KG.</p>"
           "<p>This program uses the Crypto++ library. Crypto++ is licensed under the Boost Software License, Version 1.0.</p>"
           )
        .arg(AppName).arg(AppURL).arg(AppAuthor).arg(AppAuthorMail));
}


void MainWindow::aboutQt(void)
{
  QMessageBox::aboutQt(this);
}


void MainWindow::createFullDump(void)
{
#if defined(QT_DEBUG)
#if defined(Q_CC_MSVC)
  make_minidump();
  ui->statusBar->showMessage(tr("Dump created."), 4000);
  qDebug() << "Mini dump created.";
#else
  ui->statusBar->showMessage(tr("Dump not implemented."), 4000);
  qDebug() << "Dump not implemented.";
#endif
#endif
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
  switch (event->type()) {
  case QEvent::Enter:
    if (obj->objectName() == "generatedPasswordLineEdit" && !ui->generatedPasswordLineEdit->text().isEmpty())
      ui->generatedPasswordLineEdit->setCursor(Qt::WhatsThisCursor);
    else if (obj->objectName() == "legacyPasswordLineEdit" && !ui->legacyPasswordLineEdit->text().isEmpty())
      ui->legacyPasswordLineEdit->setCursor(Qt::WhatsThisCursor);
    break;
  case QEvent::Leave:
    if (obj->objectName() == "generatedPasswordLineEdit")
      ui->generatedPasswordLineEdit->setCursor(Qt::ArrowCursor);
    else if (obj->objectName() == "legacyPasswordLineEdit")
      ui->legacyPasswordLineEdit->setCursor(Qt::ArrowCursor);
    break;
  case QEvent::MouseButtonPress:
    if (domainComboboxContains(ui->domainsComboBox->currentText())) {
      if (obj->objectName() == "generatedPasswordLineEdit")
        ui->generatedPasswordLineEdit->setEchoMode(QLineEdit::Normal);
      else if (obj->objectName() == "legacyPasswordLineEdit")
        ui->legacyPasswordLineEdit->setEchoMode(QLineEdit::Normal);
    }
    break;
  case QEvent::MouseButtonRelease:
    if (domainComboboxContains(ui->domainsComboBox->currentText())) {
      if (obj->objectName() == "generatedPasswordLineEdit")
        ui->generatedPasswordLineEdit->setEchoMode(QLineEdit::Password);
      else if (obj->objectName() == "legacyPasswordLineEdit")
        ui->legacyPasswordLineEdit->setEchoMode(QLineEdit::Password);
    }
    break;
  default:
    break;
  }
  return QObject::eventFilter(obj, event);
}
