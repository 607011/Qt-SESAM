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

#include <QDebug>
#include "masterpassworddialog.h"
#include "ui_masterpassworddialog.h"
#include "util.h"
#include "global.h"



MasterPasswordDialog::MasterPasswordDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::MasterPasswordDialog)
{
  ui->setupUi(this);
  ui->infoLabel->setStyleSheet("font-weight: bold");
  setWindowTitle(QString("%1 %2").arg(AppName).arg(AppVersion));
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(okClicked()));
  QObject::connect(ui->passwordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  QObject::connect(ui->repeatPasswordLineEdit, SIGNAL(textEdited(QString)), SLOT(comparePasswords()));
  setRepeatPassword(false);
  invalidatePassword();
  comparePasswords();
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
  SecureErase(ui->repeatPasswordLineEdit->text());
  ui->repeatPasswordLineEdit->clear();
}


void MasterPasswordDialog::setRepeatPassword(bool doRepeat)
{
  if (doRepeat) {
    invalidatePassword();
    ui->infoLabel->setText(tr("New master password"));
    ui->repeatPasswordLineEdit->show();
    ui->repeatPasswordLabel->show();
    ui->strengthLabel->show();
  }
  else {
    ui->infoLabel->setText(tr("Enter master password"));
    ui->repeatPasswordLineEdit->hide();
    ui->repeatPasswordLabel->hide();
    ui->strengthLabel->hide();
  }
  comparePasswords();
}


QString MasterPasswordDialog::masterPassword(void) const
{
  return ui->passwordLineEdit->text();
}


void MasterPasswordDialog::reject(void)
{
  // ignore
}


void MasterPasswordDialog::showEvent(QShowEvent*)
{
  ui->passwordLineEdit->selectAll();
  ui->passwordLineEdit->setFocus();
}


void MasterPasswordDialog::closeEvent(QCloseEvent*)
{
  // ...
}


void MasterPasswordDialog::okClicked(void)
{
  if (ui->passwordLineEdit->text().isEmpty()) {
    ui->passwordLineEdit->setFocus();
  }
  else if (ui->repeatPasswordLabel->isVisible()) {
    if (ui->repeatPasswordLineEdit->text() == ui->passwordLineEdit->text()) {
      accept();
    }
  }
  else {
    accept();
  }
}


void MasterPasswordDialog::comparePasswords(void)
{
  if (ui->repeatPasswordLineEdit->isVisible()) {
    QString grade;
    QColor color;
    evaluatePasswordStrength<float>(ui->passwordLineEdit->text(), color, grade, nullptr);
    ui->strengthLabel->setText(tr("%1").arg(grade));
    ui->strengthLabel->setStyleSheet(QString("background-color: rgb(%1, %2, %3); font-weight: bold").arg(color.red()).arg(color.green()).arg(color.blue()));
    ui->okPushButton->setEnabled(!ui->passwordLineEdit->text().isEmpty() && ui->repeatPasswordLineEdit->text() == ui->passwordLineEdit->text());
  }
}
