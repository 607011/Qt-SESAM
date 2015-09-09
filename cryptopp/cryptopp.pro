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

QT -= core gui

TARGET = cryptopp
TEMPLATE = lib
CONFIG += staticlib warn_off

win32 {
    QMAKE_CXXFLAGS += -wd4100
    QMAKE_CXXFLAGS_DEBUG += -sdl
    QMAKE_CXXFLAGS_RELEASE += -GA -GL -Ox
    DEFINES += _SCL_SECURE_NO_WARNINGS
    DEFINES -= UNICODE
}

DEFINES += CRYPTOPP_DISABLE_ASM


SOURCES += \
    3way.cpp \
    adler32.cpp \
    algebra.cpp \
    algparam.cpp \
    arc4.cpp \
    asn.cpp \
    authenc.cpp \
    base32.cpp \
    base64.cpp \
    basecode.cpp \
    bench.cpp \
    bench2.cpp \
    bfinit.cpp \
    blowfish.cpp \
    blumshub.cpp \
    camellia.cpp \
    cast.cpp \
    casts.cpp \
    cbcmac.cpp \
    ccm.cpp \
    channels.cpp \
    cmac.cpp \
    cpu.cpp \
    crc.cpp \
    cryptlib.cpp \
    cryptlib_bds.cpp \
    datatest.cpp \
    default.cpp \
    des.cpp \
    dessp.cpp \
    dh.cpp \
    dh2.cpp \
    dll.cpp \
    dlltest.cpp \
    dsa.cpp \
    eax.cpp \
    ec2n.cpp \
    eccrypto.cpp \
    ecp.cpp \
    elgamal.cpp \
    emsa2.cpp \
    eprecomp.cpp \
    esign.cpp \
    files.cpp \
    filters.cpp \
    fips140.cpp \
    fipsalgt.cpp \
    fipstest.cpp \
    gcm.cpp \
    gf2_32.cpp \
    gf2n.cpp \
    gf256.cpp \
    gfpcrypt.cpp \
    gost.cpp \
    gzip.cpp \
    hex.cpp \
    hmac.cpp \
    hrtimer.cpp \
    ida.cpp \
    idea.cpp \
    integer.cpp \
    iterhash.cpp \
    luc.cpp \
    mars.cpp \
    marss.cpp \
    md2.cpp \
    md4.cpp \
    md5.cpp \
    misc.cpp \
    modes.cpp \
    mqueue.cpp \
    mqv.cpp \
    nbtheory.cpp \
    network.cpp \
    oaep.cpp \
    osrng.cpp \
    panama.cpp \
    pch.cpp \
    pkcspad.cpp \
    polynomi.cpp \
    pssr.cpp \
    pubkey.cpp \
    queue.cpp \
    rabin.cpp \
    randpool.cpp \
    rc2.cpp \
    rc5.cpp \
    rc6.cpp \
    rdtables.cpp \
    regtest.cpp \
    rijndael.cpp \
    ripemd.cpp \
    rng.cpp \
    rsa.cpp \
    rw.cpp \
    safer.cpp \
    salsa.cpp \
    seal.cpp \
    seed.cpp \
    serpent.cpp \
    sha.cpp \
    sha3.cpp \
    shacal2.cpp \
    shark.cpp \
    sharkbox.cpp \
    simple.cpp \
    skipjack.cpp \
    socketft.cpp \
    sosemanuk.cpp \
    square.cpp \
    squaretb.cpp \
    strciphr.cpp \
    tea.cpp \
    test.cpp \
    tftables.cpp \
    tiger.cpp \
    tigertab.cpp \
    trdlocal.cpp \
    ttmac.cpp \
    twofish.cpp \
    validat0.cpp \
    validat1.cpp \
    validat2.cpp \
    validat3.cpp \
    vmac.cpp \
    wait.cpp \
    wake.cpp \
    whrlpool.cpp \
    winpipes.cpp \
    xtr.cpp \
    xtrcrypt.cpp \
    zdeflate.cpp \
    zinflate.cpp \
    zlib.cpp

HEADERS += \
    3way.h \
    adler32.h \
    aes.h \
    algebra.h \
    algparam.h \
    arc4.h \
    argnames.h \
    asn.h \
    authenc.h \
    base32.h \
    base64.h \
    basecode.h \
    bench.h \
    blowfish.h \
    blumshub.h \
    camellia.h \
    cast.h \
    cbcmac.h \
    ccm.h \
    channels.h \
    cmac.h \
    config.h \
    cpu.h \
    crc.h \
    cryptlib.h \
    default.h \
    des.h \
    dh.h \
    dh2.h \
    dll.h \
    dmac.h \
    dsa.h \
    eax.h \
    ec2n.h \
    eccrypto.h \
    ecp.h \
    elgamal.h \
    emsa2.h \
    eprecomp.h \
    esign.h \
    factory.h \
    files.h \
    filters.h \
    fips140.h \
    fltrimpl.h \
    gcm.h \
    gf2_32.h \
    gf2n.h \
    gf256.h \
    gfpcrypt.h \
    gost.h \
    gzip.h \
    hex.h \
    hkdf.h \
    hmac.h \
    hrtimer.h \
    ida.h \
    idea.h \
    integer.h \
    iterhash.h \
    lubyrack.h \
    luc.h \
    mars.h \
    md2.h \
    md4.h \
    md5.h \
    mdc.h \
    misc.h \
    modarith.h \
    modes.h \
    modexppc.h \
    mqueue.h \
    mqv.h \
    nbtheory.h \
    network.h \
    nr.h \
    oaep.h \
    oids.h \
    osrng.h \
    panama.h \
    pch.h \
    pkcspad.h \
    polynomi.h \
    pssr.h \
    pubkey.h \
    pwdbased.h \
    queue.h \
    rabin.h \
    randpool.h \
    rc2.h \
    rc5.h \
    rc6.h \
    resource.h \
    rijndael.h \
    ripemd.h \
    rng.h \
    rsa.h \
    rw.h \
    safer.h \
    salsa.h \
    seal.h \
    secblock.h \
    seckey.h \
    seed.h \
    serpent.h \
    serpentp.h \
    sha.h \
    sha3.h \
    shacal2.h \
    shark.h \
    simple.h \
    skipjack.h \
    smartptr.h \
    socketft.h \
    sosemanuk.h \
    square.h \
    stdcpp.h \
    strciphr.h \
    tea.h \
    tiger.h \
    trap.h \
    trdlocal.h \
    trunhash.h \
    ttmac.h \
    twofish.h \
    validate.h \
    vmac.h \
    wait.h \
    wake.h \
    whrlpool.h \
    winpipes.h \
    words.h \
    xtr.h \
    xtrcrypt.h \
    zdeflate.h \
    zinflate.h \
    zlib.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
