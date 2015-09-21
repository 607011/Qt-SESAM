QT       -= gui

TARGET = qtkeychain
TEMPLATE = lib

DEFINES += QKEYCHAIN_BUILD_QKEYCHAIN_LIB

SOURCES += keychain.cpp

macosx {
  SOURCES += keychain_mac.cpp
}

linux {
  QT += xml dbus
  SOURCES += gnomekeyring.cpp \
    keychain_unix.cpp
  HEADERS += gnomekeyring_p.h
}

win32 {
  SOURCES += keychain_win.cpp
  LIBS += Crypt32.lib
}

HEADERS += \
    keychain.h \
    keychain_p.h \
    qkeychain_export.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    org.kde.KWallet.xml
