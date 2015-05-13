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

#include <QString>
#include <QRegExp>

struct DomainSettings {
  DomainSettings(void)
    : useLowerCase(true)
    , useUpperCase(true)
    , useDigits(true)
    , useExtra(true)
    , useCustom(false)
    , iterations(DefaultIterations)
    , length(DefaultPasswordLength)
    , salt(DefaultSalt)
    , forceValidation(false)
    , validatorRegEx(DefaultValidatorPattern)
  { /* ... */ }
  DomainSettings(const DomainSettings &o)
    : useLowerCase(o.useLowerCase)
    , useUpperCase(o.useUpperCase)
    , useDigits(o.useDigits)
    , useExtra(o.useExtra)
    , useCustom(o.useCustom)
    , customCharacters(o.customCharacters)
    , iterations(o.iterations)
    , length(o.length)
    , salt(o.salt)
    , forceValidation(o.forceValidation)
    , validatorRegEx(o.validatorRegEx)
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
  bool forceValidation;
  QRegExp validatorRegEx;

  static const int DefaultIterations;
  static const int DefaultPasswordLength;
  static const QString DefaultSalt;
  static const QString DefaultValidatorPattern;
};




#endif // __DOMAINSETTINGS_H_
