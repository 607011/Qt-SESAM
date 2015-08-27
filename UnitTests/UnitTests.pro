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

TARGET = ctSESAM-testing

TEMPLATE = app qt

QT += core gui widgets concurrent network testlib

CONFIG += warn_off

win32 {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS CRYPTOPP_DISABLE_ASM CRYPTOPP_MANUALLY_INSTANTIATE_TEMPLATES
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
}

unix {
    QMAKE_CXXFLAGS += -std=c++11
    LIBS += -lcryptopp
}

SOURCES += test-main.cpp \
    ../ctSESAM/3rdparty/bigint/bigInt.cpp \
    ../ctSESAM/domainsettings.cpp \
    ../ctSESAM/password.cpp \
    ../ctSESAM/domainsettingslist.cpp \
    ../ctSESAM/global.cpp \
    ../ctSESAM/crypter.cpp \
    ../ctSESAM/securebytearray.cpp \
    ../ctSESAM/util.cpp

win32:SOURCES += \
    ../ctSESAM/3rdparty/cryptopp562/sha.cpp \
    ../ctSESAM/3rdparty/cryptopp562/iterhash.cpp \
    ../ctSESAM/3rdparty/cryptopp562/misc.cpp \
    ../ctSESAM/3rdparty/cryptopp562/simple.cpp \
    ../ctSESAM/3rdparty/cryptopp562/cryptlib.cpp \
    ../ctSESAM/3rdparty/cryptopp562/cpu.cpp \
    ../ctSESAM/3rdparty/cryptopp562/filters.cpp \
    ../ctSESAM/3rdparty/cryptopp562/queue.cpp \
    ../ctSESAM/3rdparty/cryptopp562/algparam.cpp \
    ../ctSESAM/3rdparty/cryptopp562/fips140.cpp \
    ../ctSESAM/3rdparty/cryptopp562/mqueue.cpp \
    ../ctSESAM/3rdparty/cryptopp562/rijndael.cpp \
    ../ctSESAM/3rdparty/cryptopp562/ccm.cpp \
    ../ctSESAM/3rdparty/cryptopp562/authenc.cpp \
    ../ctSESAM/3rdparty/cryptopp562/modes.cpp \
    ../ctSESAM/3rdparty/cryptopp562/strciphr.cpp \
    ../ctSESAM/3rdparty/cryptopp562/des.cpp \
    ../ctSESAM/3rdparty/cryptopp562/rdtables.cpp \
    ../ctSESAM/3rdparty/cryptopp562/dessp.cpp \
    ../ctSESAM/3rdparty/cryptopp562/rng.cpp \
    ../ctSESAM/3rdparty/cryptopp562/osrng.cpp

HEADERS  += \
    ../ctSESAM/3rdparty/bigint/bigInt.h \
    ../ctSESAM/util.h \
    ../ctSESAM/domainsettings.h \
    ../ctSESAM/password.h \
    ../ctSESAM/domainsettingslist.h \
    ../ctSESAM/global.h \
    ../ctSESAM/hackhelper.h \
    ../ctSESAM/crypter.h \
    ../ctSESAM/securebytearray.h

win32:HEADERS += \
    ../ctSESAM/3rdparty/cryptopp562/sha.h \
    ../ctSESAM/3rdparty/cryptopp562/config.h \
    ../ctSESAM/3rdparty/cryptopp562/cryptlib.h \
    ../ctSESAM/3rdparty/cryptopp562/iterhash.h \
    ../ctSESAM/3rdparty/cryptopp562/misc.h \
    ../ctSESAM/3rdparty/cryptopp562/secblock.h \
    ../ctSESAM/3rdparty/cryptopp562/simple.h \
    ../ctSESAM/3rdparty/cryptopp562/smartptr.h \
    ../ctSESAM/3rdparty/cryptopp562/stdcpp.h \
    ../ctSESAM/3rdparty/cryptopp562/cpu.h \
    ../ctSESAM/3rdparty/cryptopp562/filters.h \
    ../ctSESAM/3rdparty/cryptopp562/queue.h \
    ../ctSESAM/3rdparty/cryptopp562/algparam.h \
    ../ctSESAM/3rdparty/cryptopp562/fips140.h \
    ../ctSESAM/3rdparty/cryptopp562/mqueue.h \
    ../ctSESAM/3rdparty/cryptopp562/aes.h \
    ../ctSESAM/3rdparty/cryptopp562/ccm.h \
    ../ctSESAM/3rdparty/cryptopp562/authenc.h \
    ../ctSESAM/3rdparty/cryptopp562/modes.h \
    ../ctSESAM/3rdparty/cryptopp562/strciphr.h \
    ../ctSESAM/3rdparty/cryptopp562/des.h \
    ../ctSESAM/3rdparty/cryptopp562/rijndael.h \
    ../ctSESAM/3rdparty/cryptopp562/seckey.h \
    ../ctSESAM/3rdparty/cryptopp562/rng.h


INCLUDEPATH += $$PWD/3rdparty
