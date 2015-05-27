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

#ifndef __PASSWORD_H_
#define __PASSWORD_H_

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QScopedPointer>

class PasswordParamBase {
public:
  static const QString LowerChars;
  static const QString UpperChars;
  static const QString UpperCharsNoAmbiguous;
  static const QString Digits;
  static const QString ExtraChars;

  PasswordParamBase(void)
    : domain("none")
    , salt("This is my salt. There are many like it, but this one is mine.")
    , masterPwd("Pl4c3 4n Incr3d1bly h4rd 7o 6u355 p4SSw0rd h3r3!")
    , availableChars(Digits)
    , passwordLength(10)
    , iterations(4096)
  { /* ... */ }
  PasswordParamBase(const PasswordParamBase &o)
    : domain(o.domain)
    , salt(o.salt)
    , masterPwd(o.masterPwd)
    , availableChars(o.availableChars)
    , passwordLength(o.passwordLength)
    , iterations(o.iterations)
  { /* ... */  }
  QByteArray domain;
  QByteArray salt;
  QByteArray masterPwd;
  QString availableChars;
  int passwordLength;
  int iterations;
};


class PasswordParam : public PasswordParamBase {
public:
  PasswordParam(const QByteArray &masterPwd)
  {
    this->masterPwd = masterPwd;
  }
  PasswordParam(
      const QByteArray &domain,
      const QByteArray &salt,
      const QByteArray &masterPwd,
      const QString &availableChars,
      const int passwordLength,
      const int iterations)
  {
    this->domain = domain;
    this->salt = salt;
    this->masterPwd = masterPwd;
    this->availableChars = availableChars;
    this->passwordLength = passwordLength;
    this->iterations = iterations;
  }
};


class PasswordPrivate;

class Password : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString key READ key)
  Q_PROPERTY(QString hexKey READ hexKey)
  Q_PROPERTY(QString elapsedSeconds READ elapsedSeconds)
  Q_PROPERTY(QRegExp validator READ validator WRITE setValidator)

public:
  explicit Password(QObject *parent = 0);
  ~Password();

  void abortGeneration(void);
  bool generate(const PasswordParam &p);
  void generateAsync(const PasswordParam &p);
  bool setValidator(const QRegExp &);
  const QRegExp &validator(void) const;
  bool setValidCharacters(const QStringList &canContain, const QStringList &mustContain);
  bool isValid(void) const;
  const QByteArray &derivedKey(void) const;
  const QString &key(void) const;
  const QString &hexKey(void) const;
  qreal elapsedSeconds(void) const;
  bool isRunning(void) const;
  void waitForFinished(void);
  QString errorString(void) const;

signals:
  void generationStarted(void);
  void generated(void);
  void generationAborted(void);

private:
  QScopedPointer<PasswordPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Password)
  Q_DISABLE_COPY(Password)
};


#endif // __PASSWORD_H_
