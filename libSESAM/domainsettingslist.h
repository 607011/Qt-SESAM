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


#ifndef __DOMAINSETTINGSLIST_H_
#define __DOMAINSETTINGSLIST_H_

#include <QByteArray>
#include <QStringList>
#include <QJsonDocument>

#include "domainsettings.h"

class DomainSettingsList : public QList<DomainSettings> {
public:
  DomainSettingsList(void);
  DomainSettings at(int idx) const;
  DomainSettings at(const QString &domainName) const;
  void remove(const QString &domainName);
  void updateWith(const DomainSettings &);
  QByteArray toJson(void) const;
  QJsonDocument toJsonDocument(void) const;
  QStringList keys(void) const;
  static DomainSettingsList fromQJsonDocument(const QJsonDocument &);

  bool isDirty(void) const;
  void setDirty(bool dirty = true);

private:
  bool mDirty;
};




#endif // __DOMAINSETTINGSLIST_H_
