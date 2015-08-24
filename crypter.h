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
#include <string>

#include "cryptopp562/sha.h"
#include "cryptopp562/aes.h"
#include "cryptopp562/ccm.h"
#include "cryptopp562/filters.h"
#include "cryptopp562/misc.h"

class Crypter
{
public:
  static const int NoCryptError = -1;
  enum FormatFlags {
    DefaultEncryptionFormat = 0x00,
    AES256EncryptedMasterkeyFormat = 0x01
  };
  static QByteArray encode(const QString &masterPassword, const QByteArray &data, bool compress, int *errCode = nullptr, QString *errMsg = nullptr);
  static QByteArray decode(const QString &masterPassword, QByteArray data, bool uncompress, int *errCode = nullptr, QString *errMsg = nullptr);
};

#endif // __CRYPTER_H_
