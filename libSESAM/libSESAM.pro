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
DEFINES += QTSESAM_VERSION=\\\"$${QTSESAM_VERSION}\\\"

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0

TARGET = SESAM
TEMPLATE = lib

CONFIG += staticlib warn_off c++11

unix:QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += 3rdparty/cryptopp 3rdparty/bigint

win32-msvc* {
    QMAKE_CXXFLAGS += /wd4100
    DEFINES += _SCL_SECURE_NO_WARNINGS
    LIBS += User32.lib
    QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO
    DEFINES -= UNICODE
}

SOURCES += \
    util.cpp \
    crypter.cpp \
    domainsettings.cpp \
    domainsettingslist.cpp \
    password.cpp \
    pbkdf2.cpp \
    securebytearray.cpp \
    3rdparty/bigint/bigInt.cpp \
    3rdparty/cryptopp/3way.cpp \
    3rdparty/cryptopp/adler32.cpp \
    3rdparty/cryptopp/algebra.cpp \
    3rdparty/cryptopp/algparam.cpp \
    3rdparty/cryptopp/arc4.cpp \
    3rdparty/cryptopp/asn.cpp \
    3rdparty/cryptopp/authenc.cpp \
    3rdparty/cryptopp/base32.cpp \
    3rdparty/cryptopp/base64.cpp \
    3rdparty/cryptopp/basecode.cpp \
    3rdparty/cryptopp/bench.cpp \
    3rdparty/cryptopp/bench2.cpp \
    3rdparty/cryptopp/bfinit.cpp \
    3rdparty/cryptopp/blowfish.cpp \
    3rdparty/cryptopp/blumshub.cpp \
    3rdparty/cryptopp/camellia.cpp \
    3rdparty/cryptopp/cast.cpp \
    3rdparty/cryptopp/casts.cpp \
    3rdparty/cryptopp/cbcmac.cpp \
    3rdparty/cryptopp/ccm.cpp \
    3rdparty/cryptopp/channels.cpp \
    3rdparty/cryptopp/cmac.cpp \
    3rdparty/cryptopp/cpu.cpp \
    3rdparty/cryptopp/crc.cpp \
    3rdparty/cryptopp/cryptlib.cpp \
    3rdparty/cryptopp/cryptlib_bds.cpp \
    3rdparty/cryptopp/datatest.cpp \
    3rdparty/cryptopp/default.cpp \
    3rdparty/cryptopp/des.cpp \
    3rdparty/cryptopp/dessp.cpp \
    3rdparty/cryptopp/dh.cpp \
    3rdparty/cryptopp/dh2.cpp \
    3rdparty/cryptopp/dll.cpp \
    3rdparty/cryptopp/dlltest.cpp \
    3rdparty/cryptopp/dsa.cpp \
    3rdparty/cryptopp/eax.cpp \
    3rdparty/cryptopp/ec2n.cpp \
    3rdparty/cryptopp/eccrypto.cpp \
    3rdparty/cryptopp/ecp.cpp \
    3rdparty/cryptopp/elgamal.cpp \
    3rdparty/cryptopp/emsa2.cpp \
    3rdparty/cryptopp/eprecomp.cpp \
    3rdparty/cryptopp/esign.cpp \
    3rdparty/cryptopp/files.cpp \
    3rdparty/cryptopp/filters.cpp \
    3rdparty/cryptopp/fips140.cpp \
    3rdparty/cryptopp/fipsalgt.cpp \
    3rdparty/cryptopp/gcm.cpp \
    3rdparty/cryptopp/gf2_32.cpp \
    3rdparty/cryptopp/gf2n.cpp \
    3rdparty/cryptopp/gf256.cpp \
    3rdparty/cryptopp/gfpcrypt.cpp \
    3rdparty/cryptopp/gost.cpp \
    3rdparty/cryptopp/gzip.cpp \
    3rdparty/cryptopp/hex.cpp \
    3rdparty/cryptopp/hmac.cpp \
    3rdparty/cryptopp/hrtimer.cpp \
    3rdparty/cryptopp/ida.cpp \
    3rdparty/cryptopp/idea.cpp \
    3rdparty/cryptopp/integer.cpp \
    3rdparty/cryptopp/iterhash.cpp \
    3rdparty/cryptopp/luc.cpp \
    3rdparty/cryptopp/mars.cpp \
    3rdparty/cryptopp/marss.cpp \
    3rdparty/cryptopp/md2.cpp \
    3rdparty/cryptopp/md4.cpp \
    3rdparty/cryptopp/md5.cpp \
    3rdparty/cryptopp/misc.cpp \
    3rdparty/cryptopp/modes.cpp \
    3rdparty/cryptopp/mqueue.cpp \
    3rdparty/cryptopp/mqv.cpp \
    3rdparty/cryptopp/nbtheory.cpp \
    3rdparty/cryptopp/network.cpp \
    3rdparty/cryptopp/oaep.cpp \
    3rdparty/cryptopp/osrng.cpp \
    3rdparty/cryptopp/panama.cpp \
    3rdparty/cryptopp/pch.cpp \
    3rdparty/cryptopp/pkcspad.cpp \
    3rdparty/cryptopp/polynomi.cpp \
    3rdparty/cryptopp/pssr.cpp \
    3rdparty/cryptopp/pubkey.cpp \
    3rdparty/cryptopp/queue.cpp \
    3rdparty/cryptopp/rabin.cpp \
    3rdparty/cryptopp/randpool.cpp \
    3rdparty/cryptopp/rc2.cpp \
    3rdparty/cryptopp/rc5.cpp \
    3rdparty/cryptopp/rc6.cpp \
    3rdparty/cryptopp/rdtables.cpp \
    3rdparty/cryptopp/regtest.cpp \
    3rdparty/cryptopp/rijndael.cpp \
    3rdparty/cryptopp/ripemd.cpp \
    3rdparty/cryptopp/rng.cpp \
    3rdparty/cryptopp/rsa.cpp \
    3rdparty/cryptopp/rw.cpp \
    3rdparty/cryptopp/safer.cpp \
    3rdparty/cryptopp/salsa.cpp \
    3rdparty/cryptopp/seal.cpp \
    3rdparty/cryptopp/seed.cpp \
    3rdparty/cryptopp/serpent.cpp \
    3rdparty/cryptopp/sha.cpp \
    3rdparty/cryptopp/sha3.cpp \
    3rdparty/cryptopp/shacal2.cpp \
    3rdparty/cryptopp/shark.cpp \
    3rdparty/cryptopp/sharkbox.cpp \
    3rdparty/cryptopp/skipjack.cpp \
    3rdparty/cryptopp/socketft.cpp \
    3rdparty/cryptopp/sosemanuk.cpp \
    3rdparty/cryptopp/square.cpp \
    3rdparty/cryptopp/squaretb.cpp \
    3rdparty/cryptopp/strciphr.cpp \
    3rdparty/cryptopp/tea.cpp \
    3rdparty/cryptopp/tftables.cpp \
    3rdparty/cryptopp/tiger.cpp \
    3rdparty/cryptopp/tigertab.cpp \
    3rdparty/cryptopp/trdlocal.cpp \
    3rdparty/cryptopp/ttmac.cpp \
    3rdparty/cryptopp/twofish.cpp \
    3rdparty/cryptopp/validat0.cpp \
    3rdparty/cryptopp/validat1.cpp \
    3rdparty/cryptopp/validat2.cpp \
    3rdparty/cryptopp/validat3.cpp \
    3rdparty/cryptopp/vmac.cpp \
    3rdparty/cryptopp/wait.cpp \
    3rdparty/cryptopp/wake.cpp \
    3rdparty/cryptopp/whrlpool.cpp \
    3rdparty/cryptopp/winpipes.cpp \
    3rdparty/cryptopp/xtr.cpp \
    3rdparty/cryptopp/xtrcrypt.cpp \
    3rdparty/cryptopp/zdeflate.cpp \
    3rdparty/cryptopp/zinflate.cpp \
    3rdparty/cryptopp/zlib.cpp

HEADERS +=\
    util.h \
    crypter.h \
    domainsettings.h \
    domainsettingslist.h \
    password.h \
    pbkdf2.h \
    securebytearray.h \
    3rdparty/bigint/bigInt.h \
    3rdparty/cryptopp/3way.h \
    3rdparty/cryptopp/adler32.h \
    3rdparty/cryptopp/aes.h \
    3rdparty/cryptopp/algebra.h \
    3rdparty/cryptopp/algparam.h \
    3rdparty/cryptopp/arc4.h \
    3rdparty/cryptopp/argnames.h \
    3rdparty/cryptopp/asn.h \
    3rdparty/cryptopp/authenc.h \
    3rdparty/cryptopp/base32.h \
    3rdparty/cryptopp/base64.h \
    3rdparty/cryptopp/basecode.h \
    3rdparty/cryptopp/bench.h \
    3rdparty/cryptopp/blowfish.h \
    3rdparty/cryptopp/blumshub.h \
    3rdparty/cryptopp/camellia.h \
    3rdparty/cryptopp/cast.h \
    3rdparty/cryptopp/cbcmac.h \
    3rdparty/cryptopp/ccm.h \
    3rdparty/cryptopp/channels.h \
    3rdparty/cryptopp/cmac.h \
    3rdparty/cryptopp/config.h \
    3rdparty/cryptopp/cpu.h \
    3rdparty/cryptopp/crc.h \
    3rdparty/cryptopp/cryptlib.h \
    3rdparty/cryptopp/default.h \
    3rdparty/cryptopp/des.h \
    3rdparty/cryptopp/dh.h \
    3rdparty/cryptopp/dh2.h \
    3rdparty/cryptopp/dll.h \
    3rdparty/cryptopp/dmac.h \
    3rdparty/cryptopp/dsa.h \
    3rdparty/cryptopp/eax.h \
    3rdparty/cryptopp/ec2n.h \
    3rdparty/cryptopp/eccrypto.h \
    3rdparty/cryptopp/ecp.h \
    3rdparty/cryptopp/elgamal.h \
    3rdparty/cryptopp/emsa2.h \
    3rdparty/cryptopp/eprecomp.h \
    3rdparty/cryptopp/esign.h \
    3rdparty/cryptopp/factory.h \
    3rdparty/cryptopp/files.h \
    3rdparty/cryptopp/filters.h \
    3rdparty/cryptopp/fips140.h \
    3rdparty/cryptopp/fltrimpl.h \
    3rdparty/cryptopp/gcm.h \
    3rdparty/cryptopp/gf2_32.h \
    3rdparty/cryptopp/gf2n.h \
    3rdparty/cryptopp/gf256.h \
    3rdparty/cryptopp/gfpcrypt.h \
    3rdparty/cryptopp/gost.h \
    3rdparty/cryptopp/gzip.h \
    3rdparty/cryptopp/hex.h \
    3rdparty/cryptopp/hkdf.h \
    3rdparty/cryptopp/hmac.h \
    3rdparty/cryptopp/hrtimer.h \
    3rdparty/cryptopp/ida.h \
    3rdparty/cryptopp/idea.h \
    3rdparty/cryptopp/integer.h \
    3rdparty/cryptopp/iterhash.h \
    3rdparty/cryptopp/lubyrack.h \
    3rdparty/cryptopp/luc.h \
    3rdparty/cryptopp/mars.h \
    3rdparty/cryptopp/md2.h \
    3rdparty/cryptopp/md4.h \
    3rdparty/cryptopp/md5.h \
    3rdparty/cryptopp/mdc.h \
    3rdparty/cryptopp/misc.h \
    3rdparty/cryptopp/modarith.h \
    3rdparty/cryptopp/modes.h \
    3rdparty/cryptopp/modexppc.h \
    3rdparty/cryptopp/mqueue.h \
    3rdparty/cryptopp/mqv.h \
    3rdparty/cryptopp/nbtheory.h \
    3rdparty/cryptopp/network.h \
    3rdparty/cryptopp/nr.h \
    3rdparty/cryptopp/oaep.h \
    3rdparty/cryptopp/oids.h \
    3rdparty/cryptopp/osrng.h \
    3rdparty/cryptopp/panama.h \
    3rdparty/cryptopp/pch.h \
    3rdparty/cryptopp/pkcspad.h \
    3rdparty/cryptopp/polynomi.h \
    3rdparty/cryptopp/pssr.h \
    3rdparty/cryptopp/pubkey.h \
    3rdparty/cryptopp/pwdbased.h \
    3rdparty/cryptopp/queue.h \
    3rdparty/cryptopp/rabin.h \
    3rdparty/cryptopp/randpool.h \
    3rdparty/cryptopp/rc2.h \
    3rdparty/cryptopp/rc5.h \
    3rdparty/cryptopp/rc6.h \
    3rdparty/cryptopp/resource.h \
    3rdparty/cryptopp/rijndael.h \
    3rdparty/cryptopp/ripemd.h \
    3rdparty/cryptopp/rng.h \
    3rdparty/cryptopp/rsa.h \
    3rdparty/cryptopp/rw.h \
    3rdparty/cryptopp/safer.h \
    3rdparty/cryptopp/salsa.h \
    3rdparty/cryptopp/seal.h \
    3rdparty/cryptopp/secblock.h \
    3rdparty/cryptopp/seckey.h \
    3rdparty/cryptopp/seed.h \
    3rdparty/cryptopp/serpent.h \
    3rdparty/cryptopp/serpentp.h \
    3rdparty/cryptopp/sha.h \
    3rdparty/cryptopp/sha3.h \
    3rdparty/cryptopp/shacal2.h \
    3rdparty/cryptopp/shark.h \
    3rdparty/cryptopp/simple.h \
    3rdparty/cryptopp/skipjack.h \
    3rdparty/cryptopp/smartptr.h \
    3rdparty/cryptopp/socketft.h \
    3rdparty/cryptopp/sosemanuk.h \
    3rdparty/cryptopp/square.h \
    3rdparty/cryptopp/stdcpp.h \
    3rdparty/cryptopp/strciphr.h \
    3rdparty/cryptopp/tea.h \
    3rdparty/cryptopp/tiger.h \
    3rdparty/cryptopp/trap.h \
    3rdparty/cryptopp/trdlocal.h \
    3rdparty/cryptopp/trunhash.h \
    3rdparty/cryptopp/ttmac.h \
    3rdparty/cryptopp/twofish.h \
    3rdparty/cryptopp/validate.h \
    3rdparty/cryptopp/vmac.h \
    3rdparty/cryptopp/wait.h \
    3rdparty/cryptopp/wake.h \
    3rdparty/cryptopp/whrlpool.h \
    3rdparty/cryptopp/winpipes.h \
    3rdparty/cryptopp/words.h \
    3rdparty/cryptopp/xtr.h \
    3rdparty/cryptopp/xtrcrypt.h \
    3rdparty/cryptopp/zdeflate.h \
    3rdparty/cryptopp/zinflate.h \
    3rdparty/cryptopp/zlib.h

DISTFILES += \
    3rdparty/cryptopp/Crypto++-License
