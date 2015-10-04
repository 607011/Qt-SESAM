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
#include <QtConcurrent>
#include <QFuture>

#include "securebytearray.h"
#include "securestring.h"
#include "presets.h"
#include "password.h"
#include "pbkdf2.h"
#include "util.h"

#include "3rdparty/bigint/bigInt.h"

class PasswordPrivate {
public:
  PasswordPrivate(void)
  { /* ... */ }
  ~PasswordPrivate()
  { /* ... */ }
  PBKDF2 pbkdf2;
  SecureString password;
  DomainSettings domainSettings;
  QFuture<void> future;
};


const QString Password::LowerChars = QString("abcdefghijklmnopqrstuvwxyz").toUtf8();
const QString Password::UpperChars = QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ").toUtf8();
const QString Password::UpperCharsNoAmbiguous = QString("ABCDEFGHJKLMNPQRTUVWXYZ").toUtf8();
const QString Password::Digits = QString("0123456789").toUtf8();
// !"$%&?!<>()[]{}\|/~`´#'=-_+*~.,;:^°
const QString Password::ExtraChars = QString("!\\|\"$%/&?!<>()[]{}~`´#'=-_+*~.,;:^°").toUtf8();
const QString Password::AllChars = Password::LowerChars + Password::UpperChars + Password::Digits + Password::ExtraChars;


Password::Password(const DomainSettings &ds, QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{
  d_ptr->domainSettings = ds;
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationStarted()), SIGNAL(generationStarted()));
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationAborted()), SIGNAL(generationAborted()));
}

Password::~Password()
{
  // ...
}


void Password::setDomainSettings(const DomainSettings &ds)
{
  d_ptr->domainSettings = ds;
}


void Password::generate(const SecureByteArray &masterPassword)
{
  Q_D(Password);

  const SecureByteArray &pwd =
      d->domainSettings.domainName.toUtf8() +
      d->domainSettings.userName.toUtf8() +
      masterPassword;

  d->pbkdf2.generate(pwd, QByteArray::fromBase64(d->domainSettings.salt_base64.toUtf8()), d->domainSettings.iterations, QCryptographicHash::Sha512);

  remixed();
  emit generated();
}


const SecureString &Password::remixed(void)
{
  Q_D(Password);
  d->password.clear();
  static const BigInt::Rossi Zero(0);
  BigInt::Rossi v(d->pbkdf2.hexKey().toStdString(), BigInt::HEX_DIGIT);
  if (d->domainSettings.passwordTemplate.isEmpty() && !d->domainSettings.usedCharacters.isEmpty()) { // v2 method
    const int nChars = d->domainSettings.usedCharacters.count();
    const QString strModulus = QString("%1").arg(nChars);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    int n = d->domainSettings.passwordLength;
    while (v > Zero && n-- > 0) {
      const BigInt::Rossi &mod = v % Modulus;
      d->password += d->domainSettings.usedCharacters.at(int(mod.toUlong()));
      v = v / Modulus;
    }
  }
  else { // v3 method
    const QByteArray &templ = d->domainSettings.passwordTemplate;
    int n = 0;
    while (v > Zero && n < templ.length()) {
      const char m = templ.at(n);
      QString charSet;
      switch (m) {
      case 'x':
        charSet = d->domainSettings.usedCharacters;
        break;
      case 'o':
        charSet = d->domainSettings.extraCharacters;
        break;
      case 'a':
      case 'A':
      case 'n':
        charSet = Preset::charSetFor(m);
        break;
      default:
        qWarning() << "Invalid template character:" << m;
        break;
      }
      if (!charSet.isEmpty()) {
        const QString strModulus = QString("%1").arg(charSet.count());
        const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
        const BigInt::Rossi &mod = v % Modulus;
        d->password += charSet.at(int(mod.toUlong()));
        v = v / Modulus;
      }
      ++n;
    }
  }
  return d->password;
}


void Password::generateAsync(const SecureByteArray &masterPassword, const DomainSettings &domainSettings)
{
  Q_D(Password);
  if (!domainSettings.isEmpty())
    d->domainSettings = domainSettings;
  d->future = QtConcurrent::run(this, &Password::generate, masterPassword);
}


bool Password::isRunning(void) const
{
  return d_ptr->future.isRunning();
}


bool Password::isAborted(void) const
{
  return d_ptr->pbkdf2.isAborted();
}


qreal Password::elapsedSeconds(void) const
{
  return d_ptr->pbkdf2.elapsedSeconds();
}


void Password::abortGeneration(void)
{
  d_ptr->pbkdf2.abortGeneration();
}


SecureString Password::operator()(void) const
{
  return password();
}


const SecureString &Password::password(void) const
{
  return d_ptr->password;
}


const SecureString &Password::hexKey(void) const
{
  return d_ptr->pbkdf2.hexKey();
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}
