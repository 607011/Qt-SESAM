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

#include "lockfile.h"
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QCoreApplication>

class LockFilePrivate {
public:
  LockFilePrivate(const QString &lockFilename)
    : lockFilename(lockFilename)
    , lockFile(lockFilename)
  { /* ... */ }
  ~LockFilePrivate()
  { /* ... */ }
  QString lockFilename;
  QFile lockFile;
};


LockFile::LockFile(const QString &lockFilename)
  : d_ptr(new LockFilePrivate(lockFilename))
{
  /* ... */
}


LockFile::~LockFile()
{
  unlock();
}


bool LockFile::lock(void)
{
  Q_D(LockFile);
  if (isLocked()) {
    return false;
  }
  d->lockFile.open(QIODevice::ReadWrite | QIODevice::Truncate);
  d->lockFile.write(QByteArray::number(QCoreApplication::applicationPid()) + " " + QCoreApplication::applicationName().toUtf8() + "\n");
  d->lockFile.close();
  return true;
}


void LockFile::unlock(void)
{
  Q_D(LockFile);
  if (isLocked()) {
    d->lockFile.remove();
  }
}


bool LockFile::isLocked(void) const
{
  return QFileInfo(d_ptr->lockFilename).isFile();
}
