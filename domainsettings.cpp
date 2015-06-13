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


const QString DomainSettings::DOMAIN_NAME = "domain";
const QString DomainSettings::USER_NAME = "username";
const QString DomainSettings::LEGACY_PASSWORD = "legacyPassword";
const QString DomainSettings::NOTES = "notes";
const QString DomainSettings::USE_LOWERCASE = "useLowerCase";
const QString DomainSettings::USE_UPPERCASE = "useUpperCase";
const QString DomainSettings::USE_DIGITS = "useDigits";
const QString DomainSettings::USE_EXTRA = "useExtra";
const QString DomainSettings::USE_CUSTOM = "useCustom";
const QString DomainSettings::AVOID_AMBIGUOUS = "avoidAmbiguous";
const QString DomainSettings::CUSTOM_CHARACTER_SET = "customCharacterSet";
const QString DomainSettings::ITERATIONS = "iterations";
const QString DomainSettings::SALT = "salt";
const QString DomainSettings::LENGTH = "length";
const QString DomainSettings::FORCE_LOWERCASE = "forceLowerCase";
const QString DomainSettings::FORCE_UPPERCASE = "forceUpperCase";
const QString DomainSettings::FORCE_DIGITS = "forceDigits";
const QString DomainSettings::FORCE_EXTRA = "forceExtra";
const QString DomainSettings::FORCE_REGEX_VALIDATION = "forceRegexValidation";
const QString DomainSettings::VALIDATOR_REGEX = "validatorRegex";
const QString DomainSettings::CDATE = "cDate";
const QString DomainSettings::MDATE = "mDate";
const QString DomainSettings::DELETED = "deleted";


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
  , legacyPassword(o.legacyPassword)
  , notes(o.notes)
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
{ /* ... */ }


QVariantMap DomainSettings::toVariantMap(void) const
{
  QVariantMap map;
  map[DOMAIN_NAME] = domainName;
  map[USER_NAME] = username;
  map[LEGACY_PASSWORD] = legacyPassword;
  map[NOTES] = notes;
  map[USE_LOWERCASE] = useLowerCase;
  map[USE_UPPERCASE] = useUpperCase;
  map[USE_DIGITS] = useDigits;
  map[USE_EXTRA] = useExtra;
  map[USE_CUSTOM] = useCustom;
  map[AVOID_AMBIGUOUS] = avoidAmbiguous;
  map[CUSTOM_CHARACTER_SET] = customCharacterSet;
  map[ITERATIONS] = iterations;
  map[LENGTH] = length;
  map[SALT] = salt;
  map[FORCE_LOWERCASE] = forceLowerCase;
  map[FORCE_UPPERCASE] = forceUpperCase;
  map[FORCE_DIGITS] = forceDigits;
  map[FORCE_EXTRA] = forceExtra;
  map[FORCE_REGEX_VALIDATION] = forceRegexValidation;
  map[VALIDATOR_REGEX] = validatorRegEx.pattern();
  map[CDATE] = cDate;
  map[MDATE] = mDate;
  map[DELETED] = deleted;
  return map;
}
