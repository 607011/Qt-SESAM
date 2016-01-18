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


#include "logger.h"
#include "global.h"

#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QFile>


Logger::Logger(void)
  : mEnabled(true)
{
  /* ... */
}


Logger::~Logger()
{
  log(QString("Logger shutting down ..."));
  mFile.close();
}


Logger &Logger::instance(void)
{
  static Logger logger;
  return logger;
}


void Logger::log(const QString &message)
{
  qDebug() << message;
  if (mEnabled && mFile.isOpen()) {
    mFile.write(QString("[%1] %2\n")
                .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                .arg(message).toUtf8());
    mFile.flush();
  }
}


void Logger::setEnabled(bool enabled)
{
  mEnabled = enabled;
}


void Logger::setFileName(const QString &filename)
{
  mFile.setFileName(filename);
  mFile.open(QIODevice::WriteOnly | QIODevice::Append);
  log(QString("Logger writing to %1").arg(filename));
}
