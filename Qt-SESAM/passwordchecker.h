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


#ifndef __PASSWORDCHECKER_H_
#define __PASSWORDCHECKER_H_

#include <QtGlobal>
#include <QObject>
#include <QByteArray>
#include <QScopedPointer>

#include "util.h"

class PasswordCheckerPrivate;

class PasswordChecker : public QObject
{
  Q_OBJECT
public:
  explicit PasswordChecker(const QString &passwordFilename = QString(), QObject *parent = nullptr);
  ~PasswordChecker();

  qint64 findInPasswordFile(const QString &needle);

  static qreal entropy(const QString &);
  static void evaluatePasswordStrength(const QString &password, QColor &color, QString &grade, qreal *_fitness);

signals:

public slots:

private:
  QScopedPointer<PasswordCheckerPrivate> d_ptr;
  Q_DECLARE_PRIVATE(PasswordChecker)
  Q_DISABLE_COPY(PasswordChecker)

private: // methods
  qint64 findInPasswordFile(qint64 lo, qint64 hi, const QString &needle);
};

#endif // __PASSWORDCHECKER_H_
