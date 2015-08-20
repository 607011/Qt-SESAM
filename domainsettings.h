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
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

class DomainSettings {
public:
  DomainSettings(void);
  DomainSettings(const DomainSettings &);

  QVariantMap toVariantMap(void) const;
  bool isEmpty(void) const;

  static DomainSettings fromVariantMap(const QVariantMap &);

  static const QByteArray DefaultSalt;
  static const QByteArray DefaultSalt_base64;
  static const int DefaultIterations;
  static const int DefaultPasswordLength;

  static const QString DOMAIN_NAME;
  QString domainName;

  static const QString USER_NAME;
  QString userName;

  static const QString LEGACY_PASSWORD;
  QString legacyPassword;

  static const QString NOTES;
  QString notes;

  static const QString SALT;
  QString salt_base64;

  static const QString ITERATIONS;
  int iterations;

  static const QString LENGTH;
  int length;

  static const QString USED_CHARACTERS;
  QString usedCharacters;

  static const QString CDATE;
  QDateTime createdDate;

  static const QString MDATE;
  QDateTime modifiedDate;

  static const QString CAN_BE_DELETED_BY_REMOTE;
  bool canBeDeletedByRemote;

  static const QString DELETED;
  bool deleted;

  bool forceLowerCase;
  bool forceUpperCase;
  bool forceDigits;
  bool forceExtra;
};



#endif // __DOMAINSETTINGS_H_
