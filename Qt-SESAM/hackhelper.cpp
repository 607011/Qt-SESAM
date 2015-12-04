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

#include "hackhelper.h"


void incrementEndianless(QByteArray &b)
{
  int i = b.size();
  while (--i) {
    quint8 byte = static_cast<quint8>(b.at(i));
    ++byte;
    b[i] = static_cast<char>(byte);
    if (byte != 0)
      break;
  }
}

QDebug operator<<(QDebug debug, const PositionTable &sub)
{
  QDebugStateSaver saver(debug);
  (void)saver;
  debug.nospace() << "PositionTable {\n";
  for (int i = 0; i < sub.size(); ++i) {
    const CharacterPositions &s = sub.at(i);
    debug.nospace() << s.character() << " @ " << s.positions() << "\n";
  }
  debug.nospace() << "}";
  return debug;
}
