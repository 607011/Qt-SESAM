#include "password.h"

#include "cryptopp562/pwdbased.h"
#include "cryptopp562/sha.h"

#include "bigint/bigInt.h"

#include <QElapsedTimer>
#ifdef QT_DEBUG
#include <QtDebug>
#endif

class PasswordPrivate {
public:
  PasswordPrivate(void)
{ /* ... */ }
  ~PasswordPrivate()
  { /* ... */ }
  QString key;
  QString hexKey;
  QRegExp validator;
};



const QString Password::LowerChars = "abcdefghijklmnopqrstuvwxyz";
const QString Password::UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString Password::UpperCharsNoAmbiguous = "ABCDEFGHJKLMNPQRTUVWXYZ";
const QString Password::Digits = "0123456789";
const QString Password::ExtraChars = "#!\"§$%&/()[]{}=-_+*<>;:.";


Password::Password(QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{
  // ...
}


Password::~Password()
{ /* ... */ }


bool Password::generate(
    const QByteArray &domain,
    const QByteArray &salt,
    const QByteArray &masterPwd,
    const QString &availableChars,
    const int passwordLength,
    const int iterations,
    bool &doQuit,
    qreal *elapsed
    )
{
  Q_D(Password);
  const QByteArray &pwd = domain + masterPwd;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
  const int nChars = availableChars.count();
  byte derived[CryptoPP::SHA512::DIGESTSIZE];
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();
  unsigned int nIter = pbkdf2.DeriveKey(
        derived,
        CryptoPP::SHA512::DIGESTSIZE,
        reinterpret_cast<const byte*>(pwd.data()),
        pwd.count(),
        reinterpret_cast<const byte*>(salt.data()),
        salt.count(),
        iterations,
        doQuit);
  bool success = false;
  if (nIter > 0) {
    *elapsed = 1e-6 * elapsedTimer.nsecsElapsed();
    const QByteArray &derivedKeyBuf = QByteArray(reinterpret_cast<char*>(derived), CryptoPP::SHA512::DIGESTSIZE);
    d->hexKey = derivedKeyBuf.toHex();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(d->hexKey.toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    d->key = QString();
    int n = passwordLength;
    while (v > Zero && n-- > 0) {
      BigInt::Rossi mod = v % Modulus;
      d->key += availableChars.at(mod.toUlong());
      v = v / Modulus;
    }
    success = true;
  }
  qDebug() << "Password::generate(): exit code" << success;
  return success;
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


QString Password::errorString(void) const
{
  return d_ptr->validator.errorString();
}
