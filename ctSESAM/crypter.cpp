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
#include "3rdparty/cryptopp562/sha.h"
#include "3rdparty/cryptopp562/aes.h"
#include "3rdparty/cryptopp562/ccm.h"
#include "3rdparty/cryptopp562/filters.h"
#include "3rdparty/cryptopp562/misc.h"
#include "securebytearray.h"
#include "crypter.h"
#include "password.h"
#include "util.h"


static const unsigned char IV[16] = {0xb5, 0x4f, 0xcf, 0xb0, 0x88, 0x09, 0x55, 0xe5, 0xbf, 0x79, 0xaf, 0x37, 0x71, 0x1c, 0x28, 0xb6};
static const int CryptSaltLength = 32;
static const int AESKeySize = 256 / 8;

QByteArray Crypter::encode(const QString &masterPassword, const QByteArray &baPlain, bool compress, int iterations, int *errCode, QString *errMsg)
{
  if (errCode != nullptr)
    *errCode = NoCryptError;
  QByteArray _baPlain = compress ? qCompress(baPlain, 9) : baPlain;

  const QByteArray &salt = Password::randomSalt(CryptSaltLength);
  Password cryptPassword;
  cryptPassword.setIterations(iterations);
  cryptPassword.setSalt(salt);
  cryptPassword.generate(masterPassword.toUtf8());
  SecureByteArray key = cryptPassword.derivedKey(AESKeySize);

  std::string plain(_baPlain.constData(), _baPlain.length());
  std::string cipher;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), IV);
    CryptoPP::StringSource s(
          plain,
          true,
          new CryptoPP::StreamTransformationFilter(
            enc,
            new CryptoPP::StringSink(cipher),
            CryptoPP::StreamTransformationFilter::PKCS_PADDING
            )
          );
    Q_UNUSED(s); // just to please the compiler
  }
  catch(const CryptoPP::Exception &e)
  {
    if (errCode != nullptr)
      *errCode = (int)e.GetErrorType();
    if (errMsg != nullptr)
      *errMsg = e.what();
    if (e.GetErrorType() > NoCryptError)
      qErrnoWarning(e.GetErrorType(), e.what());
  }

  QByteArray baCipher(cipher.c_str(), cipher.length());
  QByteArray result;
  result.append(static_cast<char>(DefaultEncryptionFormat));
  result.append(salt);
  result.append(baCipher);
  return result;
}


QByteArray Crypter::decode(const QString &masterPassword, QByteArray baCipher, bool uncompress, int iterations, int *errCode, QString *errMsg)
{
  if (errCode != nullptr)
    *errCode = NoCryptError;
  int formatFlag = baCipher.at(0);
  baCipher.remove(0, 1);

  const QByteArray &salt = QByteArray(baCipher.constData(), CryptSaltLength);
  baCipher.remove(0, CryptSaltLength);
  Password cryptPassword;
  cryptPassword.setIterations(iterations);
  cryptPassword.setSalt(salt);
  cryptPassword.generate(masterPassword.toUtf8());
  SecureByteArray key = cryptPassword.derivedKey(AESKeySize);

  switch (formatFlag) {
  case DefaultEncryptionFormat:
    break;
  case AES256EncryptedMasterkeyFormat:
    break;
  default:
    qWarning() << "unknow format flag:" << formatFlag;
    break;
  }

  std::string recovered;
  try {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), IV);
    std::string cipher(baCipher.constData(), baCipher.length());
    CryptoPP::StringSource s(
          cipher,
          true,
          new CryptoPP::StreamTransformationFilter(
            dec,
            new CryptoPP::StringSink(recovered)
            )
          );
    Q_UNUSED(s); // just to please the compiler
  }
  catch(const CryptoPP::Exception &e)
  {
    if (errCode != nullptr)
      *errCode = (int)e.GetErrorType();
    if (errMsg != nullptr)
      *errMsg = e.what();
    if (e.GetErrorType() > NoCryptError)
      qErrnoWarning(e.GetErrorType(), e.what());
  }
  QByteArray plain(recovered.c_str(), recovered.length());
  return uncompress ? qUncompress(plain) : plain;
}
