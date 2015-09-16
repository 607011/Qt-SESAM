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

KeyboardHook *KeyboardHook::singleInstance = nullptr;

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
  if (singleInstance == nullptr) {
    singleInstance = new KeyboardHook;
    singleInstance->hook();
  }
  return singleInstance;
}


bool KeyboardHook::hook(void)
{
  qDebug() << "KeyboardHook::hook()";
  HINSTANCE hApp = GetModuleHandle(NULL);
  keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hApp, 0);
  if (keyboardHook == NULL)
    qWarning() << "SetWindowsHookEx() failed.";
  return keyboardHook != NULL;
}


LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  KBDLLHOOKSTRUCT *pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
  switch (wParam) {
  case WM_KEYUP:
  {
    qDebug() << "WM_KEYUP" << nCode << wParam << pKeyBoard->vkCode << pKeyBoard->flags << pKeyBoard->scanCode;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && pKeyBoard->vkCode == 86) {
      emit KeyboardHook::instance()->pasted();
    }
    break;
  }
  default:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }
  return 0;
}
