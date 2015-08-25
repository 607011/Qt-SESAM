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

#ifndef __SERVERCERTIFICATEWIDGET_H_
#define __SERVERCERTIFICATEWIDGET_H_

#include <QLayout>
#include <QDialog>
#include <QList>
#include <QSslSocket>


namespace Ui {
class ServerCertificateWidget;
}

class ServerCertificateWidget : public QDialog
{
  Q_OBJECT

public:
  explicit ServerCertificateWidget(QWidget *parent = nullptr);
  ~ServerCertificateWidget();

  void setServerSocket(const QSslSocket &);

  static void clearLayout(QLayout *);

private:
  Ui::ServerCertificateWidget *ui;
};

#endif // __SERVERCERTIFICATEWIDGET_H_
