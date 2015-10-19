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


#ifndef __SECURESTRING_H_
#define __SECURESTRING_H_

#include <QString>

/*!
 * \brief The SecureString class
 *
 * `SecureString` augments `QString` with an invalidation function that overwrites
 * the allocated memory of the object with 0, then clears the object.
 *
 */
class SecureString : public QString
{
public:
  SecureString(void);
  SecureString(const QChar *unicode, int size = -1);
  SecureString(QChar ch);
  SecureString(int size, QChar ch);
  SecureString(QLatin1String str);
  SecureString(const QString &other);
  SecureString(QString &&other);
  SecureString(const char *str);
  SecureString(const QByteArray &ba);
  ~SecureString();

protected:
  void invalidate(void);
};

#endif // __SECURESTRING_H_
