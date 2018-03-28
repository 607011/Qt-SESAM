# Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>
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

include(Qt-SESAM.pri)
VERSION = -$${QTSESAM_VERSION}

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += \
    libqrencode \
    libSESAM \
    SESAM2Chrome \
    Qt-SESAM \
    UnitTests

OTHER_FILES += \
    extensions\chrome\background.js \
    extensions\chrome\content.js \
    extensions\chrome\popup.js \
    extensions\chrome\popup.html \
    extensions\chrome\domains.json \
    extensions\chrome\default.css \
    extensions\chrome\manifest.json \
    extensions\firefox\index.js \
    extensions\firefox\README.md \
    extensions\firefox\package.json
