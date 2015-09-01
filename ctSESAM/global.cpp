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

const QString AppCompanyName = "c't";
const QString AppName = "Qt SESAM";
const QString AppVersion = "2.0 BETA";
const QString AppURL = "https://github.com/ola-ct/Qt-SESAM";
const QString AppAuthor = "Oliver Lau";
const QString AppAuthorMail = "ola@ct.de";
#if QT_VERSION >= 0x050400
const QString AppUserAgent = QString("%1/%2 (+%3) Qt/%4 (%5; %6, %7)")
    .arg(AppName)
    .arg(AppVersion)
    .arg(AppURL)
    .arg(qVersion())
    .arg(QSysInfo::prettyProductName())
    .arg(QSysInfo::currentCpuArchitecture())
    .arg(QSysInfo::buildCpuArchitecture());
#else
const QString AppUserAgent = QString("%1/%2 (+%3) Qt/%4")
    .arg(AppName)
    .arg(AppVersion)
    .arg(AppURL)
    .arg(qVersion());
#endif
