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
#include <QSslCipher>
#include <QSslCertificate>
#include <QSslSocket>
#include <QSslError>
#include <QSslKey>
#include <QTableWidget>

class OptionsDialogPrivate
{
public:
  OptionsDialogPrivate(void)
    : infoTable(new QTableWidget)
  { /* ... */ }
  QSslSocket sslSocket;
  QList<QSslCertificate> serverCertificates;
  QTableWidget *infoTable;
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
  QObject::connect(ui->verifySecureConnectionPushButton, SIGNAL(pressed()), SLOT(verifySecureConnection()));
}


OptionsDialog::~OptionsDialog()
{
  delete ui;
}


void OptionsDialog::onEncrypted(void)
{
  Q_D(OptionsDialog);
  const QSslCipher &cipher = d->sslSocket.sessionCipher();
  d->infoTable->setColumnCount(2);
  d->infoTable->setRowCount(7);
  d->infoTable->setItem(0, 0, new QTableWidgetItem(tr("Authentication")));
  d->infoTable->setItem(1, 0, new QTableWidgetItem(tr("Encryption")));
  d->infoTable->setItem(2, 0, new QTableWidgetItem(tr("Key Exchange Method")));
  d->infoTable->setItem(3, 0, new QTableWidgetItem(tr("Cipher Name")));
  d->infoTable->setItem(4, 0, new QTableWidgetItem(tr("Protocol")));
  d->infoTable->setItem(5, 0, new QTableWidgetItem(tr("Supported Bits")));
  d->infoTable->setItem(6, 0, new QTableWidgetItem(tr("Used Bits")));
  d->infoTable->setItem(0, 1, new QTableWidgetItem(cipher.authenticationMethod()));
  d->infoTable->setItem(1, 1, new QTableWidgetItem(cipher.encryptionMethod()));
  d->infoTable->setItem(2, 1, new QTableWidgetItem(cipher.keyExchangeMethod()));
  d->infoTable->setItem(3, 1, new QTableWidgetItem(cipher.name()));
  d->infoTable->setItem(4, 1, new QTableWidgetItem(cipher.protocolString()));
  d->infoTable->setItem(5, 1, new QTableWidgetItem(cipher.supportedBits()));
  d->infoTable->setItem(6, 1, new QTableWidgetItem(cipher.usedBits()));

  foreach (QSslCertificate cert, d->sslSocket.peerCertificateChain()) {
    qDebug() << cert;
    int rows = d->infoTable->rowCount();
    d->infoTable->setRowCount(rows + 16);
    d->infoTable->setItem(rows +  0, 0, new QTableWidgetItem(tr("Serial number")));
    d->infoTable->setItem(rows +  1, 0, new QTableWidgetItem(tr("Effective Date")));
    d->infoTable->setItem(rows +  2, 0, new QTableWidgetItem(tr("Expiry Date")));
    d->infoTable->setItem(rows +  3, 0, new QTableWidgetItem(tr("CommonName")));
    d->infoTable->setItem(rows +  4, 0, new QTableWidgetItem(tr("Organization")));
    d->infoTable->setItem(rows +  5, 0, new QTableWidgetItem(tr("LocalityName")));
    d->infoTable->setItem(rows +  6, 0, new QTableWidgetItem(tr("OrganizationalUnitName")));
    d->infoTable->setItem(rows +  7, 0, new QTableWidgetItem(tr("StateOrProvinceName")));
    d->infoTable->setItem(rows +  8, 0, new QTableWidgetItem(tr("CommonName")));
    d->infoTable->setItem(rows +  9, 0, new QTableWidgetItem(tr("Organization")));
    d->infoTable->setItem(rows + 10, 0, new QTableWidgetItem(tr("LocalityName")));
    d->infoTable->setItem(rows + 11, 0, new QTableWidgetItem(tr("OrganizationalUnitName")));
    d->infoTable->setItem(rows + 12, 0, new QTableWidgetItem(tr("StateOrProvinceName")));
    d->infoTable->setItem(rows + 13, 0, new QTableWidgetItem(tr("E-Mail Address")));
    d->infoTable->setItem(rows + 14, 0, new QTableWidgetItem(tr("Version")));
    d->infoTable->setItem(rows +  0, 1, new QTableWidgetItem(QString(cert.serialNumber())));
    d->infoTable->setItem(rows +  1, 1, new QTableWidgetItem(cert.effectiveDate().toString()));
    d->infoTable->setItem(rows +  2, 1, new QTableWidgetItem(cert.expiryDate().toString()));
    d->infoTable->setItem(rows +  3, 1, new QTableWidgetItem(cert.subjectInfo(QSslCertificate::CommonName).join(", ")));
    d->infoTable->setItem(rows +  4, 1, new QTableWidgetItem(cert.subjectInfo(QSslCertificate::Organization).join(", ")));
    d->infoTable->setItem(rows +  5, 1, new QTableWidgetItem(cert.subjectInfo(QSslCertificate::LocalityName).join(", ")));
    d->infoTable->setItem(rows +  6, 1, new QTableWidgetItem(cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join(", ")));
    d->infoTable->setItem(rows +  7, 1, new QTableWidgetItem(cert.subjectInfo(QSslCertificate::StateOrProvinceName).join(", ")));
    d->infoTable->setItem(rows +  8, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::CommonName).join(", ")));
    d->infoTable->setItem(rows +  9, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::Organization).join(", ")));
    d->infoTable->setItem(rows + 10, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::LocalityName).join(", ")));
    d->infoTable->setItem(rows + 11, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(", ")));
    d->infoTable->setItem(rows + 12, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(", ")));
    d->infoTable->setItem(rows + 13, 1, new QTableWidgetItem(cert.issuerInfo(QSslCertificate::EmailAddress).join(", ")));
    d->infoTable->setItem(rows + 14, 1, new QTableWidgetItem(QString(cert.version())));
  }

  d->infoTable->show();
}


void OptionsDialog::verifySecureConnection(void)
{
  Q_D(OptionsDialog);
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
  qDebug() << "OptionsDialog::sslErrorsOccured()" << errors;
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
