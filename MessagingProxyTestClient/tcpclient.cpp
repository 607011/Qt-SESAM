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

#include "tcpclient.h"

#include <QDebug>
#include <QTextStream>
#include <QHostAddress>


TcpClient::TcpClient(QObject *parent)
  : QObject(parent)
{
  mTcpSocket = new QTcpSocket(this);
  QObject::connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(displayIncoming()));
  mTcpSocket->abort();
  mTcpSocket->connectToHost(QHostAddress::LocalHost, 53548);
  if (mTcpSocket->waitForConnected()) {
    QByteArray msg = "{ \"text\": \"message from MessagingProxyTestClient\" }";
    mTcpSocket->write(msg);
  }
}

void TcpClient::displayIncoming(void)
{
  QByteArray msg = mTcpSocket->readAll();
  qDebug() << "message from proxy:" << QString::fromUtf8(msg);
}

