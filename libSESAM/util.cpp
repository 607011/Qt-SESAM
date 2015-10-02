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


#include "util.h"


QString fingerprintify(const QByteArray &ba) {
  const QByteArray &baHex = ba.toHex();
  QByteArray result = baHex;
  for (int i = baHex.size() - 2; i > 0; i -= 2)
    result.insert(i, ':');
  return result;
}


#if defined(Q_CC_GNU)
void SecureErase(QString str)
{
  for (QString::iterator i = str.begin(); i != str.end(); ++i)
    *i = 0;
  str.clear();
}
#endif


/*!
 * \brief shuffle
 *
 * Fisher-Yates shuffling of a string.
 *
 * \param s String to be shuffled
 * \return shuffled string
 */
QString shuffle(const QString& s)
{
  QString result = s;
  int n = result.count();
  QChar *c = result.data();
  while (n) {
    int j = qrand() % n--;
    QChar tmp = result.at(n);
    *(c + n) = result.at(j);
    *(c + j) = tmp;
  }
  return result;
}
