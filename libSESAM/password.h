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

#include <QDebug>
#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QByteArray>
#include <QBitArray>
#include <QVector>
#include <QMap>

#include "securebytearray.h"
#include "securestring.h"
#include "domainsettings.h"

class PasswordPrivate;


class Password : public QObject
{
  Q_OBJECT
public:
  typedef QMap<char, QString> TemplateCharacterMap;

  Password(const DomainSettings &ds = DomainSettings(), QObject *parent = Q_NULLPTR);
  ~Password();

  class Complexity {
  public:
    Complexity(void);
    Complexity(const Complexity &);
    Complexity(bool digits, bool lowercase, bool uppercase, bool extra);
    bool digits;
    bool lowercase;
    bool uppercase;
    bool extra;

    bool operator==(const Complexity &);
    bool operator!=(const Complexity &);
    int value(void) const;

    static Complexity fromValue(int c);
    static Complexity fromTemplate(const QString &templ);

  private:
    static const QVector<Complexity> Mapping;
  };

  enum PasswordError {
    NoError,
    EmptyCharacterSetError,
    EmptyTemplateError,
    InvalidTemplateError
  };

  static const QString Digits;
  static const QString LowerChars;
  static const QString UpperChars;
  static const QString ExtraChars;
  static const int DefaultMaxLength;
  static const int DefaultLength;
  static const int DefaultComplexityValue;
  static const int MaxComplexityValue;
  static const int NoComplexityValue;

  const SecureString &password(void) const;
  const SecureString &hexKey(void) const;
  const SecureString &remix(void);
  void waitForFinished(void);
  int error(void) const;
  QString errorString(void) const;
  void setDomainSettings(const DomainSettings &);

  void generate(const SecureByteArray &key);
  void generateAsync(const SecureByteArray &key, const DomainSettings &domainSettings = DomainSettings());

  bool isRunning(void) const;
  bool isAborted(void) const;
  qreal elapsedSeconds(void) const;
  void abortGeneration(void);

signals:
  void generated(void);
  void generationStarted(void);
  void generationAborted(void);

private:
  QScopedPointer<PasswordPrivate> d_ptr;
  Q_DECLARE_PRIVATE(Password)
  Q_DISABLE_COPY(Password)

  static const TemplateCharacterMap TemplateCharacters;
};


QDebug operator<<(QDebug debug, const Password::Complexity &);


#endif // __PASSWORD_H_
