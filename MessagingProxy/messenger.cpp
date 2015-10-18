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
#include <QDateTime>
#include <QJsonParseError>
#include <QSocketNotifier>

#include <iostream>

#ifdef Q_OS_WIN
#include <io.h>
#include <fcntl.h>
#endif


#include "messenger.h"


class MessengerPrivate {
public:
  static const int BufSize = 4096;
  MessengerPrivate(QObject *parent)
    : buf(new char[BufSize])
    , inNotifier(new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, parent))
    , outNotifier(new QSocketNotifier(fileno(stdout), QSocketNotifier::Write, parent))
  { /* ... */ }
  ~MessengerPrivate()
  {
    delete[] buf;
  }
  char *buf;
  QSocketNotifier *inNotifier;
  QSocketNotifier *outNotifier;
};



Messenger::Messenger(void)
  : d_ptr(new MessengerPrivate(this))
{
  Q_D(Messenger);
  std::cout.setf(std::ios_base::unitbuf);
#ifdef Q_OS_WIN
  _setmode(_fileno(stdin), _O_BINARY);
#endif
  QObject::connect(d->inNotifier, SIGNAL(activated(int)), this, SLOT(receiveCommand()));
}


void Messenger::receiveCommand(void)
{
  Q_D(Messenger);
  forever {
    int inLen = 0;
    std::cin.read(reinterpret_cast<char*>(&inLen), 4);
    if (inLen == 0)
      break;
    inLen = qMin(inLen, MessengerPrivate::BufSize);
    std::cin.read(d->buf, inLen);
    QJsonParseError jsonError;
    QJsonDocument jsonIn = QJsonDocument::fromJson(QByteArray(d->buf, inLen), &jsonError);
    QVariantMap inbound = jsonIn.toVariant().toMap();
    inbound["bytes_in"] = inLen;
    inbound["timestamp"] = QDateTime::currentDateTime().toString();
    if (jsonError.error != QJsonParseError::NoError)
      inbound["errors"] = jsonError.errorString();
    std::string msg = QJsonDocument::fromVariant(inbound).toJson(QJsonDocument::Compact).toStdString();
    quint32 outLen = msg.length();
    std::cout.write(reinterpret_cast<char*>(&outLen), 4);
    std::cout << msg;
  }
}


void Messenger::forwardCommand(QJsonObject msg)
{
  Q_D(Messenger);
  forever {
    int inLen = 0;
    std::cin.read(reinterpret_cast<char*>(&inLen), 4);
    if (inLen == 0)
      break;
    inLen = qMin(inLen, MessengerPrivate::BufSize);
    std::cin.read(d->buf, inLen);
    QJsonParseError jsonError;
    QJsonDocument jsonIn = QJsonDocument::fromJson(QByteArray(d->buf, inLen), &jsonError);
    QVariantMap inbound = jsonIn.toVariant().toMap();
    inbound["bytes_in"] = inLen;
    inbound["timestamp"] = QDateTime::currentDateTime().toString();
    if (jsonError.error != QJsonParseError::NoError)
      inbound["errors"] = jsonError.errorString();
    std::string msg = QJsonDocument::fromVariant(inbound).toJson(QJsonDocument::Compact).toStdString();
    quint32 outLen = msg.length();
    std::cout.write(reinterpret_cast<char*>(&outLen), 4);
    std::cout << msg;
  }
}


Messenger::~Messenger()
{
  // ...
}
