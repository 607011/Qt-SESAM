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
    : availableChars(Digits)
    , passwordLength(10)
    , iterations(4096)
  { /* ... */ }
  explicit PasswordParamBase(const PasswordParamBase &o)
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
  explicit PasswordParam(const QByteArray &masterPwd)
  {
    this->masterPwd = masterPwd;
  }
  PasswordParam(
      const QByteArray &domain,
      const QByteArray &masterPwd,
      const QString &availableChars,
      const int passwordLength,
      const int iterations)
  {
    this->domain = domain;
    this->masterPwd = masterPwd;
    this->availableChars = availableChars;
    this->passwordLength = passwordLength;
    this->iterations = iterations;
  }
  PasswordParam(
      const QByteArray &domain,
      const QByteArray &masterPwd,
      const QByteArray &salt,
      const QString &availableChars,
      const int passwordLength,
      const int iterations)
  {
    this->domain = domain;
    this->masterPwd = masterPwd;
    this->salt = salt;
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
  Q_PROPERTY(QRegExp validator READ validator WRITE setValidator)
  Q_PROPERTY(bool isValid READ isValid)
  Q_PROPERTY(qreal elapsedSeconds READ elapsedSeconds)
  Q_PROPERTY(QByteArray salt READ salt)

public:
  explicit Password(QObject *parent = nullptr);
  ~Password();

  void abortGeneration(void);
  bool generate(const PasswordParam &p);
  void generateAsync(const PasswordParam &p);

  const QRegExp &validator(void) const;
  bool setValidator(const QRegExp &);

  bool setValidCharacters(const QStringList &canContain, const QStringList &mustContain);

  bool isValid(void) const;
  const QString &key(void) const;
  const QString &hexKey(void) const;
  qreal elapsedSeconds(void) const;
  bool isRunning(void) const;

  const QByteArray &salt(void) const;

  void waitForFinished(void);
  QString errorString(void) const;
  void extractAESKey(__inout char *const aesKey, __in int size);

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
