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

#include <cstring>

#include "password.h"
#include "util.h"
#include "global.h"

#include "bigint/bigInt.h"

#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QtDebug>
#include <QChar>

class PasswordPrivate {
public:
  PasswordPrivate(void)
    : elapsed(0)
    , abort(false)
  { /* ... */ }
  ~PasswordPrivate()
  {
    SecureErase(derivedKey);
    SecureErase(key);
    SecureErase(hexKey);
  }
  QByteArray derivedKey;
  QString key;
  QString hexKey;
  qreal elapsed;
  bool abort;
  QMutex abortMutex;
  QFuture<void> future;
  DomainSettings domainSettings;
};

const QString Password::LowerChars = QString("abcdefghijklmnopqrstuvwxyz").toUtf8();
const QString Password::UpperChars = QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ").toUtf8();
const QString Password::UpperCharsNoAmbiguous = QString("ABCDEFGHJKLMNPQRTUVWXYZ").toUtf8();
const QString Password::Digits = QString("0123456789").toUtf8();
const QString Password::ExtraChars = QString("#!\"§$%&/()[]{}=-_+*<>;:.").toUtf8();
const QString Password::AllChars = Password::LowerChars + Password::UpperChars + Password::Digits + Password::ExtraChars;


Password::Password(QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{ /* ... */ }


Password::~Password()
{ /* ... */ }


auto xorbuf = [](QByteArray &dst, const QByteArray &src) {
  Q_ASSERT_X(dst.size() == src.size(), "xorbuf()", "size of source and destination buffers must be equal");
  for (int i = 0; i < dst.size(); ++i)
    dst[i] = dst.at(i) ^ src.at(i);
};


bool Password::generate(const QByteArray &masterKey)
{
  Q_D(Password);
  d->abortMutex.lock();
  d->abort = false;
  d->abortMutex.unlock();

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  emit generationStarted();

  const QByteArray &pwd =
      d->domainSettings.domainName.toUtf8() +
      d->domainSettings.userName.toUtf8() +
      masterKey;

  QByteArray salt = QByteArray::fromBase64(d->domainSettings.salt_base64.toUtf8());
  static const char INT_32_BE1[4] = { 0, 0, 0, 1 };
  QMessageAuthenticationCode hmac(QCryptographicHash::Sha512);
  hmac.setKey(pwd);
  hmac.addData(salt + QByteArray(INT_32_BE1, 4));

  QByteArray buffer = hmac.result();
  d->derivedKey = buffer;

  bool completed = true;
  for (int j = 1; j < d->domainSettings.iterations; ++j) {
    QMutexLocker locker(&d->abortMutex);
    if (d->abort) {
      completed = false;
      emit generationAborted();
      break;
    }
    hmac.reset();
    hmac.addData(buffer);
    buffer = hmac.result();
    xorbuf(d->derivedKey, buffer);
  }

  bool success = false;
  const int nChars = d->domainSettings.usedCharacters.count();
  if (completed && nChars > 0) {
    d->elapsed = 1e-9 * elapsedTimer.nsecsElapsed();
    d->hexKey = d->derivedKey.toHex();
    d->key.clear();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(d->hexKey.toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    int n = d->domainSettings.length;
    while (v > Zero && n-- > 0) {
      const BigInt::Rossi &mod = v % Modulus;
      d->key += d->domainSettings.usedCharacters.at(mod.toUlong());
      v = v / Modulus;
    }
    success = true;
  }
  if (success)
    emit generated();
  return success;
}


void Password::generateAsync(const QByteArray &masterKey)
{
  Q_D(Password);
  d->abort = false;
  d->future = QtConcurrent::run(this, &Password::generate, masterKey);
}


void Password::abortGeneration(void)
{
  Q_D(Password);
  d->abortMutex.lock();
  d->abort = true;
  d->abortMutex.unlock();
}


void Password::setDomainSettings(const DomainSettings &ds)
{
  Q_D(Password);
  d->domainSettings = ds;
}


const QString &Password::key(void) const
{
  return d_ptr->key;
}


const QString &Password::hexKey(void) const
{
  return d_ptr->hexKey;
}


QByteArray Password::derivedKey(int size) const
{
  Q_ASSERT_X(size <= d_ptr->derivedKey.size(), "Password::derivedKey()", "size must be <= key size");
  return size < 0
      ? d_ptr->derivedKey
      : QByteArray(d_ptr->derivedKey.constData(), size);
}


void Password::extractAESKey(char *const aesKey, int size)
{
  Q_D(Password);
  Q_ASSERT(aesKey != nullptr);
  Q_ASSERT(size > 0);
  Q_ASSERT(size % 16 == 0);
  Q_ASSERT(size <= SHA512_DIGEST_SIZE);
#ifdef WIN32
    memcpy_s(aesKey, size, d->derivedKey.data(), size);
#else
    memcpy(aesKey, d->derivedKey.data(), size);
#endif
}


qreal Password::elapsedSeconds(void) const
{
  return d_ptr->elapsed;
}


bool Password::isRunning(void) const
{
  return d_ptr->future.isRunning();
}


bool Password::isAborted(void) const
{
  return d_ptr->abort;
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}
