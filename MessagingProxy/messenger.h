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


#ifndef __MESSENGER_H_
#define __MESSENGER_H_

#include <QObject>
#include <QByteArray>
#include <QScopedPointer>

class MessengerPrivate;

class Messenger : public QObject
{
  Q_OBJECT
public:
  Messenger(void);
  ~Messenger();

public slots:
  void receiveMessage(void);
  void sendMessage(const QByteArray &msg);

signals:
  void messageReceived(QByteArray);
  void commandReceived(QByteArray);
  void quit(void);

private:
  QScopedPointer<MessengerPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Messenger)
  Q_DISABLE_COPY(Messenger)

};



#endif // __MESSENGER_H_
