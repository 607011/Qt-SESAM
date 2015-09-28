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
  d_ptr->filename = filename;
}


Exporter::~Exporter()
{
  /* ... */
}


bool Exporter::write(const SecureByteArray &data)
{
  Q_D(Exporter);
  QFile file(d->filename);
  bool opened = file.open(QIODevice::WriteOnly);
  if (!opened)
    return false;

  file.write(PemPreamble.toUtf8());
  file.write("\n");
  SecureByteArray kgk_b64 = data.toBase64();
  for (int i = 0; i < kgk_b64.size(); i += 64) {
    file.write(kgk_b64.mid(i, qMin(64, kgk_b64.size() - i)));
    file.write("\n");
  }
  file.write(PemEpilog.toUtf8());
  file.write("\n");
  file.close();
  return true;
}


SecureByteArray Exporter::read(void)
{
  Q_D(Exporter);
  QFile file(d->filename);
  bool opened = file.open(QIODevice::ReadOnly);
  if (!opened)
    return false;
  SecureByteArray imported;
  static const int MaxLineSize = 66;
  char buf[MaxLineSize];
  int state = 0;
  SecureByteArray base64;
  while (!file.atEnd()) {
    qint64 bytesRead = file.readLine(buf, MaxLineSize);
    SecureString line = QByteArray(buf, bytesRead).trimmed();
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
        base64.append(line.toUtf8());
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
  imported = SecureByteArray::fromBase64(base64);

  return imported;
}
