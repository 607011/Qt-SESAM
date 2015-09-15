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


#ifndef __KEYBOADHOOK_H_
#define __KEYBOADHOOK_H_

#include <QObject>
#include <Windows.h>

class KeyboardHook : public QObject {
  Q_OBJECT

signals:
  void pasted(void);

public:
  static KeyboardHook *instance(void);

private:
  HHOOK keyboardHook;
  KeyboardHook(void);
  ~KeyboardHook();
  bool hook(void);
  static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif
