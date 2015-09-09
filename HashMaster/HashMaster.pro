TEMPLATE = app
CONFIG += console c++11 warn_off
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

DEFINES += CRYPTOPP_DISABLE_ASM

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
