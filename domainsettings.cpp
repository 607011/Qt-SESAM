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


const QString DomainSettings::DOMAINNAME = "domain";
const QString DomainSettings::USERNAME = "username";
const QString DomainSettings::USELOWERCASE = "useLowerCase";
const QString DomainSettings::USEUPPERCASE = "useUpperCase";
const QString DomainSettings::USEDIGITS = "useDigits";
const QString DomainSettings::USEEXTRA = "useExtra";
const QString DomainSettings::USECUSTOM = "useCustom";
const QString DomainSettings::AVOIDAMBIGUOUS = "avoidAmbiguous";
const QString DomainSettings::CUSTOMCHARACTERSET = "customCharacterSet";
const QString DomainSettings::ITERATIONS = "iterations";
const QString DomainSettings::SALT = "salt";
const QString DomainSettings::LENGTH = "length";
const QString DomainSettings::FORCELOWERCASE = "forceLowerCase";
const QString DomainSettings::FORCEUPPERCASE = "forceUpperCase";
const QString DomainSettings::FORCEDIGITS = "forceDigits";
const QString DomainSettings::FORCEEXTRA = "forceExtra";
const QString DomainSettings::FORCEREGEXVALIDATION = "forceRegexValidation";
const QString DomainSettings::VALIDATORREGEX = "validatorRegex";
const QString DomainSettings::CDATE = "cDate";
const QString DomainSettings::MDATE = "mDate";
const QString DomainSettings::DELETED = "deleted";
const QString DomainSettings::LEGACY_PASSWORD = "legacyPassword";


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
  , forceLowerCase(false)
  , forceUpperCase(false)
  , forceDigits(false)
  , forceExtra(false)
  , forceRegexValidation(DefaultForceValidation)
  , validatorRegEx(DefaultValidatorPattern)
  , deleted(false)
{ /* ... */ }


DomainSettings::DomainSettings(const DomainSettings &o)
  : domainName(o.domainName)
  , username(o.username)
  , useLowerCase(o.useLowerCase)
  , useUpperCase(o.useUpperCase)
  , useDigits(o.useDigits)
  , useExtra(o.useExtra)
  , useCustom(o.useCustom)
  , avoidAmbiguous(o.avoidAmbiguous)
  , customCharacterSet(o.customCharacterSet)
  , iterations(o.iterations)
  , length(o.length)
  , salt(o.salt)
  , forceLowerCase(o.forceLowerCase)
  , forceUpperCase(o.forceUpperCase)
  , forceDigits(o.forceDigits)
  , forceExtra(o.forceExtra)
  , forceRegexValidation(o.forceRegexValidation)
  , validatorRegEx(o.validatorRegEx)
  , cDate(o.cDate)
  , mDate(o.mDate)
  , deleted(o.deleted)
  , legacyPassword(o.legacyPassword)
{ /* ... */ }


QVariantMap DomainSettings::toVariantMap(void) const
{
  QVariantMap map;
  map[DOMAINNAME] = domainName;
  map[USERNAME] = username;
  map[USELOWERCASE] = useLowerCase;
  map[USEUPPERCASE] = useUpperCase;
  map[USEDIGITS] = useDigits;
  map[USEEXTRA] = useExtra;
  map[USECUSTOM] = useCustom;
  map[AVOIDAMBIGUOUS] = avoidAmbiguous;
  map[CUSTOMCHARACTERSET] = customCharacterSet;
  map[ITERATIONS] = iterations;
  map[LENGTH] = length;
  map[SALT] = salt;
  map[FORCELOWERCASE] = forceLowerCase;
  map[FORCEUPPERCASE] = forceUpperCase;
  map[FORCEDIGITS] = forceDigits;
  map[FORCEEXTRA] = forceExtra;
  map[FORCEREGEXVALIDATION] = forceRegexValidation;
  map[VALIDATORREGEX] = validatorRegEx.pattern();
  map[CDATE] = cDate;
  map[MDATE] = mDate;
  map[DELETED] = deleted;
  map[LEGACY_PASSWORD] = legacyPassword;
  return map;
}
