!define VERSIONMAJOR "2"
!define VERSIONMINOR "0"
!define VERSIONPATCH ".4"
!define VERSION "${VERSIONMAJOR}.${VERSIONMINOR}${VERSIONPATCH}"
!define GUID "{f25f512a-7d58-4e2f-a52b-3663fd8ca813}"
!define APP "Qt-SESAM"
!define PUBLISHER "Heise Medien GmbH & Co. KG - Redaktion c't"
!define QTDIR "D:\Qt\5.5\msvc2013\bin"

Name "${APP} ${VERSION}"
OutFile "${APP}-${VERSION}-x86-setup.exe"
InstallDir $PROGRAMFILES\${APP}
InstallDirRegKey HKLM "Software\${PUBLISHER}\${APP}" "Install_Dir"
RequestExecutionLevel admin
SetCompressor lzma
ShowInstDetails show

# !include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

# !define MUI_FINISHPAGE_RUN "$INSTDIR\${APP}.exe"
# !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
# !define MUI_FINISHPAGE_RUN_TEXT "${APP} starten"

# Function LaunchLink
#   SetOutPath $INSTDIR
#   ExecShell "" '"$INSTDIR\${APP}.exe"'
# FunctionEnd


Section "vcredist"
  ClearErrors
  ReadRegDword $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\DevDiv\vc\Servicing\12.0\RuntimeMinimum" "Version"
  ${If} $R0 != "12.0.21005"
    SetOutPath "$INSTDIR"
    File "vcredist_msvc2013_x86.exe"
    ExecWait '"$INSTDIR\vcredist_msvc2013_x86.exe" /norestart /passive'
    Delete "$INSTDIR\vcredist_msvc2013_x86.exe"
  ${EndIf}
SectionEnd


Page license

  LicenseData "..\..\LICENSE"

Page directory

Page instfiles


Section "${APP}"
  SetOutPath "$INSTDIR"
  CreateDirectory "$INSTDIR\resources"
  CreateDirectory "$INSTDIR\resources\images"
  File "..\..\..\QtSESAM-Desktop_Qt_5_5_0_MSVC2013_32bit-Release\Qt-SESAM\release\${APP}.exe"
  File "..\..\LICENSE"
  File "libeay32.dll"
  File "ssleay32.dll"
  File "${QTDIR}\Qt5Core.dll"
  File "${QTDIR}\Qt5Gui.dll"
  File "${QTDIR}\Qt5Widgets.dll"
  File "${QTDIR}\Qt5Network.dll"
  File "${QTDIR}\Qt5Concurrent.dll"
  File "${QTDIR}\icudt54.dll"
  File "${QTDIR}\icuin54.dll"
  File "${QTDIR}\icuuc54.dll"

  SetOutPath "$INSTDIR\platforms"
  File "${QTDIR}\..\plugins\platforms\qminimal.dll"
  File "${QTDIR}\..\plugins\platforms\qwindows.dll"

  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "DisplayName" "${APP}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "DisplayIcon" "$INSTDIR\resources\images\ctSESAM.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "VersionMajor" "${VERSIONMAJOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "VersionMinor" "${VERSIONMINOR}"

  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}" "EstimatedSize" "$0"

  SetOutPath "$INSTDIR\resources\images"
  File /a /r "..\resources\images\"

  SetOutPath "$INSTDIR"

SectionEnd


Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${APP}"
  CreateShortCut "$SMPROGRAMS\${APP}\${APP} ${VERSION}.lnk" "$INSTDIR\${APP}.exe"
  CreateShortcut "$SMPROGRAMS\${APP}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd


Section "Desktop Icon"
  CreateShortCut "$DESKTOP\${APP}-${VERSION}.lnk" "$INSTDIR\${APP}.exe" ""
SectionEnd


# !insertmacro MUI_PAGE_FINISH

Section "Uninstall"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}"
  DeleteRegKey HKLM "SOFTWARE\${APP}"

  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\${APP}.exe"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\Qt5Core.dll"
  Delete "$INSTDIR\Qt5Gui.dll"
  Delete "$INSTDIR\Qt5Widgets.dll"
  Delete "$INSTDIR\Qt5Network.dll"
  Delete "$INSTDIR\Qt5Concurrent.dll"
  Delete "$INSTDIR\Qt5Test.dll"
  Delete "$INSTDIR\icudt54.dll"
  Delete "$INSTDIR\icuin54.dll"
  Delete "$INSTDIR\icuuc54.dll"
  Delete "$INSTDIR\libeay32.dll"
  Delete "$INSTDIR\ssleay32.dll"
  Delete "$INSTDIR\platforms\qminimal.dll"
  Delete "$INSTDIR\platforms\qwindows.dll"
  RMDir "$INSTDIR\platforms"

  Delete "$DESKTOP\${APP}-${VERSION}.lnk"
  Delete "$SMPROGRAMS\${APP}\*.*"
  RMDir "$SMPROGRAMS\${APP}"

  RMDir /r "$INSTDIR\resources"
  RMDir "$INSTDIR"
SectionEnd
