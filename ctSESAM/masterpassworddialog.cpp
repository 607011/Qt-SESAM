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

#include "masterpassworddialog.h"
#include "ui_masterpassworddialog.h"
#include "util.h"
#include "global.h"


class MasterPasswordDialogPrivate {
public:
  MasterPasswordDialogPrivate(void)
    : repeatPasswordLineEdit(nullptr)
    , doRepeatPassword(false)
  { /* ... */ }
  QLineEdit *repeatPasswordLineEdit;
  bool doRepeatPassword;
};


MasterPasswordDialog::MasterPasswordDialog(QWidget *parent)
  : QDialog(parent, Qt::WindowTitleHint)
  , ui(new Ui::MasterPasswordDialog)
  , d_ptr(new MasterPasswordDialogPrivate)
{
  ui->setupUi(this);
  ui->infoLabel->setStyleSheet("font-weight: bold");
  setWindowTitle(QString("%1 %2").arg(AppName).arg(AppVersion));
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->passwordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  setRepeatPassword(false);
}


MasterPasswordDialog::~MasterPasswordDialog()
{
  invalidatePassword();
  delete ui;
}


void MasterPasswordDialog::invalidatePassword(void)
{
  SecureErase(ui->passwordLineEdit->text());
  ui->passwordLineEdit->clear();
}


void MasterPasswordDialog::setRepeatPassword(bool doRepeat)
{
  Q_D(MasterPasswordDialog);
  d->doRepeatPassword = doRepeat;
  if (doRepeat) {
    invalidatePassword();
    SafeRenew(d->repeatPasswordLineEdit, new QLineEdit);
    d->repeatPasswordLineEdit->setEchoMode(QLineEdit::Password);
    ui->formLayout->insertRow(1, tr("Repeat password"), d->repeatPasswordLineEdit);
    setTabOrder(ui->passwordLineEdit, d->repeatPasswordLineEdit);
    ui->infoLabel->setText(tr("New master password"));
    QObject::connect(d->repeatPasswordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  }
  else {
    ui->infoLabel->setText(tr("Enter master password"));
  }
}


bool MasterPasswordDialog::repeatPassword(void) const
{
  return d_ptr->doRepeatPassword;
}


QString MasterPasswordDialog::masterPassword(void) const
{
  return ui->passwordLineEdit->text();
}


void MasterPasswordDialog::reject(void)
{
  // do nothing
}


void MasterPasswordDialog::showEvent(QShowEvent *)
{
  ui->passwordLineEdit->selectAll();
  ui->passwordLineEdit->setFocus();
}


void MasterPasswordDialog::okClicked(void)
{
  Q_D(MasterPasswordDialog);
  if (ui->passwordLineEdit->text().isEmpty()) {
    ui->passwordLineEdit->setFocus();
  }
  else {
    if (d->repeatPasswordLineEdit != nullptr) {
      if (d->repeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
        QWidget *label = ui->formLayout->labelForField(d->repeatPasswordLineEdit);
        label->deleteLater();
        d->repeatPasswordLineEdit->deleteLater();
        SafeDelete(d->repeatPasswordLineEdit);
        accept();
      }
    }
    else {
      accept();
    }
  }
}


void MasterPasswordDialog::comparePasswords(void)
{
  Q_D(MasterPasswordDialog);
  if (d->repeatPasswordLineEdit == nullptr)
    return;
  if (d->repeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
    ui->okPushButton->setEnabled(true);
  }
  else {
    ui->okPushButton->setEnabled(false);
  }
}
