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
#include <QDir>
#include <QFile>
#include <QtGlobal>

class LoggerPrivate {
public:
  LoggerPrivate(void)
    : enabled(true)
  { /* ... */ }
  QFile file;
  bool enabled;
};


Logger::Logger(void)
  : d_ptr(new LoggerPrivate)
{
  const QString &logFilePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  QDir().mkpath(logFilePath);
  setFileName(QString("%1/%2.log").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation)).arg(AppName));
}


Logger::~Logger()
{
  Q_D(Logger);
  log(QString("Logger shutting down ..."));
  d->file.close();
}


Logger &Logger::instance(void)
{
  static Logger logger;
  return logger;
}


void Logger::log(const QString &message)
{
  Q_D(Logger);
  if (d->enabled) {
    const QByteArray &logMsg = QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
        .arg(message).toUtf8();
    if (d->file.isOpen()) {
      d->file.write(logMsg);
      d->file.write("\n");
      d->file.flush();
    }
    else {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
      qDebug().noquote().nospace() << logMsg;
#else
      qDebug().nospace() << logMsg;
#endif
    }
  }
}


void Logger::setEnabled(bool enabled)
{
  Q_D(Logger);
  d->enabled = enabled;
}


void Logger::setFileName(const QString &filename)
{
  Q_D(Logger);
  if (d->file.isOpen()) {
    d->file.close();
  }
  d->file.setFileName(filename);
  d->file.open(QIODevice::WriteOnly | QIODevice::Append);
  log(QString("Logger writing to %1").arg(filename));
}
