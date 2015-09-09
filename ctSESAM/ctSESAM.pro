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

TARGET = ctSESAM

TEMPLATE = app qt

QT += core gui widgets concurrent network

TRANSLATIONS += translations/i18n_de.ts

win32 {
    CONFIG += warn_off
    CONFIG += windows
    CONFIG -= console
    DEFINES += _SCL_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS_DEBUG += -sdl
    QMAKE_CXXFLAGS_RELEASE += -GA -GL -Ox
    RC_FILE = ctSESAM.rc
    SOURCES += dump.cpp
    HEADERS += dump.h
    LIBS += User32.lib
    QMAKE_LFLAGS += /LTCG
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:WINDOWS
}

unix {
    QMAKE_CXXFLAGS += -std=c++11
    LIBS += -lcryptopp
}

SOURCES += main.cpp \
    mainwindow.cpp \
    3rdparty/bigint/bigInt.cpp \
    domainsettings.cpp \
    optionsdialog.cpp \
    progressdialog.cpp \
    domainsettingslist.cpp \
    global.cpp \
    newdomainwizard.cpp \
    masterpassworddialog.cpp \
    crypter.cpp \
    securebytearray.cpp \
    servercertificatewidget.cpp \
    util.cpp \
    pbkdf2.cpp \
    password.cpp \
    changemasterpassworddialog.cpp \
    passwordchecker.cpp

HEADERS  += \
    mainwindow.h \
    3rdparty/bigint/bigInt.h \
    util.h \
    domainsettings.h \
    optionsdialog.h \
    progressdialog.h \
    domainsettingslist.h \
    global.h \
    newdomainwizard.h \
    masterpassworddialog.h \
    hackhelper.h \
    crypter.h \
    securebytearray.h \
    servercertificatewidget.h \
    pbkdf2.h \
    password.h \
    changemasterpassworddialog.h \
    passwordchecker.h

FORMS += mainwindow.ui \
    optionsdialog.ui \
    progressdialog.ui \
    newcredentialsdialog.ui \
    newdomainwizard.ui \
    masterpassworddialog.ui \
    servercertificatewidget.ui \
    changemasterpassworddialog.ui

RESOURCES += \
    ctSESAM.qrc

DISTFILES += \
    LICENSE \
    ../README.md \
    ctSESAM.rc \
    ../LIESMICH.txt

OTHER_FILES += \
    translations/i18n_de.ts \
    .gitignore \
    ../deploy/ctSESAM.nsi \
    ../.gitignore \
    Doxyfile \
    deploy/ctSESAM.nsi \

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cryptopp/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../cryptopp/debug/ -lcryptopp
else:macx: LIBS += -L$$OUT_PWD/../cryptopp/ -lcryptopp

INCLUDEPATH += $$PWD/../cryptopp
DEPENDPATH += $$PWD/../cryptopp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/release/libcryptopp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/debug/libcryptopp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/release/cryptopp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/debug/cryptopp.lib
else:macx: PRE_TARGETDEPS += $$OUT_PWD/../cryptopp/libcryptopp.a
