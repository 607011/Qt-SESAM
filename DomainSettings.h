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

class DomainSettings {
public:
  DomainSettings(void);
  DomainSettings(const DomainSettings &o);

  QVariantMap toVariant(void) const;

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

  QString domain;
  QString username;
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
