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


#include "changemasterpassworddialog.h"
#include "ui_changemasterpassworddialog.h"
#include "util.h"

ChangeMasterPasswordDialog::ChangeMasterPasswordDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ChangeMasterPasswordDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->newPasswordLineEdit1, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  QObject::connect(ui->newPasswordLineEdit2, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
}


ChangeMasterPasswordDialog::~ChangeMasterPasswordDialog()
{
  invalidate();
  delete ui;
}


void ChangeMasterPasswordDialog::invalidate(void)
{
  SecureErase(ui->currentPasswordLineEdit->text());
  SecureErase(ui->newPasswordLineEdit1->text());
  SecureErase(ui->newPasswordLineEdit2->text());
}


void ChangeMasterPasswordDialog::reject(void)
{
  // do nothing ...
}


void ChangeMasterPasswordDialog::okClicked(void)
{
  if (ui->currentPasswordLineEdit->text().isEmpty()) {
    ui->currentPasswordLineEdit->setFocus();
  }
  else {

  }
}


const QString &ChangeMasterPasswordDialog::oldPassword(void) const
{
  return ui->currentPasswordLineEdit->text();
}


const QString &ChangeMasterPasswordDialog::newPassword(void) const
{
  return ui->newPasswordLineEdit1->text();
}


void ChangeMasterPasswordDialog::showEvent(QShowEvent *)
{
  ui->newPasswordLineEdit1->clear();
  ui->newPasswordLineEdit2->clear();
  ui->currentPasswordLineEdit->clear();
  ui->currentPasswordLineEdit->setFocus();
}


void ChangeMasterPasswordDialog::comparePasswords(void)
{
  ui->okPushButton->setEnabled(ui->newPasswordLineEdit1->text() == ui->newPasswordLineEdit2->text());
}
