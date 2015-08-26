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

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSslSocket>
#include <QSslError>
#include "servercertificatewidget.h"

class OptionsDialogPrivate
{
public:
  OptionsDialogPrivate(void)
    : serverCertificateWidget(new ServerCertificateWidget)
  { /* ... */ }
  QSslSocket sslSocket;
  QList<QSslError> sslErrors;
  QList<QSslError> ignoredSslErrors;
  QList<QSslCertificate> serverCertificates;
  ServerCertificateWidget *serverCertificateWidget;
  QSslCertificate selfSignedCertificate;
};


OptionsDialog::OptionsDialog(QWidget *parent)
  : QDialog(parent, Qt::Widget)
  , ui(new Ui::OptionsDialog)
  , d_ptr(new OptionsDialogPrivate)
{
  Q_D(OptionsDialog);
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->chooseSyncFilePushButton, SIGNAL(pressed()), SLOT(chooseSyncFile()));
  QObject::connect(&d->sslSocket, SIGNAL(encrypted()), SLOT(onEncrypted()));
  QObject::connect(&d->sslSocket, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrorsOccured(QList<QSslError>)));
  QObject::connect(ui->importServerCertificatePushButton, SIGNAL(pressed()), SLOT(verifySecureConnection()));
}


OptionsDialog::~OptionsDialog()
{
  delete ui;
}


void OptionsDialog::onEncrypted(void)
{
  Q_D(OptionsDialog);
  qDebug() << "OptionsDialog::onEncrypted()";
  d->serverCertificateWidget->setServerSocket(d->sslSocket);
  int button = d->serverCertificateWidget->exec();
  if (button == QDialog::Accepted) {
    d->ignoredSslErrors = d->sslErrors;
    setServerCertificates(d->sslSocket.peerCertificateChain());
  }
  d->sslSocket.close();
}


void OptionsDialog::verifySecureConnection(void)
{
  Q_D(OptionsDialog);
  d->sslErrors.clear();
  d->ignoredSslErrors.clear();
  d->serverCertificates.clear();
  QUrl serverUrl(ui->serverRootURLLineEdit->text());
  if (serverUrl.scheme() == "https") {
    static const int HttpsPort = 443;
    qDebug() << "Trying to connect to" << serverUrl.host() << ":" << HttpsPort << "...";
    d->sslSocket.connectToHostEncrypted(serverUrl.host(), HttpsPort);
  }
}


void OptionsDialog::sslErrorsOccured(const QList<QSslError> &errors)
{
  Q_D(OptionsDialog);
  d->sslErrors = errors;
  qDebug() << "OptionsDialog::sslErrorsOccured()";
  foreach (QSslError err, d->sslErrors) {
    qDebug() << err.certificate() << err.errorString();
  }
  d->sslSocket.ignoreSslErrors();
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


QByteArray OptionsDialog::serverCredentials(void) const
{
  return QString("Basic %1")
      .arg(QString((ui->usernameLineEdit->text() + ":" + ui->passwordLineEdit->text())
                   .toLocal8Bit().toBase64()))
      .toLocal8Bit();
}


int OptionsDialog::masterPasswordInvalidationTimeMins(void) const
{
  return ui->masterPasswordInvalidationTimeMinsSpinBox->value();
}


void OptionsDialog::setMasterPasswordInvalidationTimeMins(int minutes)
{
  ui->masterPasswordInvalidationTimeMinsSpinBox->setValue(minutes);
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
  ui->serverRootURLLineEdit->setText(url);
}


void OptionsDialog::setServerUsername(QString username)
{
  ui->usernameLineEdit->setText(username);
}


void OptionsDialog::setServerPassword(QString password)
{
  ui->passwordLineEdit->setText(password);
}


const QList<QSslCertificate> &OptionsDialog::serverCertificates(void) const
{
  return d_ptr->serverCertificates;
}


void OptionsDialog::setServerCertificates(const QList<QSslCertificate> &certChain)
{
  d_ptr->serverCertificates = certChain;
  emit updatedServerCertificates();
}


const QSslCertificate &OptionsDialog::selfSignedCertificate(void) const
{
  return d_ptr->selfSignedCertificate;
}


QSslCertificate OptionsDialog::serverCertificate(void) const
{
  return d_ptr->serverCertificates.isEmpty()
      ? QSslCertificate()
      : d_ptr->serverCertificates.last(); // XXX
}


const QList<QSslError> &OptionsDialog::sslErrors(void) const
{
  return d_ptr->sslErrors;
}


const QList<QSslError> &OptionsDialog::ignoredSslErrors(void) const
{
  return d_ptr->ignoredSslErrors;
}


void OptionsDialog::setWriteUrl(QString url)
{
  ui->writeUrlLineEdit->setText(url);
}


void OptionsDialog::setReadUrl(QString url)
{
  ui->readUrlLineEdit->setText(url);
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
  QFileInfo fi(ui->syncFileLineEdit->text());
  QString chosenFile = QFileDialog::getSaveFileName(this, tr("Choose sync file"), fi.absolutePath());
  if (!chosenFile.isEmpty())
    ui->syncFileLineEdit->setText(chosenFile);
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
