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

#include "pbkdf2.h"
#include "util.h"


#include <QElapsedTimer>
#include <QMessageAuthenticationCode>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QtDebug>
#include <QChar>


class PBKDF2Private {
public:
  PBKDF2Private(void)
    : elapsed(0)
    , abort(false)
  { /* ... */ }
  ~PBKDF2Private()
  {
    SecureErase(derivedKey);
    SecureErase(hexKey);
  }
  QByteArray salt;
  QByteArray derivedKey;
  QString hexKey;
  qreal elapsed;
  bool abort;
  QMutex abortMutex;
  QFuture<void> future;
};



PBKDF2::PBKDF2(QObject *parent)
  : QObject(parent)
  , d_ptr(new PBKDF2Private)
{ /* ... */ }


PBKDF2::~PBKDF2()
{ /* ... */ }


auto xorbuf = [](QByteArray &dst, const QByteArray &src) {
  Q_ASSERT_X(dst.size() == src.size(), "xorbuf()", "size of source and destination buffers must be equal");
  for (int i = 0; i < dst.size(); ++i)
    dst[i] = dst.at(i) ^ src.at(i);
};


void PBKDF2::generate(const SecureByteArray &pwd, const QByteArray &salt, int iterations, QCryptographicHash::Algorithm algorithm)
{
  Q_D(PBKDF2);

  qDebug() << "PBKDF2::generate()";

  d->abortMutex.lock();
  d->abort = false;
  d->abortMutex.unlock();

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  emit generationStarted();

  static const char INT_32_BE1[4] = { 0, 0, 0, 1 };
  QMessageAuthenticationCode hmac(algorithm);
  hmac.setKey(pwd);
  hmac.addData(salt + QByteArray(INT_32_BE1, 4));

  QByteArray buffer = hmac.result();
  d->derivedKey = buffer;

  for (int j = 1; j < iterations; ++j) {
    QMutexLocker locker(&d->abortMutex);
    if (d->abort) {
      emit generationAborted();
      break;
    }
    hmac.reset();
    hmac.addData(buffer);
    buffer = hmac.result();
    xorbuf(d->derivedKey, buffer);
  }

  d->hexKey = QString::fromLatin1(d->derivedKey.toHex());
  d->elapsed = 1e-9 * elapsedTimer.nsecsElapsed();
}


void PBKDF2::generateAsync(const SecureByteArray &pwd, const QByteArray &salt, int iterations, QCryptographicHash::Algorithm algorithm)
{
  Q_D(PBKDF2);
  d->abort = false;
  d->future = QtConcurrent::run(this, &PBKDF2::generate, pwd, salt, iterations, algorithm);
}


void PBKDF2::abortGeneration(void)
{
  Q_D(PBKDF2);
  d->abortMutex.lock();
  d->abort = true;
  d->abortMutex.unlock();
}


const QString &PBKDF2::hexKey(void) const
{
  return d_ptr->hexKey;
}


QByteArray PBKDF2::derivedKey(int size) const
{
  Q_ASSERT_X(size <= d_ptr->derivedKey.size(), "Password::derivedKey()", "size must be <= key size");
  if (size < 0)
    return d_ptr->derivedKey;
  else
    return QByteArray(d_ptr->derivedKey.constData(), size);
}


qreal PBKDF2::elapsedSeconds(void) const
{
  return d_ptr->elapsed;
}


bool PBKDF2::isRunning(void) const
{
  return d_ptr->future.isRunning();
}


bool PBKDF2::isAborted(void) const
{
  return d_ptr->abort;
}


void PBKDF2::waitForFinished(void)
{
  Q_D(PBKDF2);
  d->future.waitForFinished();
}
