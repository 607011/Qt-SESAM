# Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG
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
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\" HAVE_CONFIG_H

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

TARGET = qrencode
TEMPLATE = lib

CONFIG += staticlib warn_off

win32-msvc* {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    DEFINES -= UNICODE
}


SOURCES += \
    bitstream.c \
    mask.c \
    mmask.c \
    mqrspec.c \
    qrencode.c \
    qrinput.c \
    qrspec.c \
    rscode.c \
    split.c

HEADERS +=\
    bitstream.h \
    config.h \
    mask.h \
    mmask.h \
    mqrspec.h \
    qrencode.h \
    qrencode_inner.h \
    qrinput.h \
    qrspec.h \
    rscode.h \
    split.h

