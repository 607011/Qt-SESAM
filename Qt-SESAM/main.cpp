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
#include <QApplication>

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(QtSESAM);

  checkPortable();

  bool forceStart = argc > 1 && qstrcmp(argv[1], "--force-start") == 0;

  QApplication a(argc, argv);
  a.setOrganizationName(AppCompanyName);
  a.setOrganizationDomain(AppCompanyDomain);
  a.setApplicationName(AppName);
  a.setApplicationVersion(AppVersion);
  a.setQuitOnLastWindowClosed(true);

  MainWindow w(forceStart);
  w.activateWindow();
  return a.exec();
}
