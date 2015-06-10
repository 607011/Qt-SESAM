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

#include "credentialsdialog.h"
#include "ui_credentialsdialog.h"

CredentialsDialog::CredentialsDialog(QWidget *parent)
  : QDialog(parent, Qt::WindowTitleHint)
  , ui(new Ui::CredentialsDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
}


CredentialsDialog::~CredentialsDialog()
{
  delete ui;
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
    accept();
  }
  else {
    ui->passwordLineEdit->setFocus();
  }
}
