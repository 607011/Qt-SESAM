# Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT -= gui
QT += concurrent

include(../Qt-SESAM.pri)
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\"

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

TARGET = SESAM
TEMPLATE = lib

CONFIG += staticlib warn_off

win32-msvc* {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    DEFINES -= UNICODE
}

include(3rdparty/bigint/bigint.pri)
include(3rdparty/cryptopp/cryptopp.pri)

SOURCES += \
    util.cpp \
    crypter.cpp \
    domainsettings.cpp \
    domainsettingslist.cpp \
    password.cpp \
    pbkdf2.cpp \
    securebytearray.cpp \
    securestring.cpp \
    exporter.cpp

HEADERS +=\
    util.h \
    crypter.h \
    domainsettings.h \
    domainsettingslist.h \
    password.h \
    pbkdf2.h \
    securebytearray.h \
    securestring.h \
    exporter.h

DISTFILES += \
    3rdparty/cryptopp/Crypto++-License
