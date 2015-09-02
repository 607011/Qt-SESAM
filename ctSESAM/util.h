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

#ifndef __UTIL_H_
#define __UTIL_H_


#include <cstring>
#include <QObject>
#include <QColor>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <qmath.h>


#ifndef __in
#define __in
#endif

#ifndef __out
#define __out
#endif

#ifndef __inout
#define __inout
#endif


template <class T>
void SafeRenew(T& a, T obj)
{
  if (a != nullptr)
    delete a;
  a = obj;
}


template <class T>
void SafeDelete(T& a)
{
  SafeRenew<T>(a, nullptr);
}


template <typename T>
void SecureErase(T *m, int size)
{
  memset(m, 0, size);
}


template <class T>
void SecureErase(T &obj)
{
  SecureErase(obj.data(), obj.size());
  obj.clear();
}


template <typename REAL>
static REAL entropy(const QString &str) {
  static const int Range = 256;
  const QByteArray &sample = str.toLatin1();
  QVector<int> histo(Range, 0);
  for (int i = 0; i < sample.size(); ++i) {
    ++histo[static_cast<uchar>(sample.at(i))];
  }
  REAL ent = 0;
  const REAL l = sample.size();
  for (int i = 0; i < Range; ++i) {
    const REAL p = REAL(histo[i]) / l;
    if (p > 0)
      ent += p * REAL(M_LOG2E) * qLn(REAL(1) / p);
  }
  const REAL bitsPerVariate = qLn(REAL(Range)) * REAL(M_LOG2E);
  return ent / bitsPerVariate;
}


template <typename REAL>
void evaluatePasswordStrength(const QString &password, __out QColor &color, __out QString &grade, __out REAL *_fitness)
{
  REAL fitness = 0;
  color.setRgb(153, 153, 153);
  if (password.isEmpty()) {
    grade = "?";
  }
  else {
    fitness = password.size() * entropy<REAL>(password);
    if (fitness >= REAL(11.0)) {
      color.setRgb(0, 255, 30);
      grade = QObject::tr("Supercalifragilisticexpialidocious");
    }
    else if (fitness >= REAL(9.0)) {
      color.setRgb(0, 255, 30);
      grade = QObject::tr("Brutally strong");
    }
    else if (fitness >= REAL(7.0)) {
      color.setRgb(0, 255, 30);
      grade = QObject::tr("Fabulous");
    }
    else if (fitness >= REAL(5.0)) {
      color.setRgb(0, 255, 30);
      grade = QObject::tr("Very good");
    }
    else if (fitness >= REAL(4.0)) {
      color.setRgb(111, 255, 0);
      grade = QObject::tr("Good");
    }
    else if (fitness >= REAL(3.0)) {
      color.setRgb(234, 255, 0);
      grade = QObject::tr("Mediocre");
    }
    else if (fitness >= REAL(2.5)) {
      color.setRgb(255, 153, 0);
      grade = QObject::tr("You can do better");
    }
    else if (fitness >= REAL(2.0)) {
      color.setRgb(255, 48, 0);
      grade = QObject::tr("Bad");
    }
    else if (fitness >= REAL(1.5)) {
      color.setRgb(255, 0, 0);
      grade = QObject::tr("It can hardly be worse");
    }
    else {
      color.setRgb(200, 0, 0);
      grade = QObject::tr("Useless");
    }
  }
  if (_fitness != nullptr)
    *_fitness = fitness;
}

extern QString fingerprintify(const QByteArray &ba);



#endif // __UTIL_H_
