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

TARGET = pwdgen

TEMPLATE = app qt

QT += core gui widgets concurrent

CONFIG += warn_off

win32 {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS CRYPTOPP_DISABLE_ASM
    RC_FILE = ctpwdgen.rc
}


CONFIG(debug) {
    QT += testlib
    SOURCES += testpbkdf2.cpp
    HEADERS += testpbkdf2.h
}

SOURCES += main.cpp mainwindow.cpp \
    3rdparty/bigint/bigInt.cpp \
    3rdparty/cryptopp562/sha.cpp \
    3rdparty/cryptopp562/iterhash.cpp \
    3rdparty/cryptopp562/misc.cpp \
    3rdparty/cryptopp562/simple.cpp \
    3rdparty/cryptopp562/hmac.cpp \
    3rdparty/cryptopp562/cryptlib.cpp \
    3rdparty/cryptopp562/cpu.cpp \
    3rdparty/cryptopp562/filters.cpp \
    3rdparty/cryptopp562/queue.cpp \
    3rdparty/cryptopp562/algparam.cpp \
    3rdparty/cryptopp562/fips140.cpp \
    3rdparty/cryptopp562/mqueue.cpp \
    domainsettings.cpp \
    password.cpp

HEADERS  += mainwindow.h \
    3rdparty/bigint/bigInt.h \
    3rdparty/cryptopp562/sha.h \
    3rdparty/cryptopp562/config.h \
    3rdparty/cryptopp562/cryptlib.h \
    3rdparty/cryptopp562/iterhash.h \
    3rdparty/cryptopp562/misc.h \
    3rdparty/cryptopp562/secblock.h \
    3rdparty/cryptopp562/simple.h \
    3rdparty/cryptopp562/smartptr.h \
    3rdparty/cryptopp562/stdcpp.h \
    3rdparty/cryptopp562/hmac.h \
    3rdparty/cryptopp562/cpu.h \
    3rdparty/cryptopp562/filters.h \
    3rdparty/cryptopp562/queue.h \
    3rdparty/cryptopp562/algparam.h \
    3rdparty/cryptopp562/fips140.h \
    3rdparty/cryptopp562/mqueue.h \
    util.h \
    domainsettings.h \
    password.h

FORMS    += mainwindow.ui


INCLUDEPATH += $$PWD/3rdparty

RESOURCES += \
    pwdgen.qrc

DISTFILES += \
    .gitignore \
    LICENSE \
    README.md \
    ctpwdgen.rc \
    deploy/ctpwdgen.nsi
