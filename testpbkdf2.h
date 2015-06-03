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

#ifndef __TESTPBKDF2_H_
#define __TESTPBKDF2_H_

#include <QObject>
#include <QtTest/QTest>

#include "DomainSettings.h"

class TestPBKDF2 : public QObject
{
  Q_OBJECT
public:
  explicit TestPBKDF2(QObject *parent = 0);

signals:

private slots:
  void simple1(void);
  void simple2(void);
  void pin(void);
  void aesKey(void);

};

#endif // __TESTPBKDF2_H_
