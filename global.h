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


#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <random>
#include <QString>

#ifndef QT_DEBUG
#define EXEC_SPEED_TEST
#endif

#define FIXED_SHA512 (1)
#define SHA512_DIGEST_SIZE (512)

extern const QString APP_COMPANY_NAME;
extern const QString APP_NAME;
extern const QString APP_VERSION;
extern const QString APP_URL;
extern const QString APP_AUTHOR;
extern const QString APP_AUTHOR_MAIL;
extern const QString APP_USER_AGENT;

extern std::random_device gRandomDevice;


#endif // __GLOBAL_H_
