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

#include "DomainSettings.h"

#ifdef QT_DEBUG
#include <QtDebug>
#endif

const int DomainSettings::DefaultIterations = 4096;
const int DomainSettings::DefaultPasswordLength = 10;
const QString DomainSettings::DefaultSalt = "pepper";
const bool DomainSettings::DefaultUseLowerCase = true;
const bool DomainSettings::DefaultUseUpperCase = true;
const bool DomainSettings::DefaultUseDigits = true;
const bool DomainSettings::DefaultUseExtra = false;
const bool DomainSettings::DefaultUseCustom = false;
const bool DomainSettings::DefaultAvoidAmbiguous = false;
const bool DomainSettings::DefaultForceValidation = false;
const QString DomainSettings::DefaultValidatorPattern = "^(?=.*\\d)(?=.*[a-z])(?=.*[A-Z])[a-zA-Z0-9]+$";


DomainSettings::DomainSettings(void)
  : useLowerCase(DefaultUseLowerCase)
  , useUpperCase(DefaultUseUpperCase)
  , useDigits(DefaultUseDigits)
  , useExtra(DefaultUseExtra)
  , useCustom(DefaultUseCustom)
  , avoidAmbiguous(DefaultAvoidAmbiguous)
  , iterations(DefaultIterations)
  , length(DefaultPasswordLength)
  , salt(DefaultSalt)
  , forceValidation(DefaultForceValidation)
  , validatorRegEx(DefaultValidatorPattern)
{ /* ... */ }


DomainSettings::DomainSettings(const DomainSettings &o)
  : domain(o.domain)
  , username(o.username)
  , useLowerCase(o.useLowerCase)
  , useUpperCase(o.useUpperCase)
  , useDigits(o.useDigits)
  , useExtra(o.useExtra)
  , useCustom(o.useCustom)
  , avoidAmbiguous(o.avoidAmbiguous)
  , customCharacters(o.customCharacters)
  , iterations(o.iterations)
  , length(o.length)
  , salt(o.salt)
  , forceValidation(o.forceValidation)
  , validatorRegEx(o.validatorRegEx)
{ /* ... */ }


QVariantMap DomainSettings::toVariant(void) const
{
  QVariantMap map;
  map["domain"] = domain;
  map["username"] = username;
  map["useLowerCase"] = useLowerCase;
  map["useUpperCase"] = useUpperCase;
  map["useDigits"] = useDigits;
  map["useExtra"] = useExtra;
  map["useCustom"] = useCustom;
  map["avoidAmbiguous"] = avoidAmbiguous;
  map["customCharacters"] = customCharacters;
  map["iterations"] = iterations;
  map["length"] = length;
  map["salt"] = salt;
  map["forceValidation"] = forceValidation;
  map["validatorRegEx"] = validatorRegEx.pattern();
  return map;
}
