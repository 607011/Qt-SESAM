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

#include <QVariantMap>
#include <QJsonDocument>
#include <QDateTime>

#include <iostream>

#ifdef Q_OS_WIN
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);
  std::cout.setf(std::ios_base::unitbuf);
#ifdef Q_OS_WIN
  _setmode(_fileno(stdin), _O_BINARY);
#endif
  static const int BufSize = 4096;
  char *buf = new char[BufSize];
  forever {
    int inLen = 0;
    std::cin.read(reinterpret_cast<char*>(&inLen), 4);
    if (inLen == 0)
      break;
    inLen = qMin(inLen, BufSize);
    std::cin.read(buf, inLen);
    QJsonParseError jsonError;
    QJsonDocument jsonIn = QJsonDocument::fromJson(QByteArray(buf, inLen), &jsonError);
    QVariantMap inbound = jsonIn.toVariant().toMap();
    inbound["bytes_in"] = inLen;
    inbound["timestamp"] = QDateTime::currentDateTime().toString();
    if (jsonError != QJsonParseError::NoError)
      inbound["errors"] = jsonError.errorString();
    std::string msg = QJsonDocument::fromVariant(inbound).toJson(QJsonDocument::Compact).toStdString();
    quint32 outLen = msg.length();
    std::cout.write(reinterpret_cast<char*>(&outLen), 4);
    std::cout << msg;
  }
  delete[] buf;
  return 0;
}
