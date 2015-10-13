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

#include <QObject>
#include <QString>
#include <QScopedPointer>
#include <QCryptographicHash>

#include "securebytearray.h"
#include "securestring.h"

class PBKDF2Private;

/*!
 * \brief The PBKDF2 class
 *
 * `PBKDF2` implements the Password-Based Key Derivation Function 2.
 *
 */
class PBKDF2 : public QObject
{
  Q_OBJECT
public:
  explicit PBKDF2(QObject *parent = Q_NULLPTR);
  PBKDF2(const SecureByteArray &pwd, const QByteArray &salt, int iterations, QCryptographicHash::Algorithm algorithm, QObject *parent = Q_NULLPTR);
  ~PBKDF2();

  void abortGeneration(void);
  void generate(const SecureByteArray &pwd, const QByteArray &salt, int iterations, QCryptographicHash::Algorithm algorithm);
  void generateAsync(const SecureByteArray &pwd, const QByteArray &salt, int iterations, QCryptographicHash::Algorithm algorithm);

  const SecureString &hexKey(void) const;
  SecureByteArray derivedKey(int size = -1) const;
  qreal elapsedSeconds(void) const;
  bool isRunning(void) const;
  bool isAborted(void) const;
  void waitForFinished(void);

signals:
  void generationStarted(void);
  void generationAborted(void);

private:
  QScopedPointer<PBKDF2Private> d_ptr;
  Q_DECLARE_PRIVATE(PBKDF2)
  Q_DISABLE_COPY(PBKDF2)
};


#endif // __PBKDF2_H_
