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
#include <QMessageBox>
#include <QStringListModel>

#include "util.h"

static const QString CompanyName = "c't";
static const QString AppName = "ctpwdgen";
static const QString AppVersion = "1.0 ALPHA";
static const QString AppUrl = "https://github.com/ola-ct/ctpwdgen";
static const QString AppAuthor = "Oliver Lau";
static const QString AppAuthorMail = "ola@ct.de";


#ifdef QT_DEBUG
#include <QtDebug>
#include "testpbkdf2.h"
#endif


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mSettings(QSettings::IniFormat, QSettings::UserScope, CompanyName, AppName)
  , mLoaderIcon(":/images/loader.gif")
  , mCustomCharacterSetDirty(false)
  , mParameterSetDirty(false)
  , mAutoIncreaseIterations(true)
  , mCompleter(0)
{
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctpwdgen.ico"));
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->userLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->masterPasswordLineEdit1, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->masterPasswordLineEdit2, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(setDirty()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->forceRegexPlainTextEdit, SIGNAL(textChanged()), SLOT(setDirty()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(setDirty()));
  QObject::connect(ui->useDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->avoidAmbiguousCheckBox, SIGNAL(toggled(bool)), SLOT(setDirty()));
  QObject::connect(ui->forceLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->forceRegexCheckBox, SIGNAL(toggled(bool)), SLOT(updateValidator()));
  QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->masterPasswordLineEdit1, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->masterPasswordLineEdit2, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(updatePassword()));
  QObject::connect(ui->customCharactersPlainTextEdit, SIGNAL(textChanged()), SLOT(customCharacterSetChanged()));
  QObject::connect(ui->forceRegexPlainTextEdit, SIGNAL(textChanged()), SLOT(updateValidator()));
  QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
  QObject::connect(ui->useDigitsCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useExtrasCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useUpperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useLowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->useCustomCheckBox, SIGNAL(toggled(bool)), SLOT(customCharacterSetCheckBoxToggled(bool)));
  QObject::connect(ui->avoidAmbiguousCheckBox, SIGNAL(toggled(bool)), SLOT(updateUsedCharacters()));
  QObject::connect(ui->copyPasswordToClipboardPushButton, SIGNAL(pressed()), SLOT(copyPasswordToClipboard()));
  QObject::connect(ui->savePushButton, SIGNAL(pressed()), SLOT(saveCurrentSettings()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(stopPasswordGeneration()));
  QObject::connect(&mPassword, SIGNAL(generated()), SLOT(onPasswordGenerated()));
  QObject::connect(&mPassword, SIGNAL(generationAborted()), SLOT(onPasswordGenerationAborted()));
  QObject::connect(&mPassword, SIGNAL(generationStarted()), SLOT(onPasswordGenerationStarted()));
  QObject::connect(ui->actionNewDomain, SIGNAL(triggered(bool)), SLOT(newDomain()));
  QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(about()));
  QObject::connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(aboutQt()));
#ifdef QT_DEBUG
  ui->saltLineEdit->setEnabled(true);
  ui->domainLineEdit->setText("foo bar");
  ui->masterPasswordLineEdit1->setText("test");
  ui->masterPasswordLineEdit2->setText("test");
  ui->avoidAmbiguousCheckBox->setChecked(true);
#endif
  ui->domainLineEdit->selectAll();
  ui->processLabel->setMovie(&mLoaderIcon);
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  restoreSettings();
  updateUsedCharacters();
  updateValidator();
  setDirty(false);

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


void MainWindow::closeEvent(QCloseEvent *e)
{
  stopPasswordGeneration();
  int rc = (mParameterSetDirty)
      ? QMessageBox::question(
          this,
          tr("Save before exit?"),
          tr("Your parameters have changed. Do you want to save the changes before exiting?"),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes)
      : QMessageBox::NoButton;
  if (rc == QMessageBox::Yes)
    saveCurrentSettings();
  if (rc != QMessageBox::Cancel) {
    QMainWindow::closeEvent(e);
    e->accept();
  }
  else {
    e->ignore();
  }
  saveSettings();
}


void MainWindow::newDomain(void)
{
  DomainSettings domainSettings;
  ui->domainLineEdit->setText(QString());
  ui->domainLineEdit->setFocus();
  ui->masterPasswordLineEdit1->setText(QString());
  ui->masterPasswordLineEdit2->setText(QString());
  ui->useLowerCaseCheckBox->setChecked(domainSettings.useLowerCase);
  ui->useUpperCaseCheckBox->setChecked(domainSettings.useUpperCase);
  ui->useDigitsCheckBox->setChecked(domainSettings.useDigits);
  ui->useExtrasCheckBox->setChecked(domainSettings.useExtra);
  ui->iterationsSpinBox->setValue(domainSettings.iterations);
  ui->passwordLengthSpinBox->setValue(domainSettings.length);
  ui->saltLineEdit->setText(domainSettings.salt);
  ui->forceRegexPlainTextEdit->setPlainText(domainSettings.validatorRegEx.pattern());
  ui->forceRegexCheckBox->setChecked(domainSettings.forceValidation);
  updateValidator();
  updatePassword();

}

void MainWindow::setDirty(bool dirty)
{
  mParameterSetDirty = dirty;
  updateWindowTitle();
}


void MainWindow::onPasswordGenerationStarted(void)
{
  ui->copyPasswordToClipboardPushButton->setEnabled(false);
  ui->processLabel->show();
  ui->cancelPushButton->show();
  mLoaderIcon.start();
}

void MainWindow::updatePassword(void)
{
  bool validConfiguration = false;
  ui->statusBar->showMessage(QString());
  if (ui->customCharactersPlainTextEdit->toPlainText().count() > 0 &&
      !ui->masterPasswordLineEdit1->text().isEmpty() &&
      !ui->masterPasswordLineEdit2->text().isEmpty())
  {
    if (ui->masterPasswordLineEdit1->text() != ui->masterPasswordLineEdit2->text()) {
      ui->statusBar->showMessage(tr("Passwords do not match"), 2000);
    }
    else {
      validConfiguration = true;
      stopPasswordGeneration();
      generatePassword();
    }
  }
  if (!validConfiguration) {
    ui->generatedPasswordLineEdit->setText(QString());
    ui->hashPlainTextEdit->setPlainText(QString());
  }
}


void MainWindow::updateUsedCharacters(void)
{
  if (!ui->useCustomCheckBox->isChecked()) {
    stopPasswordGeneration();
    QString passwordCharacters;
    if (ui->useLowerCaseCheckBox->isChecked())
      passwordCharacters += Password::LowerChars;
    if (ui->useUpperCaseCheckBox->isChecked())
      passwordCharacters += ui->avoidAmbiguousCheckBox->isChecked() ? Password::UpperCharsNoAmbiguous : Password::UpperChars;
    if (ui->useDigitsCheckBox->isChecked())
      passwordCharacters += Password::Digits;
    if (ui->useExtrasCheckBox->isChecked())
      passwordCharacters += Password::ExtraChars;
    ui->customCharactersPlainTextEdit->blockSignals(true);
    ui->customCharactersPlainTextEdit->setPlainText(passwordCharacters);
    ui->customCharactersPlainTextEdit->blockSignals(false);
  }
  updateValidator();
}


void MainWindow::generatePassword(void)
{
  mPassword.generateAsync(
        PasswordParam(
          ui->domainLineEdit->text().toUtf8(),
          ui->saltLineEdit->text().toUtf8(),
          ui->masterPasswordLineEdit1->text().toUtf8(),
          ui->customCharactersPlainTextEdit->toPlainText(),
          ui->passwordLengthSpinBox->value(),
          ui->iterationsSpinBox->value()
          )
        );
}


void MainWindow::stopPasswordGeneration(void)
{
  if (mPassword.isRunning()) {
    mPassword.abortGeneration();
    mPassword.waitForFinished();
  }
}


void MainWindow::onPasswordGenerated(void)
{
  ui->processLabel->hide();
  ui->cancelPushButton->hide();
  ui->copyPasswordToClipboardPushButton->setEnabled(true);
  bool setKey = true;
  if (ui->forceRegexCheckBox->isChecked() && !mPassword.isValid())
    setKey = false;
  if (setKey) {
    ui->generatedPasswordLineEdit->setText(mPassword.key());
    ui->hashPlainTextEdit->setPlainText(mPassword.hexKey());
    ui->statusBar->showMessage(tr("generation time: %1 ms").arg(mPassword.elapsedSeconds(), 0, 'f', 4), 3000);
  }
  else {
    ui->statusBar->showMessage(tr("Password does not match regular expression. %1").arg(mAutoIncreaseIterations ? tr("Increasing iteration count.") : QString()));
    if (mAutoIncreaseIterations)
      ui->iterationsSpinBox->setValue( ui->iterationsSpinBox->value() + 1);
  }
}


void MainWindow::onPasswordGenerationAborted(void)
{
  onPasswordGenerated();
}


void MainWindow::copyPasswordToClipboard(void)
{
  QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
  ui->statusBar->showMessage(tr("Password copied to clipboard."));
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
      ui->useCustomCheckBox->setChecked(true);
    }
  }
  if (reallyToggle) {
    ui->useLowerCaseCheckBox->setEnabled(!checked);
    ui->useUpperCaseCheckBox->setEnabled(!checked);
    ui->useDigitsCheckBox->setEnabled(!checked);
    ui->useExtrasCheckBox->setEnabled(!checked);
    updateUsedCharacters();
    if (!checked)
      mCustomCharacterSetDirty = false;
  }
}


void MainWindow::customCharacterSetChanged(void)
{
  ui->useCustomCheckBox->setChecked(true);
  mCustomCharacterSetDirty = true;
}


void MainWindow::updateValidator(void)
{
  bool ok = false;
  QStringList canContain;
  QStringList mustContain;
  ui->forceLowerCaseCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceUpperCaseCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceDigitsCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  ui->forceExtrasCheckBox->setEnabled(!ui->forceRegexCheckBox->isChecked());
  if (ui->forceRegexCheckBox->isChecked()) {
    ok = mPassword.setValidator(QRegExp(ui->forceRegexPlainTextEdit->toPlainText()));
  }
  else {
    if (ui->useLowerCaseCheckBox->isChecked())
      canContain << "a-z";
    if (ui->useUpperCaseCheckBox->isChecked())
      canContain << "A-Z";
    if (ui->useDigitsCheckBox->isChecked())
      canContain << "0-9";
    if (ui->useExtrasCheckBox->isChecked())
      canContain << Password::ExtraChars;
    if (ui->forceLowerCaseCheckBox->isChecked())
      mustContain << "[a-z]";
    if (ui->forceUpperCaseCheckBox->isChecked())
      mustContain << "[A-Z]";
    if (ui->forceDigitsCheckBox->isChecked())
      mustContain << "[0-9]";
    if (ui->forceExtrasCheckBox->isChecked())
      mustContain << "[" + Password::ExtraChars + "]";
    ok = mPassword.setValidCharacters(canContain, mustContain);
    if (ok) {
      ui->forceRegexPlainTextEdit->blockSignals(true);
      ui->forceRegexPlainTextEdit->setPlainText(mPassword.validator().pattern());
      ui->forceRegexPlainTextEdit->blockSignals(false);
    }
  }
  if (ok) {
    updatePassword();
    ui->statusBar->showMessage(QString());
  }
  else {
    ui->statusBar->showMessage(tr("Bad regex: ") + mPassword.errorString());
  }
}


void MainWindow::saveCurrentSettings(void)
{
  DomainSettings domainSettings;
  domainSettings.domain = ui->domainLineEdit->text();
  domainSettings.username = ui->userLineEdit->text();
  domainSettings.useLowerCase = ui->useLowerCaseCheckBox->isChecked();
  domainSettings.useUpperCase = ui->useUpperCaseCheckBox->isChecked();
  domainSettings.useDigits = ui->useDigitsCheckBox->isChecked();
  domainSettings.useExtra = ui->useExtrasCheckBox->isChecked();
  domainSettings.useCustom = ui->useCustomCheckBox->isChecked();
  domainSettings.avoidAmbiguous = ui->avoidAmbiguousCheckBox->isChecked();
  domainSettings.customCharacters = ui->customCharactersPlainTextEdit->toPlainText();
  domainSettings.iterations = ui->iterationsSpinBox->value();
  domainSettings.salt = ui->saltLineEdit->text();
  domainSettings.length = ui->passwordLengthSpinBox->value();
  domainSettings.validatorRegEx = mPassword.validator();
  domainSettings.forceValidation = ui->forceRegexCheckBox->isChecked();
  saveDomainSettings(domainSettings);
  mSettings.sync();
  setDirty(false);
  ui->statusBar->showMessage(tr("Domain settings saved."), 3000);
}


void MainWindow::saveDomainSettings(DomainSettings &domainSettings)
{
  QStringListModel *model = reinterpret_cast<QStringListModel*>(ui->domainLineEdit->completer()->model());
  QStringList domains = model->stringList();
  if (!domains.contains(domainSettings.domain, Qt::CaseInsensitive)) {
    domains << domainSettings.domain;
    model->setStringList(domains);
    mDomains[domainSettings.domain] = domainSettings.toVariant();
  }
}


void MainWindow::saveSettings(void)
{
  mSettings.setValue("mainwindow/geometry", geometry());
  const QJsonDocument &json = QJsonDocument::fromVariant(mDomains);
  mSettings.setValue("data/domains", json.toJson(QJsonDocument::Compact));
  mSettings.sync();
}


void MainWindow::restoreSettings(void)
{
  restoreGeometry(mSettings.value("mainwindow/geometry").toByteArray());
  const QJsonDocument &json = QJsonDocument::fromJson(mSettings.value("data/domains").toByteArray());
  mDomains = json.toVariant().toMap();
  const QStringList &domains = json.object().keys();
  if (mCompleter) {
    QObject::disconnect(mCompleter, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
    delete mCompleter;
  }
  mCompleter = new QCompleter(domains);
  ui->domainLineEdit->setCompleter(mCompleter);
  QObject::connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(domainSelected(QString)));
}


void MainWindow::loadDomainSettings(const QString &domain)
{
  const QVariantMap &p = mDomains[domain].toMap();
  ui->domainLineEdit->setText(p["domain"].toString());
  ui->userLineEdit->setText(p["username"].toString());
  ui->useLowerCaseCheckBox->setChecked(p["useLowerCase"].toBool());
  ui->useUpperCaseCheckBox->setChecked(p["useUpperCase"].toBool());
  ui->useDigitsCheckBox->setChecked(p["useDigits"].toBool());
  ui->useExtrasCheckBox->setChecked(p["useExtra"].toBool());
  ui->useCustomCheckBox->setChecked(p["useCustom"].toBool());
  ui->avoidAmbiguousCheckBox->setChecked(p["avoidAmbiguous"].toBool());
  ui->customCharactersPlainTextEdit->blockSignals(true);
  ui->customCharactersPlainTextEdit->setPlainText(p["customCharacters"].toString());
  ui->customCharactersPlainTextEdit->blockSignals(false);
  ui->iterationsSpinBox->setValue(p["iterations"].toInt());
  ui->passwordLengthSpinBox->setValue(p["length"].toInt());
  ui->saltLineEdit->setText(p["salt"].toString());
  updateValidator();
  updatePassword();
}


void MainWindow::domainSelected(const QString &domain)
{
  loadDomainSettings(domain);
  setDirty(false);
}


void MainWindow::updateWindowTitle(void)
{
  setWindowTitle(AppName + " " + AppVersion + (mParameterSetDirty ? "*" : ""));
}


void MainWindow::about(void)
{
  QMessageBox::about(
        this, tr("About %1 %2").arg(AppName).arg(AppVersion),
        tr("<p><b>%1</b> is a domain specific password generator. "
           "See <a href=\"%2\" title=\"%1 project homepage\">%2</a> for more info.</p>"
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
           "<p>No animals were harmed during the development of <i>%1</i>. "
           "The software was programmed with CO<sub>2</sub> neutrality in mind and without the use of genetic engineering. "
           "It's vegan, free of antibiotics and hypoallergenic.</p>"
           "<p>Copyright &copy; 2015 %3 &lt;%4&gt;, Heise Medien GmbH &amp; Co. KG.</p>"
           )
        .arg(AppName).arg(AppUrl).arg(AppAuthor).arg(AppAuthorMail));
}


void MainWindow::aboutQt(void)
{
  QMessageBox::aboutQt(this);
}
