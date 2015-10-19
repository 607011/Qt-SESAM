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

#include "util.h"
#include "global.h"
#include "servercertificatewidget.h"
#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include <QDebug>
#include <QObject>
#include <QFileDialog>
#include <QFileInfo>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslSocket>
#include <QSslCipher>
#include <QSslError>
#include <QMessageBox>
#include <QCheckBox>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QMovie>
#include <QShortcut>

static const QString HTTPS = "https";

class OptionsDialogPrivate
{
public:
  OptionsDialogPrivate(void)
    : sslConf(QSslConfiguration::defaultConfiguration())
    , reply(Q_NULLPTR)
    , secure(false)
    , loaderIcon(":/images/loader.gif")
    , escShortcut(Q_NULLPTR)
#ifdef WIN32
    , smartLoginCheckbox(Q_NULLPTR)
#endif
  {
    sslConf.setCiphers(QSslSocket::supportedCiphers());
  }
  ~OptionsDialogPrivate()
  {
      SafeDelete(escShortcut);
  }
  QNetworkAccessManager NAM;
  QSslConfiguration sslConf;
  QNetworkReply *reply;
  QList<QSslError> sslErrors;
  QList<QSslCertificate> serverCertificates;
  ServerCertificateWidget serverCertificateWidget;
  bool secure;
  QMovie loaderIcon;
  QShortcut *escShortcut;
#ifdef WIN32
  QCheckBox *smartLoginCheckbox;
#endif
};


OptionsDialog::OptionsDialog(QWidget *parent)
  : QDialog(parent, Qt::Widget)
  , ui(new Ui::OptionsDialog)
  , d_ptr(new OptionsDialogPrivate)
{
  Q_D(OptionsDialog);
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctSESAM.ico"));
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->chooseSyncFilePushButton, SIGNAL(pressed()), SLOT(chooseSyncFile()));
  QObject::connect(ui->checkConnectivityPushButton, SIGNAL(pressed()), SLOT(checkConnectivity()));
  QObject::connect(ui->selectPasswordFilePushButton, SIGNAL(pressed()), SLOT(choosePasswordFile()));
  QObject::connect(ui->serverRootURLLineEdit, SIGNAL(textChanged(QString)), SLOT(onServerRootUrlChanged(QString)));
  QObject::connect(ui->saltLengthSpinBox, SIGNAL(valueChanged(int)), SIGNAL(saltLengthChanged(int)));
  QObject::connect(ui->maxPasswordLengthSpinBox, SIGNAL(valueChanged(int)), SIGNAL(maxPasswordLengthChanged(int)));
  QObject::connect(&d->NAM, SIGNAL(finished(QNetworkReply*)), SLOT(onReadFinished(QNetworkReply*)));
  QObject::connect(&d->NAM, SIGNAL(encrypted(QNetworkReply*)), SLOT(onEncrypted(QNetworkReply*)));
  QObject::connect(&d->NAM, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrorsOccured(QNetworkReply*,QList<QSslError>)));
  d->escShortcut = new QShortcut(Qt::Key_Escape, this, SLOT(reject()));

#ifdef WIN32
  d->smartLoginCheckbox = new QCheckBox(tr("Smart login"));
  d->smartLoginCheckbox->setChecked(true);
  ui->miscFormLayout->addWidget(d->smartLoginCheckbox);
#endif
}


OptionsDialog::~OptionsDialog()
{
  delete ui;
}


void OptionsDialog::checkConnectivity(void)
{
  Q_D(OptionsDialog);
  QUrl serverUrl(ui->serverRootURLLineEdit->text());
  if (serverUrl.scheme() == HTTPS) {
    setSecure(false);
    ui->accessibleLabel->setMovie(&d->loaderIcon);
    d->loaderIcon.start();
    d->sslErrors.clear();
    d->NAM.clearAccessCache();
    d->serverCertificates.clear();
    QNetworkRequest req(QUrl(ui->serverRootURLLineEdit->text() + ui->readUrlLineEdit->text()));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::UserAgentHeader, AppUserAgent);
    req.setRawHeader("Authorization", httpBasicAuthenticationString());
    req.setSslConfiguration(d->sslConf);
    d->reply = d->NAM.post(req, QByteArray());
  }
  else {
    QMessageBox::information(this, tr("Wrong protocol"), tr("The given protocol \"%1\" is not valid. Please enter a URL beginning with \"https://\".").arg(serverUrl.scheme()));
  }
}


void OptionsDialog::validateHostCertificateChain(void)
{
  Q_D(OptionsDialog);
  bool ok = (d->sslErrors.count() == 0) || d->sslErrors.at(0) == QSslError::NoError;
  if (!ok) {
    d->serverCertificateWidget.setServerSslErrors(d->reply->sslConfiguration(), d->sslErrors);
    if (d->serverCertificateWidget.exec() == QDialog::Accepted)
      setServerCertificates(d->reply->sslConfiguration().peerCertificateChain());
  }
  else {
    setServerCertificates(QList<QSslCertificate>());
  }
}


void OptionsDialog::onEncrypted(QNetworkReply*)
{
  validateHostCertificateChain();
}


void OptionsDialog::onReadFinished(QNetworkReply *reply)
{
  Q_D(OptionsDialog);
  QString warning;
  QJsonParseError parseError;
  QByteArray data = reply->readAll();
  QJsonDocument jDoc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error == QJsonParseError::NoError) {
    if (jDoc.isObject()) {
      QVariantMap map = jDoc.toVariant().toMap();
      if (map["status"].toString() == "ok") {
        setSecure(true);
        ui->accessibleLabel->setPixmap(QPixmap(":/images/check.png"));
        ui->accessibleLabel->setToolTip(tr("Connection succeeded."));
      }
      else {
        warning = tr("JSON data contains bad status: %1").arg(map["status"].toString());
      }
    }
    else {
      warning = tr("reply is not a JSON object");
    }
  }
  else {
    warning = tr("reply cannot be parsed as JSON data: %1 (data: %2)").arg(parseError.errorString()).arg(QString::fromUtf8(data));
  }
  if (!warning.isEmpty()) {
    ui->accessibleLabel->setPixmap(QPixmap(":/images/warning.png"));
    ui->accessibleLabel->setToolTip(warning);
  }
  d->loaderIcon.stop();
}


const QList<QSslCertificate> &OptionsDialog::serverCertificates(void) const
{
  return d_ptr->serverCertificates;
}


void OptionsDialog::setServerCertificates(const QList<QSslCertificate> &certChain)
{
  Q_D(OptionsDialog);
  d->serverCertificates = certChain;
  setSecure(!serverRootCertificate().isNull());
  emit serverCertificatesUpdated(certChain);
}


QSslCertificate OptionsDialog::serverRootCertificate(void) const
{
  return d_ptr->serverCertificates.isEmpty()
      ? QSslCertificate()
      : d_ptr->serverCertificates.last(); // XXX
}


void OptionsDialog::sslErrorsOccured(QNetworkReply *reply, const QList<QSslError> &errors)
{
  Q_D(OptionsDialog);
  d->sslErrors = errors;
  reply->ignoreSslErrors();
}


void OptionsDialog::setSecure(bool secure)
{
  Q_D(OptionsDialog);
  d->secure = secure;
  if (secure) {
    ui->encryptedLabel->setPixmap(QPixmap(":/images/encrypted.png"));
    ui->encryptedLabel->setToolTip(tr("Encrypted"));
  }
  else {
    ui->encryptedLabel->setPixmap(QPixmap());
    ui->encryptedLabel->setToolTip(QString());
  }
}


bool OptionsDialog::secure(void) const
{
  return d_ptr->secure;
}


void OptionsDialog::onServerRootUrlChanged(QString)
{
  ui->accessibleLabel->setPixmap(QPixmap());
  ui->accessibleLabel->setToolTip(QString());
  setSecure(false);
}


bool OptionsDialog::syncOnStart(void) const
{
  return ui->syncOnStartCheckBox->isChecked();
}


void OptionsDialog::setSyncOnStart(bool doSync)
{
  ui->syncOnStartCheckBox->setChecked(doSync);
}


QString OptionsDialog::syncFilename(void) const
{
  return ui->syncFileLineEdit->text();
}


void OptionsDialog::setSyncFilename(const QString &syncFilename)
{
  ui->syncFileLineEdit->setText(syncFilename);
}


bool OptionsDialog::useSyncServer(void) const
{
  return ui->useSyncServerCheckBox->isChecked();
}


bool OptionsDialog::useSyncFile(void) const
{
  return ui->useSyncFileCheckBox->isChecked();
}


QString OptionsDialog::serverRootUrl(void) const
{
  return ui->serverRootURLLineEdit->text();
}


QString OptionsDialog::writeUrl(void) const
{
  return ui->writeUrlLineEdit->text();
}


QString OptionsDialog::readUrl(void) const
{
  return ui->readUrlLineEdit->text();
}


QString OptionsDialog::deleteUrl(void) const
{
  return ui->deleteUrlLineEdit->text();
}


QByteArray OptionsDialog::httpBasicAuthenticationString(void) const
{
  return QString("Basic %1")
      .arg(QString((ui->usernameLineEdit->text() + ":" + ui->passwordLineEdit->text())
                   .toLocal8Bit().toBase64()))
      .toLocal8Bit();
}


void OptionsDialog::setWriteBackups(bool enabled)
{
  ui->writeBackupsCheckBox->setChecked(enabled);
}


bool OptionsDialog::writeBackups(void) const
{
  return ui->writeBackupsCheckBox->isChecked();
}


#ifdef WIN32
void OptionsDialog::setSmartLogin(bool enabled)
{
   d_ptr->smartLoginCheckbox->setChecked(enabled);
}


bool OptionsDialog::smartLogin(void) const
{
  return d_ptr->smartLoginCheckbox->isChecked();
}
#endif


int OptionsDialog::masterPasswordInvalidationTimeMins(void) const
{
  return ui->masterPasswordInvalidationTimeMinsSpinBox->value();
}


void OptionsDialog::setMasterPasswordInvalidationTimeMins(int minutes)
{
  ui->masterPasswordInvalidationTimeMinsSpinBox->setValue(minutes);
}


QString OptionsDialog::passwordFilename(void) const
{
  return ui->passwordFileLineEdit->text();
}


void OptionsDialog::setPasswordFilename(const QString &filename)
{
  ui->passwordFileLineEdit->setText(filename);
}


int OptionsDialog::maxPasswordLength(void) const
{
  return ui->maxPasswordLengthSpinBox->value();
}


void OptionsDialog::setMaxPasswordLength(int len)
{
  ui->maxPasswordLengthSpinBox->setValue(len);
}


void OptionsDialog::setUseSyncServer(bool enabled)
{
  ui->useSyncServerCheckBox->setChecked(enabled);
}


void OptionsDialog::setUseSyncFile(bool enabled)
{
  ui->useSyncFileCheckBox->setChecked(enabled);
}


QString OptionsDialog::serverUsername(void) const
{
  return ui->usernameLineEdit->text();
}


QString OptionsDialog::serverPassword(void) const
{
  return ui->passwordLineEdit->text();
}


void OptionsDialog::setServerRootUrl(QString url)
{
  ui->serverRootURLLineEdit->blockSignals(true);
  ui->serverRootURLLineEdit->setText(url);
  ui->serverRootURLLineEdit->blockSignals(false);
}


void OptionsDialog::setServerUsername(QString username)
{
  ui->usernameLineEdit->setText(username);
}


void OptionsDialog::setServerPassword(QString password)
{
  ui->passwordLineEdit->setText(password);
}


void OptionsDialog::setWriteUrl(QString url)
{
  ui->writeUrlLineEdit->setText(url);
}


void OptionsDialog::setReadUrl(QString url)
{
  ui->readUrlLineEdit->setText(url);
}


void OptionsDialog::setDeleteUrl(QString url)
{
  ui->deleteUrlLineEdit->setText(url);
}


int OptionsDialog::saltLength(void) const
{
  return ui->saltLengthSpinBox->value();
}


void OptionsDialog::setSaltLength(int n)
{
  ui->saltLengthSpinBox->setValue(n);
}


void OptionsDialog::chooseSyncFile(void)
{
  const QString &currentFile = ui->syncFileLineEdit->text();
  const QString &savePath = currentFile.isEmpty() ? QString() : QFileInfo(currentFile).absolutePath();
  QString chosenFile = QFileDialog::getSaveFileName(this, tr("Choose sync file"), savePath);
  if (!chosenFile.isEmpty())
    ui->syncFileLineEdit->setText(chosenFile);
}


void OptionsDialog::choosePasswordFile()
{
  const QString &currentFile = ui->passwordFileLineEdit->text();
  const QString &openPath = currentFile.isEmpty() ? QString() : QFileInfo(currentFile).absolutePath();
  QString chosenFile = QFileDialog::getOpenFileName(this, tr("Choose password file"), openPath);
  if (!chosenFile.isEmpty())
    ui->passwordFileLineEdit->setText(chosenFile);
}


void OptionsDialog::okClicked(void)
{
  bool ok = true;
  if (!ui->syncFileLineEdit->text().isEmpty()) {
    QFileInfo fi(ui->syncFileLineEdit->text());
    ok = fi.exists();
  }
  if (ok)
    accept();
  else
    reject();
}
