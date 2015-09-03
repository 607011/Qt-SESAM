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

#include "domainsettings.h"

#include <QtDebug>

#include <QByteArray>
#include <QJsonDocument>

const QByteArray DomainSettings::DefaultSalt = QString("pepper").toUtf8();
const QByteArray DomainSettings::DefaultSalt_base64 = DomainSettings::DefaultSalt.toBase64();
const int DomainSettings::DefaultIterations = 4096;
const int DomainSettings::DefaultPasswordLength = 10;
const int DomainSettings::DefaultSaltLength = 16;


const QString DomainSettings::DOMAIN_NAME = "domain";
const QString DomainSettings::USER_NAME = "username";
const QString DomainSettings::LEGACY_PASSWORD = "legacyPassword";
const QString DomainSettings::NOTES = "notes";
const QString DomainSettings::ITERATIONS = "iterations";
const QString DomainSettings::SALT = "salt";
const QString DomainSettings::LENGTH = "length";
const QString DomainSettings::CDATE = "cDate";
const QString DomainSettings::MDATE = "mDate";
const QString DomainSettings::USED_CHARACTERS = "usedCharacters";
const QString DomainSettings::DELETED = "deleted";


DomainSettings::DomainSettings(void)
  : iterations(DefaultIterations)
  , length(DefaultPasswordLength)
  , salt_base64(DefaultSalt_base64)
  , forceLowerCase(false)
  , forceUpperCase(false)
  , forceDigits(false)
  , forceExtra(false)
  , deleted(false)
{ /* ... */ }


DomainSettings::DomainSettings(const DomainSettings &o)
  : domainName(o.domainName)
  , userName(o.userName)
  , legacyPassword(o.legacyPassword)
  , notes(o.notes)
  , iterations(o.iterations)
  , length(o.length)
  , salt_base64(o.salt_base64)
  , usedCharacters(o.usedCharacters)
  , forceLowerCase(o.forceLowerCase)
  , forceUpperCase(o.forceUpperCase)
  , forceDigits(o.forceDigits)
  , forceExtra(o.forceExtra)
  , createdDate(o.createdDate)
  , modifiedDate(o.modifiedDate)
  , deleted(o.deleted)
{ /* ... */ }


QVariantMap DomainSettings::toVariantMap(void) const
{
  QVariantMap map;
  map[DOMAIN_NAME] = domainName;
  map[USER_NAME] = userName;
  map[LEGACY_PASSWORD] = legacyPassword;
  map[NOTES] = notes;
  map[ITERATIONS] = iterations;
  map[LENGTH] = length;
  map[SALT] = salt_base64;
  map[USED_CHARACTERS] = usedCharacters;
  map[CDATE] = createdDate;
  map[MDATE] = modifiedDate;
  map[DELETED] = deleted;
  return map;
}


DomainSettings DomainSettings::fromVariantMap(const QVariantMap &map)
{
  DomainSettings ds;
  ds.domainName = map[DOMAIN_NAME].toString();
  ds.userName = map[USER_NAME].toString();
  ds.legacyPassword = map[LEGACY_PASSWORD].toString();
  ds.notes = map[NOTES].toString();
  ds.iterations = map[ITERATIONS].toInt();
  ds.length = map[LENGTH].toInt();
  ds.salt_base64 = map[SALT].toByteArray();
  ds.usedCharacters = map[USED_CHARACTERS].toString();
  ds.createdDate = QDateTime::fromString(map[CDATE].toString(), Qt::DateFormat::ISODate);
  ds.modifiedDate = QDateTime::fromString(map[MDATE].toString(), Qt::DateFormat::ISODate);
  return ds;
}
