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
const QChar DomainSettings::TagSeparator = QChar('\t');
const QChar DomainSettings::GroupSeparator = QChar('\t');

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
const QString DomainSettings::EXTRA_CHARACTERS = "extras";
#ifndef OMIT_V2_CODE
const QString DomainSettings::USED_CHARACTERS = "usedCharacters";
#endif
const QString DomainSettings::PASSWORD_TEMPLATE = "passwordTemplate";
const QString DomainSettings::GROUP = "group";
const QString DomainSettings::EXPIRY_DATE = "expiryDate";
const QString DomainSettings::TAGS = "tags";
const QString DomainSettings::FILES = "files";


DomainSettings::DomainSettings(void)
  : salt_base64(DefaultSalt_base64)
  , iterations(DefaultIterations)
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
  , createdDate(o.createdDate)
  , modifiedDate(o.modifiedDate)
  , deleted(o.deleted)
  , extraCharacters(o.extraCharacters)
#ifndef OMIT_V2_CODE
  , usedCharacters(o.usedCharacters)
#endif
  , passwordTemplate(o.passwordTemplate)
  , groupHierarchy(o.groupHierarchy)
  , expiryDate(o.expiryDate)
  , tags(o.tags)
  , files(o.files)
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


const QChar UniqueSeparator = QChar('/');

// returns the unique name for the domain settings
QString DomainSettings::getUniqueName(void) const
{
  QString uniqueName;
  if (!groupHierarchy.isEmpty()) {
    uniqueName = groupHierarchy.join(UniqueSeparator);
    uniqueName.append(UniqueSeparator);
  }
  uniqueName.append(domainName);
  if (!userName.isEmpty()) {
    uniqueName.append(UniqueSeparator);
    uniqueName.append(userName);
  }
  return uniqueName;
}

// replaces old with new group name
void DomainSettings::replaceGroupName(QString oldName, QString newName)
{
  for (int i = 0; i < groupHierarchy.count(); i++) {
    if (groupHierarchy.at(i) == oldName) {
      groupHierarchy[i] = newName;
    }
  }
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
      map[GROUP] = groupHierarchy.join(GroupSeparator);
    }
    if (!expiryDate.isNull()) {
      map[EXPIRY_DATE] = expiryDate;
    }
    if (!tags.isEmpty()) {
      map[TAGS] = tags.join(TagSeparator);
    }
    if (!files.isEmpty()) {
      map[FILES] = files;
    }
    if (legacyPassword.isEmpty()) {
      map[SALT] = salt_base64;
      map[ITERATIONS] = iterations;
      if (!extraCharacters.isEmpty()) {
        map[EXTRA_CHARACTERS] = extraCharacters;
      }
#ifndef OMIT_V2_CODE
      if (!usedCharacters.isEmpty()) {
        map[USED_CHARACTERS] = usedCharacters;
      }
#endif
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


QJsonDocument DomainSettings::toJsonDocument(void) const
{
  return QJsonDocument::fromVariant(toVariantMap());
}


QByteArray DomainSettings::toJson(void) const
{
  return toJsonDocument().toJson();
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
  ds.createdDate = QDateTime::fromString(map[CDATE].toString(), Qt::ISODate);
  ds.modifiedDate = QDateTime::fromString(map[MDATE].toString(), Qt::ISODate);
  ds.deleted = map[DELETED].toBool();
  ds.extraCharacters = map[EXTRA_CHARACTERS].toString();
#ifndef OMIT_V2_CODE
  ds.usedCharacters = map[USED_CHARACTERS].toString();
#endif
  ds.passwordTemplate = map[PASSWORD_TEMPLATE].toByteArray();
  ds.groupHierarchy = map[GROUP].toString().split(GroupSeparator, QString::SkipEmptyParts);
  ds.expiryDate = map[EXPIRY_DATE].toDateTime();
  ds.tags = map[TAGS].toString().split(TagSeparator, QString::SkipEmptyParts);
  ds.files = map[FILES].toMap();
  return ds;
}


DomainSettings DomainSettings::fromJson(const QByteArray &data)
{
  QJsonParseError jsonError;
  DomainSettings ds = DomainSettings::fromVariantMap(QJsonDocument::fromJson(data, &jsonError)
                                                     .toVariant()
                                                     .toMap());
  return jsonError.error == QJsonParseError::NoError
      ? ds
      : DomainSettings();
  // TODO: propagate parse errors
}


#ifndef OMIT_V2_CODE
bool DomainSettings::isV2Template(const QString &templ)
{
  return !templ.isEmpty()
      && !templ.contains('n')
      && !templ.contains('a')
      && !templ.contains('A')
      && !templ.contains('o');
}
#endif


QDebug operator<<(QDebug debug, const DomainSettings &ds)
{
  QDebugStateSaver saver(debug);
  (void)saver;
  debug.nospace()
      << "DomainSettings {\n"
      << "  " << DomainSettings::DOMAIN_NAME << ": " << ds.domainName << ",\n";
  if (ds.createdDate.isValid()) {
    debug.nospace() << "  " << DomainSettings::CDATE << ": " << ds.createdDate.toString(Qt::ISODate) << ",\n";
  }
  if (ds.modifiedDate.isValid()) {
    debug.nospace() << "  " << DomainSettings::MDATE << ": " << ds.modifiedDate.toString(Qt::ISODate) << ",\n";
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
      debug.nospace() << "  " << DomainSettings::GROUP << ": " << ds.groupHierarchy.join(DomainSettings::GroupSeparator) << ",\n";
    }
    if (!ds.expiryDate.isNull()) {
      debug.nospace() << "  " << DomainSettings::EXPIRY_DATE << ": " << ds.expiryDate << ",\n";
    }
    if (!ds.tags.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::TAGS << ": " << ds.tags.join(DomainSettings::TagSeparator) << ",\n";
    }
    if (!ds.files.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::FILES << ": " << ds.files << ",\n";
    }
    if (!ds.legacyPassword.isEmpty()) {
      debug.nospace() << "  " << DomainSettings::LEGACY_PASSWORD << ": " << ds.legacyPassword << ",\n";
    }
    else {
      debug.nospace() << "  " << DomainSettings::SALT << ": " << ds.salt_base64 << ",\n";
      debug.nospace() << "  " << DomainSettings::ITERATIONS << ": " << ds.iterations << ",\n";
      if (!ds.extraCharacters.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::EXTRA_CHARACTERS << ": " << ds.extraCharacters << ",\n";
      }
#ifndef OMIT_V2_CODE
      if (!ds.usedCharacters.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::USED_CHARACTERS << ": " << ds.usedCharacters << ",\n";
      }
#endif
      if (!ds.passwordTemplate.isEmpty()) {
        debug.nospace() << "  " << DomainSettings::PASSWORD_TEMPLATE << ": " << ds.passwordTemplate << ",\n";
      }
    }
  }
  else {
    debug.nospace() << "  " << DomainSettings::DELETED << ": true\n";
  }
  debug.nospace() << "}";
  return debug;
}
