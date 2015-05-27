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

#include "password.h"

#include "cryptopp562/sha.h"
#include "cryptopp562/cryptlib.h"
#include "cryptopp562/hmac.h"
#include "cryptopp562/integer.h"

#include "bigint/bigInt.h"

#include <QElapsedTimer>
#include <QMutexLocker>
#include <QtConcurrent>

#ifdef QT_DEBUG
#include <QtDebug>
#endif

class PasswordPrivate {
public:
  PasswordPrivate(void)
    : elapsed(0)
    , abort(false)
  { /* ... */ }
  ~PasswordPrivate()
  { /* ... */ }
  QByteArray derivedKey;
  QString key;
  QString hexKey;
  QRegExp validator;
  qreal elapsed;
  bool abort;
  QMutex abortMutex;
  QFuture<void> future;
};


const QString PasswordParamBase::LowerChars = "abcdefghijklmnopqrstuvwxyz";
const QString PasswordParamBase::UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString PasswordParamBase::UpperCharsNoAmbiguous = "ABCDEFGHJKLMNPQRTUVWXYZ";
const QString PasswordParamBase::Digits = "0123456789";
const QString PasswordParamBase::ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";


Password::Password(QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{ /* ... */ }


Password::~Password()
{ /* ... */ }


bool Password::generate(const PasswordParam &p)
{
  Q_D(Password);
  d->abortMutex.lock();
  d->abort = false;
  d->abortMutex.unlock();
  emit generationStarted();
  const QByteArray &pwd = p.domain + p.masterPwd;
  const int nChars = p.availableChars.count();

  byte derivedBuf[CryptoPP::SHA512::DIGESTSIZE];
  byte *derived = derivedBuf;
  size_t derivedLen = CryptoPP::SHA512::DIGESTSIZE;
  const byte *password = reinterpret_cast<const byte*>(pwd.data());
  size_t passwordLen = pwd.count();
  const byte *saltPtr = reinterpret_cast<const byte*>(p.salt.data());
  size_t saltLen = p.salt.count();
  CryptoPP::HMAC<CryptoPP::SHA512> hmac(password, passwordLen);
  CryptoPP::SecByteBlock buffer(hmac.DigestSize());

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  unsigned int i = 1;
  bool completed = true;
  while (derivedLen > 0) {
    hmac.Update(saltPtr, saltLen);
    for (unsigned int j = 0; j < 4; ++j) {
      byte b = byte(i >> ((3 - j) * 8));
      hmac.Update(&b, 1);
    }
    hmac.Final(buffer);
    const size_t segmentLen = qMin(derivedLen, buffer.size());
#ifdef WIN32
    memcpy_s(derived, derivedLen, buffer, segmentLen);
#else
    memcpy(derived, buffer, segmentLen);
#endif
    for (unsigned int j = 1; j < p.iterations; ++j) {
      QMutexLocker locker(&d->abortMutex);
      if (d->abort) {
        completed = false;
        emit generationAborted();
        break;
      }
      hmac.CalculateDigest(buffer, buffer, buffer.size());
      xorbuf(derived, buffer, segmentLen);
    }
    derived += segmentLen;
    derivedLen -= segmentLen;
    ++i;
  }

  bool success = false;
  if (completed) {
    d->elapsed = 1e-6 * elapsedTimer.nsecsElapsed();
    d->derivedKey = QByteArray(reinterpret_cast<char*>(derivedBuf), CryptoPP::SHA512::DIGESTSIZE);
    d->hexKey = d->derivedKey.toHex();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(d->hexKey.toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    d->key = QString();
    int n = p.passwordLength;
    while (v > Zero && n-- > 0) {
      BigInt::Rossi mod = v % Modulus;
      d->key += p.availableChars.at(mod.toUlong());
      v = v / Modulus;
    }
    success = true;
  }
  if (success)
    emit generated();
  return success;
}


void Password::generateAsync(const PasswordParam &p)
{
  Q_D(Password);
  d->abort = false;
  d->future = QtConcurrent::run(this, &Password::generate, p);
}


void Password::abortGeneration(void)
{
  Q_D(Password);
  d->abortMutex.lock();
  d->abort = true;
  d->abortMutex.unlock();
}


bool Password::setValidator(const QRegExp &re)
{
  Q_D(Password);
  bool valid = re.isValid();
  if (valid)
    d->validator = re;
  return valid;
}


const QRegExp &Password::validator(void) const
{
  return d_ptr->validator;
}


bool Password::setValidCharacters(const QStringList &canContain, const QStringList &mustContain)
{
  Q_D(Password);
  QString reStr("^");
  foreach(QString s, mustContain) {
    reStr += "(?=.*" + QRegExp::escape(s) + ")";
  }
  if (!canContain.isEmpty()) {
    reStr += "[";
    foreach(QString s, canContain) {
      reStr += "(?=.*" + QRegExp::escape(s) + ")";
    }
    reStr += "]+";
  }
  reStr += "$";
  QRegExp re(reStr, Qt::CaseSensitive, QRegExp::RegExp2);
  if (re.isValid()) {
    d->validator = re;
    return true;
  }
  return false;
}


bool Password::isValid(void) const
{
  return d_ptr->validator.exactMatch(d_ptr->key);
}


const QByteArray &Password::derivedKey(void) const
{
  return d_ptr->derivedKey;
}


const QString &Password::key(void) const
{
  return d_ptr->key;
}


const QString &Password::hexKey(void) const
{
  return d_ptr->hexKey;
}


qreal Password::elapsedSeconds(void) const
{
  return d_ptr->elapsed;
}


bool Password::isRunning(void) const
{
  return d_ptr->future.isRunning();
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}


QString Password::errorString(void) const
{
  return d_ptr->validator.errorString();
}
