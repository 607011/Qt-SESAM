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


#include <QDebug>
#include <QFile>
#include "exporter.h"
#include "securestring.h"
#include "crypter.h"
#include "pbkdf2.h"
#include "cryptlib.h"
#include "ccm.h"

static const QString PemPreamble = "-----BEGIN ENCRYPTED PRIVATE KEY-----";
static const QString PemEpilog = "-----END ENCRYPTED PRIVATE KEY-----";


class ExporterPrivate {
public:
  ExporterPrivate(void)
  { /* ... */ }
  ~ExporterPrivate(void)
  { /* ... */ }
  QString filename;
};


Exporter::Exporter(QObject *parent)
  : QObject(parent)
  , d_ptr(new ExporterPrivate)
{
  /* ... */
}


Exporter::Exporter(const QString &filename, QObject *parent)
  : Exporter(parent)
{
  setFileName(filename);
}


Exporter::~Exporter()
{
  /* ... */
}


void Exporter::setFileName(const QString &filename)
{
  Q_D(Exporter);
  d->filename = filename;
}


bool Exporter::write(const SecureByteArray &data, const SecureString &pwd)
{
  Q_D(Exporter);
  Q_ASSERT((data.size() % Crypter::AESBlockSize) == 0);
  QByteArray salt = Crypter::generateSalt();
  SecureByteArray iv;
  SecureByteArray key;
  Crypter::makeKeyAndIVFromPassword(pwd.toUtf8(), salt, key, iv);
  CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
  enc.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), reinterpret_cast<const byte*>(iv.constData()));
  const int cipherSize = data.size();
  QByteArray cipher(cipherSize, static_cast<char>(0));
  CryptoPP::ArraySource s(
        reinterpret_cast<const byte*>(data.constData()), data.size(),
        true,
        new CryptoPP::StreamTransformationFilter(
          enc,
          new CryptoPP::ArraySink(reinterpret_cast<byte*>(cipher.data()), cipher.size()),
          CryptoPP::StreamTransformationFilter::NO_PADDING
          )
        );
  Q_UNUSED(s); // just to please the compiler
  QFile file(d->filename);
  bool opened = file.open(QIODevice::WriteOnly);
  if (!opened)
    return false;
  QByteArray block = salt + cipher;
  file.write(PemPreamble.toUtf8());
  file.write("\n");
  const SecureByteArray &b64 = block.toBase64();
  for (int i = 0; i < b64.size(); i += 64) {
    file.write(b64.mid(i, qMin(64, b64.size() - i)));
    file.write("\n");
  }
  file.write(PemEpilog.toUtf8());
  file.write("\n");
  file.close();
  return true;
}


SecureByteArray Exporter::read(const SecureString &pwd)
{
  Q_D(Exporter);
  SecureByteArray plain;
  QFile file(d->filename);
  bool opened = file.open(QIODevice::ReadOnly);
  if (opened) {
    static const int MaxLineSize = 64 + 2;
    int state = 0;
    QByteArray b64;
    while (!file.atEnd()) {
      char buf[MaxLineSize];
      const qint64 bytesRead = file.readLine(buf, MaxLineSize);
      const QString line = QByteArray(buf, bytesRead).trimmed();
      switch (state) {
      case 0:
        if (line == PemPreamble) {
          state = 1;
        }
        else {
          qWarning() << "bad format";
        }
        break;
      case 1:
        if (line != PemEpilog) {
          b64.append(line.toUtf8());
        }
        else {
          state = 2;
        }
        break;
      case 2:
        // ignore trailing lines
        break;
      default:
        qWarning() << "Oops! Should never have gotten here.";
        break;
      }
    }
    file.close();
    SecureByteArray iv;
    SecureByteArray key;
    QByteArray imported = QByteArray::fromBase64(b64);
    QByteArray salt = imported.mid(0, Crypter::SaltSize);
    QByteArray cipher = imported.mid(Crypter::SaltSize);
    Crypter::makeKeyAndIVFromPassword(pwd.toUtf8(), salt, key, iv);
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(reinterpret_cast<const byte*>(key.constData()), key.size(), reinterpret_cast<const byte*>(iv.constData()));
    plain = SecureByteArray(cipher.size(), static_cast<char>(0));
    CryptoPP::ArraySource s(
          reinterpret_cast<const byte*>(cipher.constData()), cipher.size(),
          true,
          new CryptoPP::StreamTransformationFilter(
            dec,
            new CryptoPP::ArraySink(reinterpret_cast<byte*>(plain.data()), plain.size()),
            CryptoPP::StreamTransformationFilter::NO_PADDING
            )
          );
    Q_UNUSED(s); // just to please the compiler
  }
  return plain;
}
