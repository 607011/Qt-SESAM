#include "passwordgenerator.h"

#include "cryptopp562/pwdbased.h"
#include "cryptopp562/sha.h"

#include "bigint/bigInt.h"

#include <QString>
#include <QElapsedTimer>

PasswordGenerator::PasswordGenerator(QObject *parent)
  : QObject(parent)
{
  // ...
}


bool PasswordGenerator::generate(
    const QByteArray &domain,
    const QByteArray &salt,
    const QByteArray &masterPwd,
    const QString &availableChars,
    const int passwordLength,
    int iterations,
    bool &doQuit,
    qreal &elapsed,
    QString &key,
    QByteArray &hexKey
    )
{
  const QByteArray &pwd = domain + masterPwd;
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
  const int nChars = availableChars.count();
  byte *derived = new byte[CryptoPP::SHA512::DIGESTSIZE];
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
    elapsed = 1e-6 * elapsedTimer.nsecsElapsed();
    const QByteArray &derivedKeyBuf = QByteArray(reinterpret_cast<char*>(derived), CryptoPP::SHA512::DIGESTSIZE);
    hexKey = derivedKeyBuf.toHex();
    const QString strModulus = QString("%1").arg(nChars);
    BigInt::Rossi v(QString(hexKey).toStdString(), BigInt::HEX_DIGIT);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    static const BigInt::Rossi Zero(0);
    key = QString();
    int n = passwordLength;
    while (v > Zero && n-- > 0) {
      BigInt::Rossi mod = v % Modulus;
      const QString &chrs = availableChars;
      if (chrs.size() == nChars)
        key += chrs.at(mod.toUlong());
      v = v / Modulus;
    }
    success = true;
  }
  delete[] derived;
  return success;
}
