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
#include <QByteArray>

#include "newdomainwizard.h"
#include "ui_newdomainwizard.h"
#include "password.h"
#include "crypter.h"
#include "domainsettings.h"
#include "global.h"


class NewDomainWizardPrivate {
public:
  NewDomainWizardPrivate(void)
    : saltSize(12)
    , KGK(nullptr)
  { /* ... */ }
  int saltSize;
  Password password;
  const SecureByteArray *KGK;
};


NewDomainWizard::NewDomainWizard(QWidget *parent)
  : QDialog(parent, Qt::Dialog)
  , ui(new Ui::NewDomainWizard)
  , d_ptr(new NewDomainWizardPrivate)
{
  Q_D(NewDomainWizard);
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctSESAM.ico"));
  QObject::connect(ui->acceptPushButton, SIGNAL(pressed()), SLOT(generatePassword()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->lowercasePushButton, SIGNAL(pressed()), SLOT(addLowercaseToUsedCharacters()));
  QObject::connect(ui->uppercasePushButton, SIGNAL(pressed()), SLOT(addUppercaseToUsedCharacters()));
  QObject::connect(ui->digitsPushButton, SIGNAL(pressed()), SLOT(addDigitsToUsedCharacters()));
  QObject::connect(ui->extraPushButton, SIGNAL(pressed()), SLOT(addExtraCharactersToUsedCharacters()));
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(onUsedCharactersChanged()));
  QObject::connect(ui->usedCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(checkValidity()));
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(checkValidity()));
  QObject::connect(&d->password, SIGNAL(generated()), SLOT(passwordGenerated()));
  ui->forceLowerCaseCheckBox->setToolTip(tr("Force the use of any lower case character"));
  ui->forceUpperCaseCheckBox->setToolTip(tr("Force the use of any upper case character"));
  ui->forceDigitsCheckBox->setToolTip(tr("Force the use of any digit"));
  ui->forceExtraCheckBox->setToolTip(tr("Force the use of any of %1").arg(Password::ExtraChars));
  clear();
}


NewDomainWizard::~NewDomainWizard()
{
  delete ui;
}


void NewDomainWizard::showEvent(QShowEvent *)
{
  checkValidity();
  if (ui->domainLineEdit->text().isEmpty())
    ui->domainLineEdit->setFocus();
}


void NewDomainWizard::closeEvent(QCloseEvent *e)
{
  QDialog::closeEvent(e);
  reject();
}


void NewDomainWizard::clear(void)
{
  ui->domainLineEdit->clear();
  ui->urlLineEdit->clear();
  ui->userLineEdit->clear();
  ui->legacyPasswordLineEdit->clear();
  ui->iterationsSpinBox->setValue(DomainSettings::DefaultIterations);
  ui->passwordLengthSpinBox->setValue(DomainSettings::DefaultPasswordLength);
  ui->notesPlainTextEdit->clear();
  ui->usedCharactersPlainTextEdit->setPlainText(Password::AllChars);
  ui->forceLowerCaseCheckBox->setChecked(false);
  ui->forceUpperCaseCheckBox->setChecked(false);
  ui->forceDigitsCheckBox->setChecked(false);
  ui->forceExtraCheckBox->setChecked(false);
  renewSalt();
  ui->lowercasePushButton->setEnabled(false);
  ui->uppercasePushButton->setEnabled(false);
  ui->digitsPushButton->setEnabled(false);
  ui->extraPushButton->setEnabled(false);
  ui->domainLineEdit->setFocus();
}


void NewDomainWizard::renewSalt(void)
{
  Q_D(NewDomainWizard);
  ui->saltBase64LineEdit->setText(Crypter::randomBytes(d->saltSize).toBase64());
}


bool NewDomainWizard::checkValidity(void)
{
  bool enabled = !ui->domainLineEdit->text().isEmpty() && !ui->usedCharactersPlainTextEdit->toPlainText().isEmpty();
  ui->acceptPushButton->setEnabled(enabled);
  return enabled;
}


QString NewDomainWizard::domain(void) const
{
  return ui->domainLineEdit->text();
}


QString NewDomainWizard::url(void) const
{
  return ui->urlLineEdit->text();
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


void NewDomainWizard::setDomain(const QString &domainName)
{
  ui->domainLineEdit->setText(domainName);
  ui->urlLineEdit->setFocus();
}


void NewDomainWizard::setKGK(const SecureByteArray *kgk)
{
  Q_D(NewDomainWizard);
  d->KGK = kgk;
}


void NewDomainWizard::setSaltSize(int saltLength)
{
  Q_D(NewDomainWizard);
  d->saltSize = saltLength;
  renewSalt();
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


bool NewDomainWizard::passwordContainsAnyOf(const QString &forcedCharacters) const
{
  foreach (QChar c, forcedCharacters) {
    if (d_ptr->password().contains(c))
      return true;
  }
  return false;
}


bool NewDomainWizard::passwordMeetsRules(void) const
{
  if (ui->forceLowerCaseCheckBox && !passwordContainsAnyOf(Password::LowerChars))
    return false;
  if (ui->forceUpperCaseCheckBox && !passwordContainsAnyOf(Password::UpperChars))
    return false;
  if (ui->forceDigitsCheckBox && !passwordContainsAnyOf(Password::Digits))
    return false;
  if (ui->forceExtraCheckBox && !passwordContainsAnyOf(Password::ExtraChars))
    return false;
  return true;
}


void NewDomainWizard::passwordGenerated(void)
{
  Q_D(NewDomainWizard);
  if (!passwordMeetsRules()) {
    renewSalt();
    generatePassword();
  }
  else {
    accept();
  }
}


void NewDomainWizard::generatePassword(void)
{
  Q_D(NewDomainWizard);
  if (!ui->usedCharactersPlainTextEdit->toPlainText().isEmpty()) {
    DomainSettings ds;
    ds.domainName = ui->domainLineEdit->text();
    ds.url = ui->urlLineEdit->text();
    ds.userName = ui->userLineEdit->text();
    ds.notes = ui->notesPlainTextEdit->toPlainText();
    ds.salt_base64 = ui->saltBase64LineEdit->text();
    ds.legacyPassword = ui->legacyPasswordLineEdit->text();
    ds.iterations = ui->iterationsSpinBox->value();
    ds.length = ui->passwordLengthSpinBox->value();
    ds.usedCharacters = ui->usedCharactersPlainTextEdit->toPlainText();
    d->password.generateAsync(*d->KGK, ds);
  }
}
