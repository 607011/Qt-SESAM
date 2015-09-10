REM *************************************************************************
REM * Build portable application from QtSESAM binaries.                     *
REM *                                                                       *
REM * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG *
REM *************************************************************************

@ECHO OFF

SET QTDIR="D:\Qt\5.5\msvc2013\bin"
SET DESTDIR="portable"
SET BUILDIR="..\..\..\QtSESAM-Desktop_Qt_5_5_0_MSVC2013_32bit-Release\ctSESAM\release"

ECHO Making directories in %DESTDIR% ...
IF NOT EXIST %DESTDIR% (
  MKDIR %DESTDIR%
)
IF NOT EXIST %DESTDIR%\platforms (
  MKDIR %DESTDIR%\platforms
)
IF NOT EXIST %DESTDIR%\resources\images (
  MKDIR %DESTDIR%\resources\images
)

ECHO Copying files to %DESTDIR% ...
COPY /B /Y ..\..\LICENSE portable >NUL
COPY /B /Y ssleay32.dll portable >NUL
COPY /B /Y libeay32.dll portable >NUL
COPY /B /Y msvcp120.dll portable >NUL
COPY /B /Y msvcr120.dll portable >NUL
COPY /B /Y %QTDIR%\Qt5Core.dll portable >NUL
COPY /B /Y %QTDIR%\Qt5Gui.dll portable >NUL
COPY /B /Y %QTDIR%\Qt5Widgets.dll portable >NUL
COPY /B /Y %QTDIR%\Qt5Network.dll portable >NUL
COPY /B /Y %QTDIR%\Qt5Concurrent.dll portable >NUL
COPY /B /Y %QTDIR%\icudt54.dll portable >NUL
COPY /B /Y %QTDIR%\icuin54.dll portable >NUL
COPY /B /Y %QTDIR%\icuuc54.dll portable >NUL
COPY /B /Y %QTDIR%\..\plugins\platforms\qminimal.dll portable\platforms >NUL
COPY /B /Y %QTDIR%\..\plugins\platforms\qwindows.dll portable\platforms >NUL
COPY /B /Y %BUILDDIR%\ctSESAM.exe portable >NUL
