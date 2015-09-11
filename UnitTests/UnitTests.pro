# Copyright (c) 2015 Oliver Lau <ola@ct.de>
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

include(../QtSESAM.pri)
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\"

TARGET = ctSESAM-testing

TEMPLATE = app qt

QT += core gui widgets concurrent network testlib

CONFIG += warn_off c++11 testcase


win32-msvc* {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS CRYPTOPP_DISABLE_ASM CRYPTOPP_MANUALLY_INSTANTIATE_TEMPLATES
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    DEFINES -= UNICODE
}

SOURCES += test-main.cpp \
    ../ctSESAM/3rdparty/bigint/bigInt.cpp \
    ../ctSESAM/domainsettings.cpp \
    ../ctSESAM/pbkdf2.cpp \
    ../ctSESAM/password.cpp \
    ../ctSESAM/domainsettingslist.cpp \
    ../ctSESAM/global.cpp \
    ../ctSESAM/crypter.cpp \
    ../ctSESAM/securebytearray.cpp \
    ../ctSESAM/util.cpp

HEADERS  += \
    ../ctSESAM/3rdparty/bigint/bigInt.h \
    ../ctSESAM/util.h \
    ../ctSESAM/domainsettings.h \
    ../ctSESAM/pbkdf2.h \
    ../ctSESAM/password.h \
    ../ctSESAM/domainsettingslist.h \
    ../ctSESAM/global.h \
    ../ctSESAM/hackhelper.h \
    ../ctSESAM/crypter.h \
    ../ctSESAM/securebytearray.h

INCLUDEPATH += $$PWD/3rdparty


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cryptopp/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../cryptopp/debug/ -lcryptopp
else:unix: LIBS += -L$$OUT_PWD/../cryptopp/ -lcryptopp

INCLUDEPATH += $$PWD/../cryptopp
DEPENDPATH += $$PWD/../cryptopp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/release/libcryptopp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/debug/libcryptopp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/release/cryptopp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/debug/cryptopp.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/libcryptopp.a
