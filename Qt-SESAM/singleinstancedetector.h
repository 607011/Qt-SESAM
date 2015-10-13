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


#ifndef __SINGLEINSTANCEDETECTOR_H_
#define __SINGLEINSTANCEDETECTOR_H_

#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QSharedMemory>
#include "global.h"
#include "util.h"

class SingleInstanceDetector
{
public:
  static SingleInstanceDetector &instance(void)
  {
    static SingleInstanceDetector singleInstance;
    return singleInstance;
  }


  bool attach(void)
  {
    if (sharedMem == Q_NULLPTR)
      sharedMem = new QSharedMemory(AppName);
    return sharedMem->attach(QSharedMemory::ReadOnly);
  }


  bool alreadyRunning(void)
  {
    if (isRunning)
      return true;
    if (sharedMem == Q_NULLPTR)
      sharedMem = new QSharedMemory(AppName);
    if (sharedMem->create(1, QSharedMemory::ReadOnly)) {
      attach();
      return false;
    }
    isRunning = true;
    return true;
  }


  void detach(void)
  {
    if (sharedMem != Q_NULLPTR)
      sharedMem->detach();
    SafeDelete(sharedMem);
  }


private:
  SingleInstanceDetector(void)
    : sharedMem(Q_NULLPTR)
    , isRunning(false)
  { /* ... */ }
  ~SingleInstanceDetector()
  {
    detach();
  }
  /* Windows: QSharedMemory does not "own"
   * the shared memory segment. When all threads or processes that
   * have an instance of QSharedMemory attached to a particular
   * shared memory segment have either destroyed their instance of
   * QSharedMemory or exited, the Windows kernel releases the
   * shared memory segment automatically.
   *
   * Unix: QSharedMemory "owns" the shared memory segment. When
   * the last thread or process that has an instance of
   * QSharedMemory attached to a particular shared memory segment
   * detaches from the segment by destroying its instance of
   * QSharedMemory, the Unix kernel release the shared memory
   * segment. But if that last thread or process crashes without
   * running the QSharedMemory destructor, the shared memory
   * segment survives the crash.
   */
  QSharedMemory *sharedMem;
  bool isRunning;
};

#endif // __SINGLEINSTANCEDETECTOR_H_
