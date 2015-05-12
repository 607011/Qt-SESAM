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

#include "cryptopp562/pwdbased.h"
#include "cryptopp562/sha.h"

#include "bigint/bigInt.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mLoaderIcon(":/images/loader.gif")
    , mElapsed(0)
{
    ui->setupUi(this);
    QObject::connect(ui->domainLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
    QObject::connect(ui->masterPasswordLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
    QObject::connect(ui->saltLineEdit, SIGNAL(textChanged(QString)), SLOT(updatePassword()));
    QObject::connect(ui->passwordLengthSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
    QObject::connect(ui->iterationsSpinBox, SIGNAL(valueChanged(int)), SLOT(updatePassword()));
    QObject::connect(ui->digitsCheckBox, SIGNAL(toggled(bool)), SLOT(updatePassword()));
    QObject::connect(ui->extrasCheckBox, SIGNAL(toggled(bool)), SLOT(updatePassword()));
    QObject::connect(ui->upperCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updatePassword()));
    QObject::connect(ui->lowerCaseCheckBox, SIGNAL(toggled(bool)), SLOT(updatePassword()));
    QObject::connect(ui->copyPasswordToClipboardPushButton, SIGNAL(pressed()), SLOT(copyPasswordToClipboard()));
    QObject::connect(this, SIGNAL(passwordGenerated(QString)), SLOT(onPasswordGenerated(QString)));
    ui->domainLineEdit->selectAll();
    ui->processLabel->setMovie(&mLoaderIcon);
    ui->processLabel->hide();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updatePassword(void)
{
    mPasswordCharacters = QString();
    static const QString UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const QString LowerChars = "abcdefghijklmnopqrstuvwxyz";
    static const QString Digits = "0123456789";
    static const QString ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";
    if (ui->extrasCheckBox->isChecked())
        mPasswordCharacters += ExtraChars;
    if (ui->lowerCaseCheckBox->isChecked())
        mPasswordCharacters += LowerChars;
    if (ui->upperCaseCheckBox->isChecked())
        mPasswordCharacters += UpperChars;
    if (ui->digitsCheckBox->isChecked())
        mPasswordCharacters += Digits;
    if (mPasswordCharacters.count() > 0) {
        if (!ui->domainLineEdit->text().isEmpty() && !ui->saltLineEdit->text().isEmpty() && !ui->masterPasswordLineEdit->text().isEmpty()) {
            ui->copyPasswordToClipboardPushButton->setEnabled(false);
            ui->processLabel->show();
            mLoaderIcon.start();
            mPasswordGeneratorFuture.cancel();
            mPasswordGeneratorFuture = QtConcurrent::run(this, &MainWindow::generatePassword);
        }
    }
    else {
        ui->generatedPasswordLineEdit->setText(tr("<invalid>"));
        ui->statusBar->showMessage("");
    }
}


void MainWindow::copyPasswordToClipboard(void)
{
    QApplication::clipboard()->setText(ui->generatedPasswordLineEdit->text());
}


void MainWindow::onPasswordGenerated(QString key)
{
    ui->statusBar->showMessage(tr("generation time: %1 ms").arg(mElapsed, 0, 'f', 4), 3000);
    ui->generatedPasswordLineEdit->setText(key);
    ui->processLabel->hide();
    ui->copyPasswordToClipboardPushButton->setEnabled(true);
    mLoaderIcon.stop();
}


void MainWindow::generatePassword(void)
{
    const QByteArray &domain = ui->domainLineEdit->text().toUtf8();
    const QByteArray &salt = ui->saltLineEdit->text().toUtf8();
    const QByteArray &masterPwd = ui->masterPasswordLineEdit->text().toUtf8();
    const QByteArray &pwd = domain + masterPwd;
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
    const int nChars = mPasswordCharacters.count();
    byte *derived = new byte[nChars];
    mElapsedTimer.start();
    pbkdf2.DeriveKey(
                derived,
                nChars,
                0,
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
    BigInt::Rossi v(hexKey.toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    QString key;
    int n = ui->passwordLengthSpinBox->value();
    while (v > Zero && n-- > 0) {
        BigInt::Rossi mod = v % Modulus;
        key += mPasswordCharacters.at(mod.toUlong());
        v = v / Modulus;
    }
    delete[] derived;
    emit passwordGenerated(key);
}
