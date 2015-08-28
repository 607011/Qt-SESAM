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

#ifndef __PBKDF2_H_
#define __PBKDF2_H_

#include "domainsettings.h"
#include "crypter.h"

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QScopedPointer>
#include <QCryptographicHash>


class PBKDF2Private;


class PBKDF2 : public QObject
{
  Q_OBJECT
public:
  explicit PBKDF2(QObject *parent = nullptr);
  ~PBKDF2();

  void abortGeneration(void);
  void generate(const SecureByteArray &masterPassword, QCryptographicHash::Algorithm algorithm);
  void generateAsync(const SecureByteArray &masterPassword, QCryptographicHash::Algorithm algorithm);

  const QString &key(void) const;
  const QString &hexKey(void) const;
  QByteArray derivedKey(int size = -1) const;
  qreal elapsedSeconds(void) const;
  bool isRunning(void) const;
  bool isAborted(void) const;

  void waitForFinished(void);
  QString errorString(void) const;
  void setDomainSettings(const DomainSettings &);
  void setIterations(int iterations);
  void setSalt_base64(const QByteArray &);
  void setSalt(const QByteArray &);
  QByteArray salt(void) const;

  static const QString LowerChars;
  static const QString UpperChars;
  static const QString UpperCharsNoAmbiguous;
  static const QString Digits;
  static const QString ExtraChars;
  static const QString AllChars;

signals:
  void generationStarted(void);
  void generated(void);
  void generationAborted(void);

private:
  QScopedPointer<PBKDF2Private> d_ptr;
  Q_DECLARE_PRIVATE(PBKDF2)
  Q_DISABLE_COPY(PBKDF2)
};


#endif // __PBKDF2_H_
