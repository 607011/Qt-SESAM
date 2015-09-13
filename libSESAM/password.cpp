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
#include "password.h"
#include "pbkdf2.h"
#include "util.h"

#include "3rdparty/bigint/bigInt.h"

class PasswordPrivate {
public:
  PasswordPrivate(void)
  {
    // ...
  }
  ~PasswordPrivate()
  {
    SecureErase(key);
  }
  PBKDF2 pbkdf2;
  QString key;
  DomainSettings domainSettings;
  QFuture<void> future;
};


const QString Password::LowerChars = QString("abcdefghijklmnopqrstuvwxyz").toUtf8();
const QString Password::UpperChars = QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ").toUtf8();
const QString Password::UpperCharsNoAmbiguous = QString("ABCDEFGHJKLMNPQRTUVWXYZ").toUtf8();
const QString Password::Digits = QString("0123456789").toUtf8();
const QString Password::ExtraChars = QString("#!\"§$%&/()[]{}=-_+*<>;:.").toUtf8();
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

  const QByteArray &pwd =
      d->domainSettings.domainName.toUtf8() +
      d->domainSettings.userName.toUtf8() +
      masterPassword;

  d->pbkdf2.generate(pwd, QByteArray::fromBase64(d->domainSettings.salt_base64.toUtf8()), d->domainSettings.iterations, QCryptographicHash::Sha512);

  const int nChars = d->domainSettings.usedCharacters.count();
  if (nChars > 0) {
    d->key.clear();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(d->pbkdf2.hexKey().toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    int n = d->domainSettings.length;
    while (v > Zero && n-- > 0) {
      const BigInt::Rossi &mod = v % Modulus;
      d->key += d->domainSettings.usedCharacters.at(mod.toUlong());
      v = v / Modulus;
    }
    emit generated();
  }
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


const QString &Password::key(void) const
{
  return d_ptr->key;
}


const QString &Password::hexKey(void) const
{
  return d_ptr->pbkdf2.hexKey();
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}
