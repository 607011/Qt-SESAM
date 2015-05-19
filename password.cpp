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
  QString key;
  QString hexKey;
  QRegExp validator;
  qreal elapsed;
  bool abort;
  QMutex abortMutex;
  QFuture<void> future;
};



const QString Password::LowerChars = "abcdefghijklmnopqrstuvwxyz";
const QString Password::UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString Password::UpperCharsNoAmbiguous = "ABCDEFGHJKLMNPQRTUVWXYZ";
const QString Password::Digits = "0123456789";
const QString Password::ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";


Password::Password(QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{ /* ... */ }


Password::~Password()
{ /* ... */ }


bool Password::generate(const PasswordParam &p)
{
  Q_D(Password);
  d->abort = false;
  qDebug() << "Password::generate() has just started.";
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
  while (derivedLen > 0) {
    hmac.Update(saltPtr, saltLen);
    for (unsigned int j = 0; j < 4; ++j) {
      byte b = byte(i >> ((3 - j) * 8));
      hmac.Update(&b, 1);
    }
    hmac.Final(buffer);
    const size_t segmentLen = qMin(derivedLen, buffer.size());
    memcpy(derived, buffer, segmentLen);
    for (unsigned int j = 1; j < p.iterations; ++j) {
      QMutexLocker locker(&d->abortMutex);
      if (d->abort) {
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

  d->abortMutex.lock();
  bool completed = !d->abort;
  d->abortMutex.unlock();
  bool success = false;
  qDebug() << "Password::generate(): computation" << (completed ? "has finished" : "was aborted");
  if (completed) {
    d->elapsed = 1e-6 * elapsedTimer.nsecsElapsed();
    const QByteArray &derivedKeyBuf = QByteArray(reinterpret_cast<char*>(derivedBuf), CryptoPP::SHA512::DIGESTSIZE);
    d->hexKey = derivedKeyBuf.toHex();
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
  qDebug() << "Password::generate() is about exit ...";
  if (success)
    emit generated();
  return success;
}


void Password::generateAsync(const PasswordParam &p)
{
  Q_D(Password);
  qDebug() << "Password::generateAsync() ...";
  d->abort = false;
  d->future = QtConcurrent::run(this, &Password::generate, p);
}


void Password::abortGeneration(void)
{
  Q_D(Password);
  d->abortMutex.lock();
  qDebug() << "Password::abortGeneration()";
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
  qDebug() << re << re.isValid();
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
