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
#include <QStringList>
#include <QCoreApplication>


static const int NoApplicationId = -1;


class LockFilePrivate {
public:
  LockFilePrivate(const QString &lockFilename)
    : lockFilename(lockFilename)
    , lockFile(lockFilename)
    , applicationId(NoApplicationId)
  { /* ... */ }
  ~LockFilePrivate()
  { /* ... */ }
  QString lockFilename;
  QFile lockFile;
  QString applicationName;
  int applicationId;
};



LockFile::LockFile(const QString &lockFilename)
  : d_ptr(new LockFilePrivate(lockFilename))
{
  checkExtractInfo();
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
    d->applicationId = NoApplicationId;
  }
}


bool LockFile::isLocked(void) const
{
  return QFileInfo(d_ptr->lockFilename).isFile();
}


void LockFile::extractInfo(void)
{
  Q_D(LockFile);
  d->applicationId = NoApplicationId;
  d->applicationName.clear();
  d->lockFile.open(QIODevice::ReadOnly);
  QString line = d->lockFile.readLine();
  d->lockFile.close();
  QStringList parts = line.split(' ', QString::SkipEmptyParts);
  if (parts.count() == 2) {
    bool ok = false;
    int appId = parts.at(0).toInt(&ok);
    if (ok) {
      d->applicationId = appId;
    }
    d->applicationName = parts.at(1).trimmed();
  }
}


void LockFile::checkExtractInfo(void)
{
  Q_D(LockFile);
  if (isLocked() && (d->applicationId == NoApplicationId || d->applicationName.isEmpty())) {
    extractInfo();
  }
}


int LockFile::applicationId(void)
{
  Q_D(LockFile);
  checkExtractInfo();
  return d->applicationId;
}


QString LockFile::applicationName(void)
{
  Q_D(LockFile);
  checkExtractInfo();
  return d->applicationName;
}
