/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>

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


#ifndef __DOMAINSETTINGS_H_
#define __DOMAINSETTINGS_H_

struct DomainSettings {
  DomainSettings(void)
    : useLowerCase(true)
    , useUpperCase(true)
    , useDigits(true)
    , useExtra(true)
    , useCustom(false)
    , iterations(4096)
    , length(10)
    , salt("This is my salt. There are many like it, but this one is mine.")
    , validator("^(?=.*\d)(?=.*[a-z])(?=.*[A-Z])[a-zA-Z0-9]+$", Qt::CaseSensitive, QRegExp::RegExp2)
  { /* ... */ }
  DomainSettings(const DomainSettings &o)
    : useLowerCase(o.useLowerCase)
    , useUpperCase(o.useUpperCase)
    , useDigits(o.useDigits)
    , useExtra(o.useExtra)
    , useCustom(o.useCustom)
    , iterations(o.iterations)
    , length(o.length)
    , salt(o.salt)
    , validator(o.validator)
  { /* ... */ }
  bool useLowerCase;
  bool useUpperCase;
  bool useDigits;
  bool useExtra;
  bool useCustom;
  QString customCharacters;
  int iterations;
  int length;
  QString salt;
  QRegExp validator;
};




#endif // __DOMAINSETTINGS_H_
