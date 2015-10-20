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

#ifndef __TCPSERVER_H_
#define __TCPSERVER_H_

#include <QObject>
#include <QScopedPointer>
#include <QTcpServer>
#include <QByteArray>

class TcpServerPrivate;

class TcpServer : public QTcpServer
{
  Q_OBJECT
public:
  explicit TcpServer(QTcpServer *parent = Q_NULLPTR);
  ~TcpServer();

signals:
  void connectionEstablished(void);
  void commandReceived(QByteArray);

private slots:
  void forwardCommand(void);
  void sendCommand(QByteArray);
  void gotConnection(void);

private:
  QScopedPointer<TcpServerPrivate> d_ptr;
  Q_DECLARE_PRIVATE(TcpServer)
  Q_DISABLE_COPY(TcpServer)

  static const int Port = 53548;
};

#endif // __TCPSERVER_H_
