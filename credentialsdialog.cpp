/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>

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

#include "credentialsdialog.h"
#include "ui_credentialsdialog.h"

CredentialsDialog::CredentialsDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::CredentialsDialog)
{
  ui->setupUi(this);
}

CredentialsDialog::~CredentialsDialog()
{
  delete ui;
}


const QString &CredentialsDialog::username(void) const
{
  return ui->usernameLineEdit->text();
}


const QString &CredentialsDialog::password(void) const
{
  return ui->passwordLineEdit->text();
}


void CredentialsDialog::showEvent(QShowEvent *)
{
  ui->usernameLineEdit->selectAll();
  ui->usernameLineEdit->setFocus();
}
