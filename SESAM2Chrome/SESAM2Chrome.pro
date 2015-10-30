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

TEMPLATE = app

include(../Qt-SESAM.pri)
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\"

QT += core network concurrent
QT -= gui

TARGET = SESAM2Chrome
CONFIG += console
CONFIG -= app_bundle

win32:DEFINES -= UNICODE

SOURCES += main.cpp \
    tcpserver.cpp \
    messenger.cpp

HEADERS += \
    tcpserver.h \
    messenger.h

OTHER_FILES += \
    extension/background.js \
    extension/manifest.json \
    extension/popup.html \
    extension/content.js \
    extension/domains.json \
    extension/popup.js \
    manifest-dev.json \
    extension/default.css
