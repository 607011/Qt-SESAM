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

#include "tcpserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

class TcpServerPrivate
{
public:
  TcpServerPrivate(void)
    : conn(Q_NULLPTR)
  { /* ... */ }
  ~TcpServerPrivate()
  { /* ... */ }
  QTcpSocket *conn;
};

TcpServer::TcpServer(QTcpServer *parent)
  : QTcpServer(parent)
  , d_ptr(new TcpServerPrivate)
{
  listen(QHostAddress::LocalHost, Port);
  QObject::connect(this, SIGNAL(newConnection()), this, SLOT(gotConnection()));
}


TcpServer::~TcpServer()
{
  QTcpServer::close();
}


void TcpServer::gotConnection(void)
{
  Q_D(TcpServer);
  d->conn = nextPendingConnection();
  connect(d->conn, SIGNAL(readyRead()), this, SLOT(forwardCommand()));
  connect(d->conn, SIGNAL(disconnected()), d->conn, SLOT(deleteLater()));
}


void TcpServer::forwardCommand(void)
{
  Q_D(TcpServer);
  QByteArray msg = d->conn->readAll();
  emit commandReceived(msg);
}


void TcpServer::sendCommand(QByteArray msg)
{
  Q_D(TcpServer);
  d->conn->write(msg);
}
