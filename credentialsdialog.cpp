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
#include "util.h"
#include "global.h"

CredentialsDialog::CredentialsDialog(QWidget *parent)
  : QDialog(parent, Qt::WindowTitleHint)
  , ui(new Ui::CredentialsDialog)
  , mRepeatPasswordLineEdit(nullptr)
{
  ui->setupUi(this);
  ui->infoLabel->setStyleSheet("font-weight: bold");
  setWindowTitle(QString("%1 %2").arg(APP_NAME).arg(APP_VERSION));
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->passwordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  setRepeatPassword(false);
}


CredentialsDialog::~CredentialsDialog()
{
  invalidatePassword();
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
    invalidatePassword();
    SafeRenew(mRepeatPasswordLineEdit, new QLineEdit);
    mRepeatPasswordLineEdit->setEchoMode(QLineEdit::Password);
    ui->formLayout->insertRow(1, tr("Repeat password"), mRepeatPasswordLineEdit);
    setTabOrder(ui->passwordLineEdit, mRepeatPasswordLineEdit);
    ui->infoLabel->setText(tr("New master password"));
    QObject::connect(mRepeatPasswordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  }
  else {
    ui->infoLabel->setText(tr("Enter master password"));
  }
}


QString CredentialsDialog::masterPassword(void) const
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
  if (ui->passwordLineEdit->text().isEmpty()) {
    ui->passwordLineEdit->setFocus();
  }
  else {
    if (mRepeatPasswordLineEdit != nullptr) {
      if (mRepeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
        QWidget *label = ui->formLayout->labelForField(mRepeatPasswordLineEdit);
        label->deleteLater();
        mRepeatPasswordLineEdit->deleteLater();
        SafeDelete(mRepeatPasswordLineEdit);
        accept();
      }
    }
    else {
      accept();
    }
  }
}


void CredentialsDialog::comparePasswords(void)
{
  if (mRepeatPasswordLineEdit == nullptr)
    return;
  if (mRepeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
    ui->okPushButton->setEnabled(true);
  }
  else {
    ui->okPushButton->setEnabled(false);
  }
}
