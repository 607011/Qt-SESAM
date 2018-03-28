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

#include <QDebug>
#include <QtConcurrent>
#include <iostream>

#ifdef Q_OS_WIN
#include <io.h>
#include <fcntl.h>
#endif


#include "messenger.h"


class MessengerPrivate {
public:
  MessengerPrivate(void)
  { /* ... */ }
  ~MessengerPrivate()
  { /* ... */ }
  QFuture<void> inFuture;
};



Messenger::Messenger(void)
  : d_ptr(new MessengerPrivate)
{
  Q_D(Messenger);
  std::cout.setf(std::ios_base::unitbuf);
#ifdef Q_OS_WIN
  _setmode(_fileno(stdin), _O_BINARY);
#endif
  d->inFuture = QtConcurrent::run(this, &Messenger::receiveMessage);
}


Messenger::~Messenger()
{
  Q_D(Messenger);
  d->inFuture.waitForFinished();
}


void Messenger::receiveMessage(void)
{
  static const int BufSize = 4096;
  char buf[BufSize];
  forever {
    int inLen = 0;
    std::cin.read(reinterpret_cast<char*>(&inLen), sizeof(inLen));
    if (inLen == 0)
      break;
    inLen = qMin(inLen, BufSize);
    std::cin.read(buf, inLen);
    const QByteArray &msg = QByteArray(buf, inLen);
    emit messageReceived(msg);
  }
  emit quit();
}


void Messenger::sendMessage(const QByteArray &msg)
{
  quint32 outLen = static_cast<quint32>(msg.length());
  std::cout.write(reinterpret_cast<char*>(&outLen), sizeof(outLen));
  std::cout.write(msg.data(), msg.length());
}
