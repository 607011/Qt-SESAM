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

#include <QFileDialog>
#include <QFileInfo>

OptionsDialog::OptionsDialog(QWidget *parent)
  : QDialog(parent, Qt::Widget)
  , ui(new Ui::OptionsDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->chooseSyncFilePushButton, SIGNAL(pressed()), SLOT(chooseSyncFile()));
  QObject::connect(ui->chooseCertFilePushButton, SIGNAL(pressed()), SLOT(chooseCertFile()));
}


OptionsDialog::~OptionsDialog()
{
  delete ui;
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


const QByteArray &OptionsDialog::serverCredentials(void) const
{
  return mServerCredentials;
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


QString OptionsDialog::serverCertificateFilename(void) const
{
  return ui->certFileLineEdit->text();
}


void OptionsDialog::setServerCertificateFilename(QString filename)
{
  ui->certFileLineEdit->setText(filename);
}


const QList<QSslCertificate> &OptionsDialog::serverCertificates(void) const
{
  return mServerCertificates;
}


QList<QByteArray> OptionsDialog::serverCertificatesPEM(void) const
{
  QList<QByteArray> chain;
  foreach (QSslCertificate cert, mServerCertificates) {
    chain << cert.toPem();
  }
  return chain;
}


void OptionsDialog::setWriteUrl(QString url)
{
  ui->writeUrlLineEdit->setText(url);
}


void OptionsDialog::setReadUrl(QString url)
{
  ui->readUrlLineEdit->setText(url);
}


void OptionsDialog::chooseSyncFile(void)
{
  QFileInfo fi(ui->syncFileLineEdit->text());
  QString chosenFile = QFileDialog::getSaveFileName(this, tr("Choose sync file"), fi.absolutePath());
  if (!chosenFile.isEmpty())
    ui->syncFileLineEdit->setText(chosenFile);
}


void OptionsDialog::chooseCertFile(void)
{
  QFileInfo fi(ui->certFileLineEdit->text());
  QString chosenFile = QFileDialog::getOpenFileName(this, tr("Choose certificate file"), fi.absolutePath());
  if (!chosenFile.isEmpty()) {
    mServerCertificates = QSslCertificate::fromPath(chosenFile, QSsl::Der);
    if (!mServerCertificates.isEmpty())
      ui->certFileLineEdit->setText(chosenFile);
  }
}


void OptionsDialog::okClicked(void)
{
  mServerCredentials = QString("Basic %1")
      .arg(QString((ui->usernameLineEdit->text() + ":" + ui->passwordLineEdit->text())
                   .toLocal8Bit().toBase64()))
      .toLocal8Bit();

  QFileInfo fi(ui->syncFileLineEdit->text());
  if (fi.isFile()) {
    accept();
  }
  else {
    reject();
  }
}
