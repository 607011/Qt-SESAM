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


#include "passwordchecker.h"

#include <QDebug>
#include <QFile>
#include <QColor>

class PasswordCheckerPrivate
{
public:
  PasswordCheckerPrivate(void)
  { /* ... */ }
  ~PasswordCheckerPrivate()
  { /* ... */ }
  QFile pwdFile;
  QString pwdFilename;
};


PasswordChecker::PasswordChecker(const QString &passwordFilename, QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordCheckerPrivate)
{
  d_ptr->pwdFilename = passwordFilename;
}


PasswordChecker::~PasswordChecker()
{
  // ...
}


qint64 PasswordChecker::findInPasswordFile(const QString &needle)
{
  Q_D(PasswordChecker);
  qint64 pos = -1;
  if (!d->pwdFilename.isEmpty()) {
    d->pwdFile.setFileName(d->pwdFilename);
    d->pwdFile.open(QIODevice::ReadOnly);
    if (d->pwdFile.isOpen()) {
      pos = findInPasswordFile(0, d->pwdFile.size(), needle);
      d->pwdFile.close();
    }
  }
  return pos;
}


qint64 PasswordChecker::findInPasswordFile(qint64 lo, qint64 hi, const QString &needle)
{
  Q_D(PasswordChecker);
  if (hi <= lo)
    return -1;
  qint64 mid = (lo + hi) / 2;
  d->pwdFile.seek(mid);
  char ch = '\0';
  while (d->pwdFile.read(&ch, 1) == 1 && !d->pwdFile.atEnd()) {
    if (ch == '\n')
      break;
    d->pwdFile.seek(d->pwdFile.pos() - 2);
  }
  mid = d->pwdFile.pos();
  const QString &word = QString::fromLatin1(d->pwdFile.readLine()).trimmed();
  const int comparison = needle.compare(word, Qt::CaseInsensitive);
  if (comparison < 0) {
    return findInPasswordFile(lo, mid - 1, needle);
  }
  else if (comparison > 0) {
    return findInPasswordFile(mid + word.size(), hi, needle);
  }
  return mid;
}


qreal PasswordChecker::entropy(const QString &str) {
  static const int Range = 256;
  const QByteArray &sample = str.toLatin1();
  QVector<int> histo(Range, 0);
  for (int i = 0; i < sample.size(); ++i) {
    ++histo[static_cast<uchar>(sample.at(i))];
  }
  qreal ent = 0;
  const qreal l = sample.size();
  for (int i = 0; i < Range; ++i) {
    const qreal p = qreal(histo[i]) / l;
    if (p > 0) {
      ent += p * M_LOG2E * qLn(1.0 / p);
    }
  }
  const qreal bitsPerVariate = qLn(qreal(Range)) * M_LOG2E;
  return ent / bitsPerVariate;
}


void PasswordChecker::evaluatePasswordStrength(const QString &password, QColor &color, QString &grade, qreal *_fitness)
{
  qreal fitness = 0;
  color.setRgb(153, 153, 153);
  if (password.isEmpty()) {
    grade = "?";
  }
  else {
    fitness = password.size() * entropy(password);
    if (fitness >= 11.0) {
      color.setRgb(0, 255, 30);
      grade = tr("Supercalifragilisticexpialidocious");
    }
    else if (fitness >= 9.0) {
      color.setRgb(0, 255, 30);
      grade = tr("Brutally strong");
    }
    else if (fitness >= 7.0) {
      color.setRgb(0, 255, 30);
      grade = tr("Fabulous");
    }
    else if (fitness >= 5.0) {
      color.setRgb(0, 255, 30);
      grade = tr("Very good");
    }
    else if (fitness >= 4.0) {
      color.setRgb(111, 255, 0);
      grade = tr("Good");
    }
    else if (fitness >= 3.0) {
      color.setRgb(234, 255, 0);
      grade = tr("Mediocre");
    }
    else if (fitness >= 2.5) {
      color.setRgb(255, 153, 0);
      grade = tr("You can do better");
    }
    else if (fitness >= 2.0) {
      color.setRgb(255, 48, 0);
      grade = tr("Bad");
    }
    else if (fitness >= 1.5) {
      color.setRgb(255, 0, 0);
      grade = tr("It can hardly be worse");
    }
    else {
      color.setRgb(200, 0, 0);
      grade = tr("Useless");
    }
  }
  if (_fitness != Q_NULLPTR)
    *_fitness = fitness;
}
