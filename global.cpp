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


#include "global.h"
#include <QSysInfo>

const QString APP_COMPANY_NAME = "c't";
const QString APP_NAME = "Qt SESAM";
const QString APP_VERSION = "1.0";
const QString APP_URL = "https://github.com/ola-ct/Qt-SESAM";
const QString APP_AUTHOR = "Oliver Lau";
const QString APP_AUTHOR_MAIL = "ola@ct.de";
#if QT_VERSION >= 0x050400
const QString APP_USER_AGENT = QString("%1/%2 (+%3) Qt/%4 (%5; %6, %7)")
    .arg(APP_NAME)
    .arg(APP_VERSION)
    .arg(APP_URL)
    .arg(qVersion())
    .arg(QSysInfo::prettyProductName())
    .arg(QSysInfo::currentCpuArchitecture())
    .arg(QSysInfo::buildCpuArchitecture());
#else
const QString APP_USER_AGENT = QString("%1/%2 (+%3) Qt/%4")
    .arg(APP_NAME)
    .arg(APP_VERSION)
    .arg(APP_URL)
    .arg(qVersion());
#endif


std::random_device gRandomDevice;
