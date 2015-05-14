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

#include "util.h"

static const QString CompanyName = "c't";
static const QString AppName = "ctpwdgen";
static const QString AppVersion = "1.0 ALPHA";
static const QString AppUrl = "https://github.com/ola-ct/ctpwdgen";
static const QString AppAuthor = "Oliver Lau";
static const QString AppAuthorMail = "ola@ct.de";

#ifdef QT_DEBUG
#include "testpbkdf2.h"
#endif


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mSettings(QSettings::IniFormat, QSettings::UserScope, CompanyName, AppName)
  , mLoaderIcon(":/images/loader.gif")
  , mElapsed(0)
  , mCustomCharacterSetDirty(false)
  , mAutoIncreaseIterations(true)
  , mCompleter(0)
  , mQuitHashing(false)
{
  ui->setupUi(this);
  setWindowTitle(AppName + " " + AppVersion);
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
  QObject::connect(ui->avoidAmbiguousCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->copyPasswordToClipboardPushButton, SIGNAL(pressed()), SLOT(copyPasswordToClipboard()));
  QObject::connect(ui->savePushButton, SIGNAL(pressed()), SLOT(saveCurrentSettings()));
  QObject::connect(this, SIGNAL(passwordGenerated(QString, QString)), SLOT(onPasswordGenerated(QString, QString)));
  QObject::connect(ui->actionNewDomain, SIGNAL(triggered(bool)), SLOT(newDomain()));
  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));
  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&mLoaderIcon);
  ui->processLabel->hide();
  restoreSettings();
  updateUsedCharacters();
  updateValidator();

#ifdef QT_DEBUG
  TestPBKDF2 tc;
  QTest::qExec(&tc, 0, 0);
#endif
}


MainWindow::~MainWindow()
{
  delete ui;
  safeDelete(mCompleter);
}


void MainWindow::closeEvent(QCloseEvent *)
{
  saveSettings();
}


void MainWindow::domainSelected(const QString &domain)
{
  loadSettings(domain);
}


void MainWindow::newDomain(void)
{
  DomainSettings domainSettings;
  ui->domainLineEdit->setText(QString());
  ui->domainLineEdit->setFocus();
  ui->masterPasswordLineEdit1->setText(QString());
  ui->masterPasswordLineEdit2->setText(QString());
  ui->lowerCaseCheckBox->setChecked(domainSettings.useLowerCase);
  ui->upperCaseCheckBox->setChecked(domainSettings.useUpperCase);
  ui->digitsCheckBox->setChecked(domainSettings.useDigits);
  ui->extrasCheckBox->setChecked(domainSettings.useExtra);
  ui->iterationsSpinBox->setValue(domainSettings.iterations);
  ui->passwordLengthSpinBox->setValue(domainSettings.length);
  ui->saltLineEdit->setText(domainSettings.salt);
  ui->forceCharactersPlainTextEdit->setPlainText(domainSettings.validatorRegEx.pattern());
  ui->forceCustomCharacterSetCheckBox->setChecked(domainSettings.forceValidation);
  updateValidator();
  updatePassword();

}


void MainWindow::updatePassword(void)
{
  bool validConfiguration = false;
  ui->statusBar->showMessage(QString());
  if (ui->charactersPlainTextEdit->toPlainText().count() > 0 &&
      !ui->masterPasswordLineEdit1->text().isEmpty() &&
      !ui->masterPasswordLineEdit2->text().isEmpty())
  {
    if (ui->masterPasswordLineEdit1->text() != ui->masterPasswordLineEdit2->text()) {
      ui->statusBar->showMessage(tr("Passwords do not match"), 2000);
    }
    else {
      ui->copyPasswordToClipboardPushButton->setEnabled(false);
      ui->processLabel->show();
      mLoaderIcon.start();
      validConfiguration = true;
      stopPasswordGeneration();
      mPasswordGeneratorFuture = QtConcurrent::run(this, &MainWindow::generatePassword);
    }
  }
  if (!validConfiguration) {
    ui->generatedPasswordLineEdit->setText(QString());
    ui->hashPlainTextEdit->setPlainText(QString());
  }
}


void MainWindow::updateUsedCharacters(void)
{
  if (!ui->customCharacterSetCheckBox->isChecked()) {
    stopPasswordGeneration();
    QString passwordCharacters;
    static const QString UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const QString UpperCharsNoAmbiguous = "ABCDEFGHJKLMNPQRTUVWXYZ";
    static const QString LowerChars = "abcdefghijklmnopqrstuvwxyz";
    static const QString Digits = "0123456789";
    static const QString ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";
    if (ui->lowerCaseCheckBox->isChecked())
      passwordCharacters += LowerChars;
    if (ui->upperCaseCheckBox->isChecked())
      passwordCharacters += ui->avoidAmbiguousCheckBox->isChecked() ? UpperCharsNoAmbiguous : UpperChars;
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


void MainWindow::onPasswordGenerated(QString key, QString hexKey)
{
  ui->processLabel->hide();
  ui->copyPasswordToClipboardPushButton->setEnabled(true);
  int pos = 0;
  bool setKey = true; //XXX
  if (ui->forceCustomCharacterSetCheckBox->isChecked() && !mValidator.validate(key, pos))
    setKey = false;
  if (setKey) {
    ui->generatedPasswordLineEdit->setText(key);
    ui->hashPlainTextEdit->setPlainText(hexKey);
    ui->statusBar->showMessage(tr("generation time: %1 ms").arg(mElapsed, 0, 'f', 4), 3000);
  }
  else {
    ui->statusBar->showMessage(tr("Password does not match regular expression. %1").arg(mAutoIncreaseIterations ? tr("Increasing iteration count.") : QString()));
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
    ui->statusBar->showMessage(QString());
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
  domainSettings.validatorRegEx = mValidator.regExp();
  domainSettings.forceValidation = ui->forceCustomCharacterSetCheckBox->isChecked();
  saveDomainSettings(ui->domainLineEdit->text(), domainSettings);
  mSettings.sync();
  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
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
  mSettings.setValue(domain + "/length", domainSettings.length);
  mSettings.setValue(domain + "/salt", domainSettings.salt);
  mSettings.setValue(domain + "/validator/pattern", domainSettings.validatorRegEx.pattern());
  mSettings.setValue(domain + "/validator/force", domainSettings.forceValidation);
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
  ui->lowerCaseCheckBox->setChecked(mSettings.value(domain + "/useLowerCase", true).toBool());
  ui->upperCaseCheckBox->setChecked(mSettings.value(domain + "/useUpperCase", true).toBool());
  ui->digitsCheckBox->setChecked(mSettings.value(domain + "/useDigits", true).toBool());
  ui->extrasCheckBox->setChecked(mSettings.value(domain + "/useExtra", false).toBool());
  ui->iterationsSpinBox->setValue(mSettings.value(domain + "/iterations", DomainSettings::DefaultIterations).toInt());
  ui->passwordLengthSpinBox->setValue(mSettings.value(domain + "/length", DomainSettings::DefaultPasswordLength).toInt());
  ui->saltLineEdit->setText(mSettings.value(domain + "/salt", DomainSettings::DefaultSalt).toString());
  ui->forceCharactersPlainTextEdit->setPlainText(mSettings.value(domain + "/validator/pattern", DomainSettings::DefaultValidatorPattern).toString());
  ui->forceCustomCharacterSetCheckBox->setChecked(mSettings.value(domain + "/validator/force", false).toBool());
  updateValidator();
  updatePassword();
}


void MainWindow::generatePassword(void)
{
  QString key;
  QByteArray hexKey;
  mPasswordGenerator.generate(
        ui->domainLineEdit->text().toUtf8(),
        ui->saltLineEdit->text().toUtf8(),
        ui->masterPasswordLineEdit1->text().toUtf8(),
        ui->charactersPlainTextEdit->toPlainText(),
        ui->passwordLengthSpinBox->value(),
        ui->iterationsSpinBox->value(),
        mQuitHashing,
        mElapsed,
        key,
        hexKey
        );
  emit passwordGenerated(key, hexKey);
}


void MainWindow::stopPasswordGeneration(void)
{
  if (mPasswordGeneratorFuture.isRunning()) {
    mQuitHashing = true;
    mPasswordGeneratorFuture.waitForFinished();
  }
  mQuitHashing = false;
}


void MainWindow::about(void)
{
    QMessageBox::about(this, tr("About %1 %2").arg(AppName).arg(AppVersion),
                       tr("<p><b>%1</b> is a domain specific password generator. "
                          "See <a href=\"%2\" title=\"%1 project homepage\">%2</a> for more info.</p>"
                          "<p>Copyright &copy; 2015 %3 &lt;%4&gt;, Heise Medien GmbH &amp; Co. KG.</p>"
                          "<p>This program is free software: you can redistribute it and/or modify "
                          "it under the terms of the GNU General Public License as published by "
                          "the Free Software Foundation, either version 3 of the License, or "
                          "(at your option) any later version.</p>"
                          "<p>This program is distributed in the hope that it will be useful, "
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
                          "GNU General Public License for more details.</p>"
                          "You should have received a copy of the GNU General Public License "
                          "along with this program. "
                          "If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0\">http://www.gnu.org/licenses</a>.</p>"
                          "<p>No animals were harmed during the development of this software. "
                          "It was programmed CO2 neutrally and without the use of genetic engineering. "
                          "It is vegan, free of antibiotics and hypoallergenic.</p>")
                       .arg(AppName).arg(AppUrl).arg(AppAuthor).arg(AppAuthorMail));
}


void MainWindow::aboutQt(void)
{
    QMessageBox::aboutQt(this);
}
