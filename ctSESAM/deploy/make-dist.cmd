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

ECHO Removing old files ...
RD /S /Q %DESTDIR%
DEL %DESTDIR%.zip
DEL %DESTDIR%.7z
DEL %DESTDIR%.zip.txt
DEL %DESTDIR%.7z.txt

ECHO Making directories in %DESTDIR% ...
IF NOT EXIST %DESTDIR% MKDIR %DESTDIR%
IF NOT EXIST %DESTDIR%\platforms MKDIR %DESTDIR%\platforms
IF NOT EXIST %DESTDIR%\resources\images MKDIR %DESTDIR%\resources\images

ECHO Copying files to %DESTDIR% ...
COPY /B ..\..\LICENSE %DESTDIR% >NUL
COPY /B ..\..\LIESMICH.txt %DESTDIR% >NUL
COPY /B ssleay32.dll %DESTDIR% >NUL
COPY /B libeay32.dll %DESTDIR% >NUL
COPY /B msvcp120.dll %DESTDIR% >NUL
COPY /B msvcr120.dll %DESTDIR% >NUL
COPY /B %QTDIR%\Qt5Core.dll %DESTDIR% >NUL
COPY /B %QTDIR%\Qt5Gui.dll %DESTDIR% >NUL
COPY /B %QTDIR%\Qt5Widgets.dll %DESTDIR% >NUL
COPY /B %QTDIR%\Qt5Network.dll %DESTDIR% >NUL
COPY /B %QTDIR%\Qt5Concurrent.dll %DESTDIR% >NUL
COPY /B %QTDIR%\icudt54.dll %DESTDIR% >NUL
COPY /B %QTDIR%\icuin54.dll %DESTDIR% >NUL
COPY /B %QTDIR%\icuuc54.dll %DESTDIR% >NUL
COPY /B %QTDIR%\..\plugins\platforms\qminimal.dll %DESTDIR%\platforms >NUL
COPY /B %QTDIR%\..\plugins\platforms\qwindows.dll %DESTDIR%\platforms >NUL
COPY /B %BUILDDIR%\QtSESAM.exe %DESTDIR% >NUL
COPY /B ..\resources\images\* %DESTDIR%\resources\images >NUL
COPY /Y NUL %DESTDIR%\PORTABLE >NUL

ECHO Build compressed archives ...
7z a -t7z -mmt=on %DESTDIR%.7z %DESTDIR%
7z a -tZip -mmt=on %DESTDIR%.zip %DESTDIR%
