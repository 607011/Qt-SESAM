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

#include <QByteArray>

#include "newdomainwizard.h"
#include "ui_newdomainwizard.h"
#include "password.h"
#include "global.h"

NewDomainWizard::NewDomainWizard(QWidget *parent)
  : QDialog(parent, Qt::Dialog)
  , ui(new Ui::NewDomainWizard)
{
  ui->setupUi(this);
  ui->usedCharactersPlainTextEdit->setPlainText(Password::AllChars);
  QObject::connect(ui->addPushButton, SIGNAL(pressed()), SLOT(accept()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->lowercasePushButton, SIGNAL(pressed()), SLOT(addLowercaseToUsedCharacters()));
  QObject::connect(ui->uppercasePushButton, SIGNAL(pressed()), SLOT(addUppercaseToUsedCharacters()));
  QObject::connect(ui->digitsPushButton, SIGNAL(pressed()), SLOT(addDigitsToUsedCharacters()));
  QObject::connect(ui->extraPushButton, SIGNAL(pressed()), SLOT(addExtraCharactersToUsedCharacters()));
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(onUsedCharactersChanged()));
  renewSalt();
  ui->domainLineEdit->setFocus();
}


NewDomainWizard::~NewDomainWizard()
{
  delete ui;
}


void NewDomainWizard::renewSalt(void)
{
  QByteArray salt(12, static_cast<char>(0));
  for (int i = 0; i < salt.size(); ++i)
    salt[i] = static_cast<char>(gRandomDevice());
  ui->saltBase64LineEdit->setText(salt.toBase64());
}


QString NewDomainWizard::domain(void) const
{
  return ui->domainLineEdit->text();
}


QString NewDomainWizard::username(void) const
{
  return ui->userLineEdit->text();
}


QString NewDomainWizard::legacyPassword(void) const
{
  return ui->legacyPasswordLineEdit->text();
}


int NewDomainWizard::iterations(void) const
{
  return ui->iterationsSpinBox->value();
}


int NewDomainWizard::passwordLength(void) const
{
  return ui->passwordLengthSpinBox->value();
}


QString NewDomainWizard::salt_base64(void) const
{
  return ui->saltBase64LineEdit->text();
}


QString NewDomainWizard::notes(void) const
{
  return ui->notesPlainTextEdit->toPlainText();
}


QString NewDomainWizard::usedCharacters(void) const
{
  return ui->usedCharactersPlainTextEdit->toPlainText();
}


bool NewDomainWizard::forceLowercase(void) const
{
  return ui->forceLowerCaseCheckBox->isChecked();
}


bool NewDomainWizard::forceUppercase(void) const
{
  return ui->forceUpperCaseCheckBox->isChecked();
}


bool NewDomainWizard::forceDigits(void) const
{
  return ui->forceDigitsCheckBox->isChecked();
}


bool NewDomainWizard::forceExtra(void) const
{
  return ui->forceExtraCheckBox->isChecked();
}


void NewDomainWizard::setForceLowercase(bool doForce)
{
  ui->forceLowerCaseCheckBox->setChecked(doForce);
}


void NewDomainWizard::setForceUppercase(bool doForce)
{
  ui->forceUpperCaseCheckBox->setChecked(doForce);
}


void NewDomainWizard::setForceDigits(bool doForce)
{
  ui->forceDigitsCheckBox->setChecked(doForce);
}


void NewDomainWizard::setForceExtra(bool doForce)
{
  ui->forceExtraCheckBox->setChecked(doForce);
}


void NewDomainWizard::addLowercaseToUsedCharacters(void)
{
  ui->usedCharactersPlainTextEdit->setPlainText(ui->usedCharactersPlainTextEdit->toPlainText() + Password::LowerChars);
}


void NewDomainWizard::addUppercaseToUsedCharacters(void)
{
  ui->usedCharactersPlainTextEdit->setPlainText(ui->usedCharactersPlainTextEdit->toPlainText() + Password::UpperChars);
}


void NewDomainWizard::addDigitsToUsedCharacters(void)
{
  ui->usedCharactersPlainTextEdit->setPlainText(ui->usedCharactersPlainTextEdit->toPlainText() + Password::Digits);
}


void NewDomainWizard::addExtraCharactersToUsedCharacters(void)
{
  ui->usedCharactersPlainTextEdit->setPlainText(ui->usedCharactersPlainTextEdit->toPlainText() + Password::ExtraChars);
}


void NewDomainWizard::onUsedCharactersChanged(void)
{
  ui->lowercasePushButton->setEnabled(true);
  ui->uppercasePushButton->setEnabled(true);
  ui->digitsPushButton->setEnabled(true);
  ui->extraPushButton->setEnabled(true);
}
