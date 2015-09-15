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


#include <QDebug>
#include "keyboardhook.h"

KeyboardHook::KeyboardHook(void)
  : QObject()
  , keyboardHook(NULL)
{
  // do nothing ...
}


KeyboardHook::~KeyboardHook()
{
  if (keyboardHook != NULL)
    UnhookWindowsHookEx(keyboardHook);
}


KeyboardHook *KeyboardHook::instance(void)
{
  static KeyboardHook *keyboardHookInstance = new KeyboardHook;
  keyboardHookInstance->hook();
  return keyboardHookInstance;
}


bool KeyboardHook::hook(void)
{
  HINSTANCE hApp = GetModuleHandle(NULL);
  keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hApp, 0);
  if (keyboardHook == NULL)
    qWarning() << "SetWindowsHookEx() failed.";
  return keyboardHook != NULL;
}


LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
  switch (wParam) {
  case WM_KEYUP:
  {
    const DWORD dwKeyCode = pKeyBoard->vkCode;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && dwKeyCode == 86) {
      emit KeyboardHook::instance()->pasted();
    }
    break;
  }
  default:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }
  return 0;
}
