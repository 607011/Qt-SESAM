/*

    Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

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

#include "securestring.h"
#include "util.h"


SecureString::SecureString(void)
{
  /* ... */
}


SecureString::SecureString(const QChar *unicode, int size)
  : QString(unicode, size)
{
  /* ... */
}


SecureString::SecureString(QChar ch)
  : QString(ch)
{
  /* ... */
}


SecureString::SecureString(int size, QChar ch)
  : QString(size, ch)
{
  /* ... */
}


SecureString::SecureString(QLatin1String str)
  : QString(str)
{
  /* ... */
}


SecureString::SecureString(const QString &other)
  : QString(other)
{
  /* ... */
}


SecureString::SecureString(QString &&other)
  : QString(other)
{
  /* ... */
}


SecureString::SecureString(const char *str)
  : QString(str)
{
  /* ... */
}


SecureString::SecureString(const QByteArray &ba)
  : QString(ba)
{
  /* ... */
}


SecureString::~SecureString()
{
  invalidate();
}


void SecureString::invalidate(void)
{
  SecureErase(*this);
}
