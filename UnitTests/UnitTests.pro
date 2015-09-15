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

include(../Qt-SESAM.pri)
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\"

TARGET = Qt-SESAM-UnitTests

TEMPLATE = app qt

QT += core concurrent testlib
QT -= gui

CONFIG += console warn_off c++11 testcase no_testcase_installs

win32-msvc* {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    DEFINES -= UNICODE
}

SOURCES += test-main.cpp

HEADERS  +=

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/release/ -lSESAM
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/debug/ -lSESAM
else:unix: LIBS += -L$$OUT_PWD/../libSESAM/ -lSESAM


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/release/ -lSESAM
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/debug/ -lSESAM
else:unix: LIBS += -L$$OUT_PWD/../libSESAM/ -lSESAM

INCLUDEPATH += $$PWD/../libSESAM $$PWD/../libSESAM/3rdparty/cryptopp
DEPENDPATH += $$PWD/../libSESAM

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/release/libSESAM.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/debug/libSESAM.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/release/SESAM.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/debug/SESAM.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/libSESAM.a
