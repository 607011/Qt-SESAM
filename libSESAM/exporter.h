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

#ifndef __EXPORTER_H_
#define __EXPORTER_H_

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QScopedPointer>

#include "securebytearray.h"
#include "securestring.h"


class ExporterPrivate;

class Exporter : public QObject
{
  Q_OBJECT
public:
  explicit Exporter(QObject *parent = Q_NULLPTR);
  Exporter(const QString &filename, QObject *parent = Q_NULLPTR);
  ~Exporter();
  void setFileName(const QString &);
  bool write(const SecureByteArray &data, const SecureString &pwd);
  SecureByteArray read(const SecureString &pwd);

signals:

public slots:

private:
  QScopedPointer<ExporterPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Exporter)
  Q_DISABLE_COPY(Exporter)
};

#endif // __EXPORTER_H_
