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

#ifndef __UTIL_H_
#define __UTIL_H_

#include "cryptopp562/misc.h"

template <class T>
void SafeRenew(T& a, T obj)
{
  if (a != nullptr)
    delete a;
  a = obj;
}


template <class T>
void SafeDelete(T& a)
{
  SafeRenew<T>(a, nullptr);
}


template <class T>
void SecureErase(T &obj)
{
  CryptoPP::memset_z(obj.data(), 0, obj.size());
  obj.clear();
}


template <typename T>
void SecureErase(T *m, int size)
{
  CryptoPP::memset_z(m, 0, size);
}


#endif // __UTIL_H_
