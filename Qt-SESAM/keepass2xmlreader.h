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


#ifndef __KEEPASS2XMLREADER_H_
#define __KEEPASS2XMLREADER_H_


#include <QScopedPointer>
#include "domainsettingslist.h"

class KeePass2XmlReaderPrivate;

class KeePass2XmlReader
{
public:
  KeePass2XmlReader(const QString &xmlFilename);
  ~KeePass2XmlReader();
  void close(void);
  bool isOpen(void) const;
  bool isValid(void) const;
  QString errorString(void) const;
  int errorColumn(void) const;
  int errorLine(void) const;
  DomainSettingsList domains(void) const;

private:
  QScopedPointer<KeePass2XmlReaderPrivate> d_ptr;
  Q_DECLARE_PRIVATE(KeePass2XmlReader)
  Q_DISABLE_COPY(KeePass2XmlReader)
};

#endif // __KEEPASS2XMLREADER_H_
