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

VERSION = -$${QTSESAM_VERSION}

TARGET = Qt-SESAM

TEMPLATE = app qt

QT += core gui widgets concurrent network

TRANSLATIONS = $$files(translations/i18n_*.ts)
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/translations
ts-to-qm.name     = lrelease ${QMAKE_FILE_IN}
ts-to-qm.input    = TRANSLATIONS
ts-to-qm.output   = $$QM_DIR/${QMAKE_FILE_BASE}.qm
ts-to-qm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
ts-to-qm.CONFIG   = no_link
QMAKE_EXTRA_COMPILERS += ts-to-qm
PRE_TARGETDEPS += compiler_ts-to-qm_make_all


CONFIG += c++11

VERSION_PE_HEADER = 2.0

win32-g++ {
    CONFIG += warn_off
    CONFIG += windows
    CONFIG -= console
    QMAKE_CXXFLAGS_RELEASE += -O3
}

win32-msvc* {
    CONFIG += warn_off
    CONFIG += windows
    CONFIG -= console
    DEFINES += _SCL_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS_DEBUG += /sdl
    QMAKE_CXXFLAGS_RELEASE += /GA /GL /Ox
    RC_FILE = QtSESAM.rc
    SOURCES += dump.cpp
    HEADERS += dump.h
    LIBS += User32.lib
    QMAKE_LFLAGS += /LTCG
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    QMAKE_LFLAGS_CONSOLE = /SUBSYSTEM:WINDOWS
}

SOURCES += main.cpp \
    mainwindow.cpp \
    optionsdialog.cpp \
    progressdialog.cpp \
    global.cpp \
    newdomainwizard.cpp \
    masterpassworddialog.cpp \
    servercertificatewidget.cpp \
    changemasterpassworddialog.cpp \
    passwordchecker.cpp

HEADERS  += \
    mainwindow.h \
    optionsdialog.h \
    progressdialog.h \
    global.h \
    newdomainwizard.h \
    masterpassworddialog.h \
    hackhelper.h \
    servercertificatewidget.h \
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
    QtSESAM.qrc

DISTFILES += \
    ../LICENSE \
    ../README.md \
    ../LIESMICH.txt \
    QtSESAM.rc

OTHER_FILES += \
    translations/i18n_de.ts \
    .gitignore \
    ../deploy/Qt-SESAM.nsi \
    ../.gitignore \
    Doxyfile \

unix {
    target.path = /bin
    INSTALLS += target
}

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


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/release/ -lSESAM
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libSESAM/debug/ -lSESAM
else:unix: LIBS += -L$$OUT_PWD/../libSESAM/ -lSESAM

INCLUDEPATH += $$PWD/../libSESAM
DEPENDPATH += $$PWD/../libSESAM

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/release/libSESAM.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/debug/libSESAM.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/release/SESAM.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/debug/SESAM.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libSESAM/libSESAM.a
