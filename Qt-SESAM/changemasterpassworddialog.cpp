/*

    Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

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

#include <QDebug>
#include <QColor>
#include "changemasterpassworddialog.h"
#include "ui_changemasterpassworddialog.h"
#include "passwordchecker.h"
#include "util.h"


class ChangeMasterPasswordDialogPrivate
{
public:
  ChangeMasterPasswordDialogPrivate(void)
    : passwordChecker(Q_NULLPTR)
  { /* ... */ }
  ~ChangeMasterPasswordDialogPrivate()
  {
    SafeDelete(passwordChecker);
  }

  PasswordChecker *passwordChecker;
};


ChangeMasterPasswordDialog::ChangeMasterPasswordDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ChangeMasterPasswordDialog)
  , d_ptr(new ChangeMasterPasswordDialogPrivate)
{
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctSESAM.ico"));
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->newPasswordLineEdit1, SIGNAL(textChanged(QString)), SLOT(comparePasswords()));
  QObject::connect(ui->newPasswordLineEdit2, SIGNAL(textChanged(QString)), SLOT(comparePasswords()));
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


void ChangeMasterPasswordDialog::okClicked(void)
{
  if (ui->currentPasswordLineEdit->text().isEmpty()) {
    ui->currentPasswordLineEdit->setFocus();
  }
  else {
    accept();
  }
}


QString ChangeMasterPasswordDialog::oldPassword(void) const
{
  return ui->currentPasswordLineEdit->text();
}


QString ChangeMasterPasswordDialog::newPassword(void) const
{
  return ui->newPasswordLineEdit1->text();
}


void ChangeMasterPasswordDialog::setPasswordFilename(const QString &filename)
{
  Q_D(ChangeMasterPasswordDialog);
  if (filename.isEmpty()) {
    SafeDelete(d->passwordChecker);
  }
  else {
    SafeRenew(d->passwordChecker, new PasswordChecker(filename));
  }
}


void ChangeMasterPasswordDialog::showEvent(QShowEvent *)
{
  ui->newPasswordLineEdit1->clear();
  ui->newPasswordLineEdit2->clear();
  ui->currentPasswordLineEdit->clear();
  ui->currentPasswordLineEdit->setFocus();
  comparePasswords();
}


void ChangeMasterPasswordDialog::comparePasswords(void)
{
  Q_D(ChangeMasterPasswordDialog);
  QString password = ui->newPasswordLineEdit1->text();
  if (!password.isEmpty()) {
    bool found = false;
    QString grade;
    QColor color;
    if (d->passwordChecker != Q_NULLPTR) {
      qint64 pos = d->passwordChecker->findInPasswordFile(password);
      found = (pos >= 0);
      if (found) {
        grade = tr("Listed");
        color.setRgb(147, 209, 240);
      }
    }
    if (!found) {
      PasswordChecker::evaluatePasswordStrength(ui->newPasswordLineEdit1->text(), color, grade, Q_NULLPTR);
    }
    ui->strengthLabel->setText(tr("%1").arg(grade));
    ui->strengthLabel->setStyleSheet(QString("background-color: rgb(%1, %2, %3); font-weight: bold").arg(color.red()).arg(color.green()).arg(color.blue()));
  }
  ui->okPushButton->setEnabled(!ui->newPasswordLineEdit1->text().isEmpty() && ui->newPasswordLineEdit1->text() == ui->newPasswordLineEdit2->text());
}
