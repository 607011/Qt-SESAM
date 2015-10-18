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

#include "tcpserverthread.h"


TcpServerThread::TcpServerThread(int socketDescriptor, QObject *parent)
  : QThread(parent)
  , mSocketDescriptor(socketDescriptor)
{
  /* ... */
}

void TcpServerThread::run(void)
{
  QTcpSocket tcpSocket;
  if (!tcpSocket.setSocketDescriptor(mSocketDescriptor)) {
    emit error(tcpSocket.error());
    return;
  }

  emit started();
  //  QByteArray block;
  //  QDataStream out(&block, QIODevice::WriteOnly);
  //  out.setVersion(QDataStream::Qt_4_0);
  //  out << (quint16)0;
  //  out << text;
  //  out.device()->seek(0);
  //  out << (quint16)(block.size() - sizeof(quint16));
  //  tcpSocket.write(block);
  tcpSocket.disconnectFromHost();
  tcpSocket.waitForDisconnected();
}

