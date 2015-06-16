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

#include "domainsettingslist.h"

#include <QtDebug>


DomainSettingsList::DomainSettingsList(void)
  : mDirty(false)
{
  // ...
}


bool DomainSettings::isEmpty(void) const
{
  return domainName.isEmpty();
}


DomainSettings DomainSettingsList::at(const QString &domainName) const
{
  for (DomainSettingsList::const_iterator ds = constBegin(); ds != constEnd(); ++ds)
    if (ds->domainName == domainName)
      return *ds;
  return DomainSettings();
}


DomainSettings DomainSettingsList::at(int idx) const
{
  return QList<DomainSettings>::at(idx);
}


void DomainSettingsList::remove(const QString &domainName)
{
  qDebug() << "DomainSettingsList::remove(" << domainName << ")";
  int toDeleteIdx = -1;
  for (int i = 0; i < count(); ++i) {
    if (at(i).domainName == domainName) {
      toDeleteIdx = i;
      break;
    }
  }
  if (toDeleteIdx > -1)
    removeAt(toDeleteIdx);
  setDirty();
}


void DomainSettingsList::updateWith(const DomainSettings &src)
{
  qDebug() << "DomainSettingsList::updateWith(" << src.domainName << ")";
  bool found = false;
  for (DomainSettingsList::iterator d = begin(); d != end() && !found; ++d) {
    if (d->domainName == src.domainName) {
      *d = src;
      found = true;
    }
  }
  if (!found)
    append(src);
  setDirty();
}


QByteArray DomainSettingsList::toJson(void) const
{
  QVariantMap domains;
  for (DomainSettingsList::const_iterator d = constBegin(); d != constEnd(); ++d)
    domains[d->domainName] = d->toVariantMap();
  return QJsonDocument::fromVariant(domains).toJson(QJsonDocument::Compact);
}


QStringList DomainSettingsList::keys(void) const
{
  QStringList names;
  for (DomainSettingsList::const_iterator d = constBegin(); d != constEnd(); ++d)
    names << d->domainName;
  return names;
}


DomainSettingsList DomainSettingsList::fromQJsonDocument(const QJsonDocument &json)
{
  DomainSettingsList dl;
  const QVariantMap &map = json.toVariant().toMap();
  foreach(QString key, map.keys()) {
    DomainSettings ds = DomainSettings::fromVariantMap(map[key].toMap());
    if (key.size() > 0)
      dl << ds;
  }
  return dl;
}


bool DomainSettingsList::isDirty(void) const
{
  return mDirty;
}


void DomainSettingsList::setDirty(bool dirty)
{
  mDirty = dirty;
}
