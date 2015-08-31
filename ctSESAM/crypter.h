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


#ifndef __CRYPTER_H_
#define __CRYPTER_H_

#include <QByteArray>
#include <QString>
#include "securebytearray.h"
#include "util.h"
#include "3rdparty/cryptopp562/filters.h"
#include "3rdparty/cryptopp562/aes.h"

class Crypter
{
public:
  static const int SaltSize;
  static const int AESKeySize;
  static const int AESBlockSize;
  static const int KGKSize;
  static const int KGKIterations;
  static const int DomainIterations;
  static const int EEKSize;
  enum FormatFlags {
    ObsoleteDefaultEncryptionFormat = 0x00,
    AES256EncryptedMasterkeyFormat = 0x01
  };
  static SecureByteArray makeKeyFromPassword(
      __in const SecureByteArray &masterPassword,
      __in const QByteArray &salt);
  static void makeKeyAndIVFromPassword(
      __in const SecureByteArray &masterPassword,
      __in const QByteArray &salt,
      __out SecureByteArray &key,
      __out SecureByteArray &IV);
  static QByteArray encode(__in const SecureByteArray &key,
                           __in const SecureByteArray &IV,
                           __in const QByteArray &salt,
                           __in const SecureByteArray &KGK,
                           __in const QByteArray &data,
                           __in bool compress);
  static QByteArray decode(__in const SecureByteArray &masterPassword,
                           __in QByteArray baCipher,
                           __in bool uncompress,
                           __out SecureByteArray &KGK);
  static QByteArray randomBytes(int size = SaltSize);
  static QByteArray encrypt(const SecureByteArray &key, const SecureByteArray &IV, const QByteArray &baPlain, CryptoPP::StreamTransformationFilter::BlockPaddingScheme padding);
  static SecureByteArray decrypt(const SecureByteArray &key, const SecureByteArray &IV, const QByteArray &baCipher, CryptoPP::StreamTransformationFilter::BlockPaddingScheme padding);
};

#endif // __CRYPTER_H_
