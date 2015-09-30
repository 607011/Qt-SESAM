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
const int DomainSettings::DefaultIterations = 8192;
const int DomainSettings::DefaultPasswordLength = 16;
const int DomainSettings::DefaultSaltLength = 16;


const QString DomainSettings::DOMAIN_NAME = "domain";
const QString DomainSettings::URL = "url";
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
const QString DomainSettings::EXTRAS = "extras";


DomainSettings::DomainSettings(void)
  : salt_base64(DefaultSalt_base64)
  , iterations(DefaultIterations)
  , length(DefaultPasswordLength)
  , deleted(false)
{ /* ... */ }


DomainSettings::DomainSettings(const DomainSettings &o)
  : domainName(o.domainName)
  , userName(o.userName)
  , url(o.url)
  , legacyPassword(o.legacyPassword)
  , notes(o.notes)
  , salt_base64(o.salt_base64)
  , iterations(o.iterations)
  , length(o.length)
  , usedCharacters(o.usedCharacters)
  , createdDate(o.createdDate)
  , modifiedDate(o.modifiedDate)
  , deleted(o.deleted)
  , extras(o.extras)
{ /* ... */ }


QVariantMap DomainSettings::toVariantMap(void) const
{
  QVariantMap map;
  map[DOMAIN_NAME] = domainName;
  if (deleted)
    map[DELETED] = true;
  map[CDATE] = createdDate;
  map[MDATE] = modifiedDate;
  if (!deleted) {
    if (!userName.isEmpty())
        map[USER_NAME] = userName;
    if (!url.isEmpty())
      map[URL] = url;
    if (!legacyPassword.isEmpty())
      map[LEGACY_PASSWORD] = legacyPassword;
    if (!notes.isEmpty())
      map[NOTES] = notes;
    map[ITERATIONS] = iterations;
    map[LENGTH] = length;
    map[SALT] = salt_base64;
    map[USED_CHARACTERS] = usedCharacters;
    if (!extras.isEmpty())
      map[EXTRAS] = extras;
  }
  return map;
}


DomainSettings DomainSettings::fromVariantMap(const QVariantMap &map)
{
  DomainSettings ds;
  ds.domainName = map[DOMAIN_NAME].toString();
  ds.userName = map[USER_NAME].toString();
  ds.url = map[URL].toString();
  ds.legacyPassword = map[LEGACY_PASSWORD].toString();
  ds.notes = map[NOTES].toString();
  ds.iterations = map[ITERATIONS].toInt();
  ds.length = map[LENGTH].toInt();
  ds.salt_base64 = map[SALT].toByteArray();
  ds.usedCharacters = map[USED_CHARACTERS].toString();
  ds.createdDate = QDateTime::fromString(map[CDATE].toString(), Qt::DateFormat::ISODate);
  ds.modifiedDate = QDateTime::fromString(map[MDATE].toString(), Qt::DateFormat::ISODate);
  ds.deleted = map[DELETED].toBool();
  ds.extras = map[URL].toString();
  return ds;
}
