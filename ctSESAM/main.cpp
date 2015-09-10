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

#include "mainwindow.h"
#include "global.h"
#include <QDebug>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QString>
#include <QSettings>
#include <QDir>
#include <QFileInfo>


void detectPortable(void)
{
  static const QString PortableFlagFile = "PORTABLE";
  QString cwdSettingsFileName = QDir::currentPath() + "/" + PortableFlagFile;
  QFileInfo fi(cwdSettingsFileName);
  if (fi.isFile() && fi.isWritable())
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::currentPath());
}


int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(ctSESAM);

  detectPortable();

  QApplication a(argc, argv);
  a.setOrganizationName(AppCompanyName);
  a.setOrganizationDomain(AppCompanyName);
  a.setApplicationName(AppName);
  a.setApplicationVersion(AppVersion);

  QTranslator translator;
  bool ok = translator.load(":/translations/i18n_" + QLocale::system().name());
#ifdef QT_DEBUG
  if (!ok)
    qWarning() << "Could not load translations for" << QLocale::system().name() << "locale";
#endif
  if (ok)
    a.installTranslator(&translator);

  MainWindow w;
  w.activateWindow();

  return a.exec();
}
