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


#ifndef __DOMAINSETTINGS_H_
#define __DOMAINSETTINGS_H_

#include <QString>
#include <QRegExp>
#include <QVariantMap>
#include <QDateTime>
#include <QList>
#include <QStringList>

class DomainSettings {
public:
  DomainSettings(void);
  DomainSettings(const DomainSettings &o);

  QVariantMap toVariantMap(void) const;
  bool isEmpty(void) const;

  static DomainSettings fromVariantMap(const QVariantMap &);

  static const int DefaultIterations;
  static const int DefaultPasswordLength;
  static const bool DefaultUseLowerCase;
  static const bool DefaultUseUpperCase;
  static const bool DefaultUseDigits;
  static const bool DefaultUseExtra;
  static const bool DefaultUseCustom;
  static const bool DefaultAvoidAmbiguous;
  static const QString DefaultSalt;
  static const bool DefaultForceValidation;
  static const QString DefaultValidatorPattern;

  static const QString DOMAIN_NAME;
  static const QString USER_NAME;
  static const QString NOTES;
  static const QString LEGACY_PASSWORD;
  static const QString USE_LOWERCASE;
  static const QString USE_UPPERCASE;
  static const QString USE_DIGITS;
  static const QString USE_EXTRA;
  static const QString USE_CUSTOM;
  static const QString AVOID_AMBIGUOUS;
  static const QString SALT;
  static const QString CUSTOM_CHARACTER_SET;
  static const QString ITERATIONS;
  static const QString LENGTH;
  static const QString FORCE_LOWERCASE;
  static const QString FORCE_UPPERCASE;
  static const QString FORCE_DIGITS;
  static const QString FORCE_EXTRA;
  static const QString FORCE_REGEX_VALIDATION;
  static const QString VALIDATOR_REGEX;
  static const QString CDATE;
  static const QString MDATE;
  static const QString DELETED;

  QString domainName;
  QString userName;
  QString legacyPassword;
  QString notes;
  bool useLowerCase;
  bool useUpperCase;
  bool useDigits;
  bool useExtra;
  bool useCustom;
  bool avoidAmbiguous;
  QString customCharacterSet;
  int iterations;
  int length;
  QString salt;
  bool forceLowerCase;
  bool forceUpperCase;
  bool forceDigits;
  bool forceExtra;
  bool forceRegexValidation;
  QRegExp validatorRegEx;
  QDateTime cDate;
  QDateTime mDate;
  bool deleted;
};



#endif // __DOMAINSETTINGS_H_
