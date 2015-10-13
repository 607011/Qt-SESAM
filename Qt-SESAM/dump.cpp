/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "dump.h"

void make_minidump(void)
{
  auto hDbgHelp = LoadLibraryA("dbghelp");
  if (hDbgHelp == NULL)
    return;
  auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
  if (pMiniDumpWriteDump == NULL)
    return;

  char name[MAX_PATH];
  {
    auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
    SYSTEMTIME t;
    GetSystemTime(&t);
    wsprintfA(nameEnd - strlen(".exe"),
              "_%4d%02d%02d_%02d%02d%02d.dmp",
              t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
  }

  HANDLE hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
    return;

  pMiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithFullMemory | MiniDumpWithPrivateWriteCopyMemory),
        NULL,
        NULL,
        NULL);

  CloseHandle(hFile);
  return;
}
