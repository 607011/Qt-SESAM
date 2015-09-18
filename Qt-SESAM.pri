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

QTSESAM_VERSION = 2.0.5

DEFINES += CRYPTOPP_DISABLE_X86ASM CRYPTOPP_DISABLE_SSSE3

contains(QT_ARCH, i386) {
    DEFINES += PLATFORM=32
} else {
    DEFINES += PLATFORM=64
}

unix:QMAKE_CXXFLAGS += -std=c++11

CONFIG += c++11
