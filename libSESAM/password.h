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
#include <QString>
#include <QScopedPointer>
#include <QByteArray>
#include <QBitArray>
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
  enum {
    TemplateDigits = 0,
    TemplateLowercase,
    TemplateUppercase,
    TemplateExtra }
  TemplatePlaceholder;

  Password(const DomainSettings &ds = DomainSettings(), QObject *parent = Q_NULLPTR);
  ~Password();

  static const QString LowerChars;
  static const QString UpperChars;
  static const QString UpperCharsNoAmbiguous;
  static const QString Digits;
  static const QString ExtraChars;
  static const QString AllChars;
  static const int DefaultMaxLength;
  static const int DefaultLength;
  static const int DefaultComplexity;

  SecureString operator()(void) const;
  const SecureString &password(void) const;
  const SecureString &hexKey(void) const;
  const SecureString &remixed(void);
  void waitForFinished(void);
  QString errorString(void) const;
  void setDomainSettings(const DomainSettings &);

  static QBitArray deconstructedComplexity(int complexity);
  static const QString &charSetFor(char);

  void generate(const SecureByteArray &masterPassword);
  void generateAsync(const SecureByteArray &masterPassword, const DomainSettings &domainSettings = DomainSettings());

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

#endif // __PASSWORD_H_
