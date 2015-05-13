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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QClipboard>
#include <QtConcurrent>
#include <QMessageBox>

#include "cryptopp562/pwdbased.h"
#include "cryptopp562/sha.h"

#include "bigint/bigInt.h"

#include "util.h"

static const QString CompanyName = "c't";
static const QString AppName = "pwdgen";


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mSettings(QSettings::IniFormat, QSettings::UserScope, CompanyName, AppName)
  , mLoaderIcon(":/images/loader.gif")
  , mElapsed(0)
  , mCustomCharacterSetDirty(false)
  , mAutoIncreaseIterations(true)
  , mCompleter(0)
{
  ui->setupUi(this);
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->masterPasswordLineEdit1, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->masterPasswordLineEdit2, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->charactersPlainTextEdit, SIGNAL(textChanged()), SLOT(updatePassword()));
  QObject::connect(ui->charactersPlainTextEdit, SIGNAL(textChanged()), SLOT(customCharacterSetChanged()));
  QObject::connect(ui->forceCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(updateValidator()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->digitsCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->extrasCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->upperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->lowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->customCharacterSetCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->customCharacterSetCheckBox, SIGNAL(toggled(bool)), SLOT(customCharacterSetCheckBoxToggled(bool)));
  QObject::connect(ui->copyPasswordToClipboardPushButton, SIGNAL(pressed()), SLOT(copyPasswordToClipboard()));
  QObject::connect(ui->savePushButton, SIGNAL(pressed()), SLOT(saveCurrentSettings()));
  QObject::connect(this, SIGNAL(passwordGenerated(QString)), SLOT(onPasswordGenerated(QString)));
  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&mLoaderIcon);
  ui->processLabel->hide();
  restoreSettings();
  updateUsedCharacters();
  updateValidator();
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent *)
{
  saveSettings();
}


void MainWindow::domainSelected(const QString &domain)
{
  loadSettings(domain);
}


void MainWindow::updatePassword(void)
{
  bool valid = false;
  ui->statusBar->showMessage("");
  if (!ui->masterPasswordLineEdit1->text().isEmpty() && !ui->masterPasswordLineEdit2->text().isEmpty()) {
    if (ui->masterPasswordLineEdit1->text() != ui->masterPasswordLineEdit2->text()) {
      ui->statusBar->showMessage(tr("Passwords do not match"), 2000);
    }
    else {
      ui->copyPasswordToClipboardPushButton->setEnabled(false);
      ui->processLabel->show();
      mLoaderIcon.start();
      valid = true;
      if (mPasswordGeneratorFuture.isFinished())
        mPasswordGeneratorFuture = QtConcurrent::run(this, &MainWindow::generatePassword);
    }
  }
  if (!valid) {
    ui->generatedPasswordLineEdit->setText(tr("<invalid>"));
  }
}


void MainWindow::updateUsedCharacters(void)
{
  if (!ui->customCharacterSetCheckBox->isChecked()) {
    QString passwordCharacters;
    static const QString UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const QString LowerChars = "abcdefghijklmnopqrstuvwxyz";
    static const QString Digits = "0123456789";
    static const QString ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";
    if (ui->lowerCaseCheckBox->isChecked())
      passwordCharacters += LowerChars;
    if (ui->upperCaseCheckBox->isChecked())
      passwordCharacters += UpperChars;
    if (ui->digitsCheckBox->isChecked())
      passwordCharacters += Digits;
    if (ui->extrasCheckBox->isChecked())
      passwordCharacters += ExtraChars;
    ui->charactersPlainTextEdit->blockSignals(true);
    ui->charactersPlainTextEdit->setPlainText(passwordCharacters);
    ui->charactersPlainTextEdit->blockSignals(false);
  }
  updatePassword();
}


void MainWindow::copyPasswordToClipboard(void)
{
  QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Password copied to clipboard."));
}


void MainWindow::onPasswordGenerated(QString key)
{
  ui->processLabel->hide();
  ui->copyPasswordToClipboardPushButton->setEnabled(true);
  int pos = 0;
  bool setKey = true;
  if (ui->forceCustomCharacterSetCheckBox->isChecked() && !mValidator.validate(key, pos))
    setKey = false;
  if (setKey) {
    ui->generatedPasswordLineEdit->setText(key);
    ui->statusBar->showMessage(tr("generation time: %1 ms").arg(mElapsed, 0, 'f', 4), 3000);
  }
  else {
    ui->statusBar->showMessage(tr("Password does not match regular expression. %1").arg(mAutoIncreaseIterations ? tr("Increasing iteration count.") : ""));
    if (mAutoIncreaseIterations)
      ui->iterationsSpinBox->setValue( ui->iterationsSpinBox->value() + 1);
  }
}


void MainWindow::customCharacterSetCheckBoxToggled(bool checked)
{
  bool reallyToggle = true;
  if (!checked && mCustomCharacterSetDirty) {
    int button = QMessageBox::warning(
          this,
          tr("Really uncheck this option?"),
          tr("Custom characters have changed. If you uncheck the checkbox you will lose your changes. Do you really want to continue?"),
          QMessageBox::Yes,
          QMessageBox::No);
    if (button != QMessageBox::Yes) {
      reallyToggle = false;
      ui->customCharacterSetCheckBox->setChecked(true);
    }
  }
  if (reallyToggle) {
    ui->lowerCaseCheckBox->setEnabled(!checked);
    ui->upperCaseCheckBox->setEnabled(!checked);
    ui->digitsCheckBox->setEnabled(!checked);
    ui->extrasCheckBox->setEnabled(!checked);
    updateUsedCharacters();
    if (!checked)
      mCustomCharacterSetDirty = false;
  }
}


void MainWindow::customCharacterSetChanged(void)
{
  ui->customCharacterSetCheckBox->setChecked(true);
  mCustomCharacterSetDirty = true;
}


void MainWindow::updateValidator(void)
{
  QRegExp re(ui->forceCharactersPlainTextEdit->toPlainText(), Qt::CaseSensitive, QRegExp::RegExp2);
  if (re.isValid()) {
    mValidator.setRegExp(re);
    ui->statusBar->showMessage("");
    updatePassword();
  }
  else {
    ui->statusBar->showMessage(re.errorString());
  }
}


void MainWindow::saveCurrentSettings(void)
{
  DomainSettings domainSettings;
  domainSettings.useLowerCase = ui->lowerCaseCheckBox->isChecked();
  domainSettings.useUpperCase = ui->upperCaseCheckBox->isChecked();
  domainSettings.useDigits = ui->digitsCheckBox->isChecked();
  domainSettings.useExtra = ui->extrasCheckBox->isChecked();
  domainSettings.iterations = ui->iterationsSpinBox->value();
  domainSettings.salt = ui->saltLineEdit->text();
  domainSettings.validator = mValidator.regExp();
  saveDomainSettings(ui->domainLineEdit->text(), domainSettings);
  mSettings.sync();
  ui->statusBar->showMessage(tr("Settings saved."), 3000);
}


void MainWindow::saveDomainSettings(const QString &domain, const DomainSettings &domainSettings)
{
  QStringListModel *model = reinterpret_cast<QStringListModel*>(ui->domainLineEdit->completer()->model());
  QStringList domains = model->stringList();
  if (!domains.contains(domain, Qt::CaseInsensitive)) {
    domains << domain;
    model->setStringList(domains);
  }
  mSettings.setValue(domain + "/useLowerCase", domainSettings.useLowerCase);
  mSettings.setValue(domain + "/useUpperCase", domainSettings.useUpperCase);
  mSettings.setValue(domain + "/useDigits", domainSettings.useDigits);
  mSettings.setValue(domain + "/useExtra", domainSettings.useExtra);
  mSettings.setValue(domain + "/iterations", domainSettings.iterations);
  mSettings.setValue(domain + "/salt", domainSettings.salt);
  mSettings.setValue(domain + "/validator/pattern", domainSettings.validator.pattern());
}


void MainWindow::saveSettings(void)
{
  mSettings.setValue("mainwindow/geometry", geometry());
  QStringListModel *model = reinterpret_cast<QStringListModel*>(ui->domainLineEdit->completer()->model());
  mSettings.setValue("domains", model->stringList());
  mSettings.sync();
}


void MainWindow::restoreSettings(void)
{
  restoreGeometry(mSettings.value("mainwindow/geometry").toByteArray());
  QStringList domains = mSettings.value("domains").toStringList();
  if (mCompleter) {
    QObject::disconnect(ui->domainLineEdit->completer(), SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
  }
  safeRenew(mCompleter, new QCompleter(domains));
  ui->domainLineEdit->setCompleter(mCompleter);
  QObject::connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
}


void MainWindow::loadSettings(const QString &domain)
{
  ui->lowerCaseCheckBox->setChecked(mSettings.value(domain + "/useLowerCase").toBool());
  ui->upperCaseCheckBox->setChecked(mSettings.value(domain + "/useUpperCase").toBool());
  ui->digitsCheckBox->setChecked(mSettings.value(domain + "/useDigits").toBool());
  ui->extrasCheckBox->setChecked(mSettings.value(domain + "/useExtra").toBool());
  ui->iterationsSpinBox->setValue(mSettings.value(domain + "/iterations").toInt());
  ui->saltLineEdit->setText(mSettings.value(domain + "/salt").toString());
  ui->forceCharactersPlainTextEdit->setPlainText(mSettings.value(domain + "/validator/pattern").toString());
  updateValidator();
  updatePassword();
}


void MainWindow::generatePassword(void)
{
  const QByteArray &domain = ui->domainLineEdit->text().toUtf8();
  const QByteArray &salt = ui->saltLineEdit->text().toUtf8();
  const QByteArray &masterPwd = ui->masterPasswordLineEdit1->text().toUtf8();
  const QByteArray &pwd = domain + masterPwd;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
  const int nChars = ui->charactersPlainTextEdit->toPlainText().count();
  byte *derived = new byte[nChars];
  mElapsedTimer.start();
  pbkdf2.DeriveKey(
        derived,
        nChars,
        reinterpret_cast<const byte*>(pwd.data()),
        pwd.count(),
        reinterpret_cast<const byte*>(salt.data()),
        salt.count(),
        ui->iterationsSpinBox->value()
        );
  mElapsed = 1e-6 * mElapsedTimer.nsecsElapsed();
  const QByteArray &derivedKeyBuf = QByteArray(reinterpret_cast<char*>(derived), nChars);
  const QByteArray &hexKey = derivedKeyBuf.toHex();
  const QString strModulus = QString("%1").arg(nChars);
  BigInt::Rossi v(QString(hexKey).toStdString(), BigInt::HEX_DIGIT);
  const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
  static const BigInt::Rossi Zero(0);
  QString key;
  int n = ui->passwordLengthSpinBox->value();
  while (v > Zero && n-- > 0) {
    BigInt::Rossi mod = v % Modulus;
    key += ui->charactersPlainTextEdit->toPlainText().at(mod.toUlong());
    v = v / Modulus;
  }
  delete[] derived;
  emit passwordGenerated(key);
}
