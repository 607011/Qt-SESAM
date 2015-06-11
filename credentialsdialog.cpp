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

#include "3rdparty/cryptopp562/misc.h"
#include "credentialsdialog.h"
#include "ui_credentialsdialog.h"

CredentialsDialog::CredentialsDialog(QWidget *parent)
  : QDialog(parent, Qt::WindowTitleHint)
  , ui(new Ui::CredentialsDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->repeatPasswordLineEdit, SIGNAL(textChanged(QString)), SLOT(checkRepetition(QString)));
  setRepeatPassword(false);
}


CredentialsDialog::~CredentialsDialog()
{
  delete ui;
}


void CredentialsDialog::invalidatePassword(void)
{
  CryptoPP::memset_z(ui->passwordLineEdit->text().data(), 0, ui->passwordLineEdit->text().size());
  ui->passwordLineEdit->clear();
}


void CredentialsDialog::setRepeatPassword(bool doRepeat)
{
  if (doRepeat) {
    ui->repeatPasswordLabel->show();
    ui->repeatPasswordLineEdit->show();
    setWindowTitle(tr("New master password"));
  }
  else {
    ui->repeatPasswordLabel->hide();
    ui->repeatPasswordLineEdit->hide();
    setWindowTitle(tr("Enter master password"));
  }
}


QString CredentialsDialog::password(void) const
{
  return ui->passwordLineEdit->text();
}


void CredentialsDialog::reject(void)
{
  // do nothing
}


void CredentialsDialog::showEvent(QShowEvent *)
{
  ui->passwordLineEdit->selectAll();
  ui->passwordLineEdit->setFocus();
}


void CredentialsDialog::okClicked(void)
{
  if (!ui->passwordLineEdit->text().isEmpty()) {
    if (ui->repeatPasswordLineEdit->isVisible()) {
      if (ui->repeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
        accept();
      }
    }
    else {
      accept();
    }
  }
  else {
    ui->passwordLineEdit->setFocus();
  }
}


void CredentialsDialog::checkRepetition(QString repeatedPassword)
{
  Q_UNUSED(repeatedPassword);
  // TODO ...
}
