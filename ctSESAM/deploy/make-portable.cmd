REM *************************************************************************
REM * Build portable application from QtSESAM binaries.                     *
REM *                                                                       *
REM * Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG *
REM *************************************************************************

@ECHO OFF

SET QTDIR="D:\Qt\5.5\msvc2013\bin"
SET DESTDIR="QtSESAM-portable"
SET BUILDDIR="..\..\..\QtSESAM-Desktop_Qt_5_5_0_MSVC2013_32bit-Release\ctSESAM\release"
SET PATH=%PATH%;C:\Program Files\7-Zip

ECHO Making directories in %DESTDIR% ...
IF NOT EXIST %DESTDIR% MKDIR %DESTDIR%
IF NOT EXIST %DESTDIR%\platforms MKDIR %DESTDIR%\platforms
IF NOT EXIST %DESTDIR%\resources\images MKDIR %DESTDIR%\resources\images

ECHO Copying files to %DESTDIR% ...
COPY /B /Y ..\..\LICENSE %DESTDIR% >NUL
COPY /B /Y ..\..\LIESMICH.txt %DESTDIR% >NUL
COPY /B /Y ssleay32.dll %DESTDIR% >NUL
COPY /B /Y libeay32.dll %DESTDIR% >NUL
COPY /B /Y msvcp120.dll %DESTDIR% >NUL
COPY /B /Y msvcr120.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\Qt5Core.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\Qt5Gui.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\Qt5Widgets.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\Qt5Network.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\Qt5Concurrent.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\icudt54.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\icuin54.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\icuuc54.dll %DESTDIR% >NUL
COPY /B /Y %QTDIR%\..\plugins\platforms\qminimal.dll %DESTDIR%\platforms >NUL
COPY /B /Y %QTDIR%\..\plugins\platforms\qwindows.dll %DESTDIR%\platforms >NUL
COPY /B /Y %BUILDDIR%\ctSESAM.exe %DESTDIR% >NUL
COPY /B /Y ..\resources\images\* %DESTDIR%\resources\images >NUL

ECHO Compressing ...
7z a -t7z -mmt=on %DESTDIR%.7z %DESTDIR%
7z a -tZip -mmt=on %DESTDIR%.zip %DESTDIR%
