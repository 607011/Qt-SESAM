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


#ifndef __LOCKFILE_H_
#define __LOCKFILE_H_

#include <QString>
#include <QScopedPointer>

class LockFilePrivate;

class LockFile
{
public:
  LockFile(const QString &lockFilename);
  ~LockFile();

  bool lock(void);
  void unlock(void);
  bool isLocked(void) const;

private:
  QScopedPointer<LockFilePrivate> d_ptr;
  Q_DECLARE_PRIVATE(LockFile)
  Q_DISABLE_COPY(LockFile)
};

#endif // __LOCKFILE_H_
