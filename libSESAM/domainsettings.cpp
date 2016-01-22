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

#include <QDebug>
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
const QString DomainSettings::CDATE = "cDate";
const QString DomainSettings::MDATE = "mDate";
const QString DomainSettings::DELETED = "deleted";
const QString DomainSettings::PASSWORD_LENGTH = "length";
const QString DomainSettings::USED_CHARACTERS = "usedCharacters";
// v3 settings
const QString DomainSettings::EXTRA_CHARACTERS = "extras";
const QString DomainSettings::PASSWORD_TEMPLATE = "passwordTemplate";
const QString DomainSettings::GROUP = "group";
const QString DomainSettings::EXPIRY_DATE = "expiryDate";
const QString DomainSettings::TAGS = "tags";


DomainSettings::DomainSettings(void)
  : salt_base64(DefaultSalt_base64)
  , iterations(DefaultIterations)
  , passwordLength(DefaultPasswordLength)
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
  , passwordLength(o.passwordLength)
  , usedCharacters(o.usedCharacters)
  , createdDate(o.createdDate)
  , modifiedDate(o.modifiedDate)
  , deleted(o.deleted)
  // v3 settings
  , extraCharacters(o.extraCharacters)
  , passwordTemplate(o.passwordTemplate)
  , groupHierarchy(o.groupHierarchy)
  , expiryDate(o.expiryDate)
  , tags(o.tags)
{ /* ... */ }


bool DomainSettings::expired(void) const
{
  return !expiryDate.isNull() && expiryDate < QDateTime::currentDateTime();
}


bool DomainSettings::isEmpty(void) const
{
  return domainName.isEmpty();
}


void DomainSettings::clear(void)
{
  *this = DomainSettings();
}


QVariantMap DomainSettings::toVariantMap(void) const
{
  QVariantMap map;
  map[DOMAIN_NAME] = domainName;
  if (deleted) {
    map[DELETED] = true;
  }
  map[CDATE] = createdDate;
  if (modifiedDate.isValid()) {
    map[MDATE] = modifiedDate;
  }
  if (!deleted) {
    if (!userName.isEmpty()) {
        map[USER_NAME] = userName;
    }
    if (!url.isEmpty()) {
      map[URL] = url;
    }
    if (!notes.isEmpty()) {
      map[NOTES] = notes;
    }
    if (!groupHierarchy.isEmpty()) {
      map[GROUP] = groupHierarchy;
    }
    if (!expiryDate.isNull()) {
      map[EXPIRY_DATE] = expiryDate;
    }
    if (!tags.isEmpty()) {
      map[TAGS] = tags.join(QChar('\t'));
    }
    if (legacyPassword.isEmpty()) {
      map[SALT] = salt_base64;
      map[ITERATIONS] = iterations;
      map[PASSWORD_LENGTH] = passwordLength;
      map[USED_CHARACTERS] = usedCharacters;
      // v3 settings
      if (!extraCharacters.isEmpty()) {
        map[EXTRA_CHARACTERS] = extraCharacters;
      }
      if (!passwordTemplate.isEmpty()) {
        map[PASSWORD_TEMPLATE] = passwordTemplate;
      }
    }
    else {
      map[LEGACY_PASSWORD] = legacyPassword;
    }
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
  ds.salt_base64 = map[SALT].toByteArray();
  ds.iterations = map[ITERATIONS].toInt();
  ds.passwordLength = map[PASSWORD_LENGTH].toInt();
  ds.usedCharacters = map[USED_CHARACTERS].toString();
  ds.createdDate = QDateTime::fromString(map[CDATE].toString(), Qt::DateFormat::ISODate);
  ds.modifiedDate = QDateTime::fromString(map[MDATE].toString(), Qt::DateFormat::ISODate);
  ds.deleted = map[DELETED].toBool();
  // v3 settings
  ds.extraCharacters = map[EXTRA_CHARACTERS].toString();
  ds.passwordTemplate = map[PASSWORD_TEMPLATE].toByteArray();
  ds.groupHierarchy = map[GROUP].toString();
  ds.expiryDate = map[EXPIRY_DATE].toDateTime();
  ds.tags = map[TAGS].toString().split(QChar('\t'));
  return ds;
}


QDebug operator<<(QDebug debug, const DomainSettings &ds)
{
  QDebugStateSaver saver(debug);
  (void)saver;
  debug.nospace()
      << "DomainSettings {\n"
      << "  " << DomainSettings::DOMAIN_NAME << ": " << ds.domainName << ",\n";
  if (ds.createdDate.isValid()) {
    debug.nospace() << "  " << DomainSettings::CDATE << ": " << ds.createdDate << ",\n";
  }
  if (ds.modifiedDate.isValid()) {
    debug.nospace() << "  " << DomainSettings::MDATE << ": " << ds.modifiedDate << ",\n";
  }
  if (!ds.deleted) {
    if (!ds.userName.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::USER_NAME << ": " << ds.userName << ",\n";
    }
    if (!ds.url.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::URL << ": " << ds.url << ",\n";
    }
    if (!ds.notes.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::NOTES << ": " << ds.notes << ",\n";
    }
    if (!ds.groupHierarchy.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::GROUP << ": " << ds.groupHierarchy << ",\n";
    }
    if (!ds.expiryDate.isNull()) {
      debug.nospace() << "  " << DomainSettings::EXPIRY_DATE << ": " << ds.expiryDate << ",\n";
    }
    if (!ds.tags.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::TAGS << ": " << ds.tags.join(';') << ",\n";
    }
    if (!ds.legacyPassword.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::LEGACY_PASSWORD << ": " << ds.legacyPassword << ",\n";
    }
    else {
      debug.nospace() << "  " << DomainSettings::SALT << ": " << ds.salt_base64 << ",\n";
      debug.nospace() << "  " << DomainSettings::ITERATIONS << ": " << ds.iterations << ",\n";
      debug.nospace() << "  " << DomainSettings::PASSWORD_LENGTH << ": " << ds.passwordLength << ",\n";
      debug.nospace() << "  " << DomainSettings::USED_CHARACTERS << ": " <<  ds.usedCharacters << ",\n";
      // v3 settings
      if (!ds.extraCharacters.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::EXTRA_CHARACTERS << ": " << ds.extraCharacters << ",\n";
      }
      if (!ds.passwordTemplate.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::PASSWORD_TEMPLATE << ": " << ds.passwordTemplate << ",\n";
      }
    }
  }
  else {
    debug.nospace() << "  " << DomainSettings::DELETED << ": true,\n";
  }
  debug.nospace() << "}";
  return debug;
}
