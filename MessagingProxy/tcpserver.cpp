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
#include "tcpserverthread.h"
#include <QTcpServer>
#include <QNetworkConfigurationManager>
#include <QNetworkConfiguration>

class TcpServerPrivate
{
public:
  TcpServerPrivate(void)
  {
    /* ... */
  }
  ~TcpServerPrivate()
  {
  }
};

TcpServer::TcpServer(QTcpServer *parent)
  : QTcpServer(parent)
  , d_ptr(new TcpServerPrivate)
{
  listen(QHostAddress::LocalHost, Port);
}


TcpServer::~TcpServer()
{
  QTcpServer::close();
}


void TcpServer::incomingConnection(qintptr socketDescriptor)
{
  TcpServerThread *thread = new TcpServerThread(socketDescriptor, this);
  QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  QObject::connect(thread, SIGNAL(started()), SIGNAL(connectionEstablished()));
  QObject::connect(thread, SIGNAL(gotCommand(QJsonObject)), SIGNAL(commandReceived(QJsonObject)));
  thread->start();
}
