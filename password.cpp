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
  QByteArray salt;
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


auto xorbuf = [](QByteArray &dst, const QByteArray &src) {
  Q_ASSERT_X(dst.size() == src.size(), "xorbuf()", "size of source and destination buffers must be equal");
  for (int i = 0; i < dst.size(); ++i)
    dst[i] = dst.at(i) ^ src.at(i);
};


bool Password::generate(const PasswordParam &p)
{
  Q_D(Password);
  d->abortMutex.lock();
  d->abort = false;
  d->abortMutex.unlock();

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  emit generationStarted();

  const QByteArray &pwd = p.domain + p.masterPwd;
  const int nChars = p.availableChars.count();

  d->salt = p.salt;

  static const char INT_32_BE1[4] = { 0, 0, 0, 1 };

  QMessageAuthenticationCode hmac(QCryptographicHash::Sha512);
  hmac.setKey(pwd);
  hmac.addData(p.salt);
  hmac.addData(INT_32_BE1, 4);

  QByteArray buffer = hmac.result();
  d->derivedKey = buffer;

  bool completed = true;
  for (unsigned int j = 1; j < p.iterations; ++j) {
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
  if (completed) {
    d->elapsed = 1e-6 * elapsedTimer.nsecsElapsed();
    d->hexKey = d->derivedKey.toHex();
    d->key.clear();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(d->hexKey.toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    int n = p.passwordLength;
    while (v > Zero && n-- > 0) {
      const BigInt::Rossi &mod = v % Modulus;
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
  if (!mustContain.isEmpty()) {
    reStr += "(?=.*[" + QRegExp::escape(mustContain.join(QString())) + "])";
  }
  if (!canContain.isEmpty()) {
    reStr += "[" + QRegExp::escape(canContain.join(QString())) + "]+";
  }
  reStr += "$";
  QRegExp re(reStr, Qt::CaseSensitive, QRegExp::RegExp2);
  qDebug() << re.pattern() << "isValid() =" << re.isValid();
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


const QString &Password::key(void) const
{
  return d_ptr->key;
}


const QString &Password::hexKey(void) const
{
  return d_ptr->hexKey;
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


const QByteArray &Password::salt(void) const
{
  return d_ptr->salt;
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}


QString Password::errorString(void) const
{
  return d_ptr->validator.errorString();
}
