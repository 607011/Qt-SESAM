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


#include "keepass2xmlreader.h"
#include "domainsettings.h"
#include "domainsettingslist.h"
#include "util.h"

#include <QDebug>
#include <QFile>
#include <QVector>
#include <QDomDocument>
#include <QDomNode>


class KeePass2XmlReaderPrivate {
public:
  KeePass2XmlReaderPrivate(void)
    : ok(true)
    , xmlErrorLine(-1)
    , xmlErrorColumn(-1)
    , groupNames(10)
  { /* ... */ }
  ~KeePass2XmlReaderPrivate()
  { /* ... */ }
  QFile xmlFile;
  QDomDocument xml;
  bool ok;
  int xmlErrorLine;
  int xmlErrorColumn;
  QString xmlErrorString;
  QString errorString;
  DomainSettingsList domains;
  QVector<QString> groupNames;
};


KeePass2XmlReader::KeePass2XmlReader(const QString &xmlFilename)
  : d_ptr(new KeePass2XmlReaderPrivate)
{
  Q_D(KeePass2XmlReader);
  d->xmlFile.setFileName(xmlFilename);
  d->ok = d->xmlFile.open(QIODevice::ReadOnly);
  d->errorString = d->ok ? QString() : d->xmlFile.errorString();
  if (d->ok) {
    d->ok = d->xml.setContent(&d->xmlFile, &d->xmlErrorString, &d->xmlErrorLine, &d->xmlErrorColumn);
    if (d->ok) {
      parseXml(d->xml.documentElement(), 0);
    }
  }
  d->xmlFile.close();
}


KeePass2XmlReader::~KeePass2XmlReader()
{
  /* ... */
}


QString KeePass2XmlReader::groupHierarchy(int level)
{
  Q_D(KeePass2XmlReader);
  QString g;
  for (int l = 2; l <= level; ++l) {
    g.append(d->groupNames[l]);
    if (l < level) {
      g.append("/");
    }
  }
  return g;
}


QDomElement KeePass2XmlReader::findChildByTagName(const QDomElement &root, const QString &tagName)
{
  if (!root.isNull() && root.hasChildNodes()) {
    QDomNode child = root.firstChild();
    while (!child.isNull()) {
      QDomElement e = child.toElement();
      if (!e.isNull()) {
        if (e.tagName() == tagName) {
          return e;
        }
      }
      child = child.nextSibling();
    }
  }
  return QDomElement();
}


void KeePass2XmlReader::parseXml(const QDomElement &e, int level)
{
  Q_D(KeePass2XmlReader);
  if (level < d->groupNames.size()) {
    QDomNode n = e.firstChild();
    while (!n.isNull()) {
      QDomElement e = n.toElement();
      if (!e.isNull()) {
        if (e.tagName() == "Group") {
          QDomElement groupName = findChildByTagName(e, "Name");
          if (!groupName.isNull()) {
            d->groupNames[level] = groupName.text();
          }
          else {
            qWarning() << "warning: group has no name";
          }
          QDomElement entry = findChildByTagName(e, "Entry");
          if (!entry.isNull()) {
            DomainSettings ds;
            ds.groupHierarchy = groupHierarchy(level);
            QDomNode child = entry.firstChild();
            while (!child.isNull()) {
              QDomElement eChild = child.toElement();
              if (eChild.tagName() == "String") {
                QDomElement eKey = findChildByTagName(eChild, "Key");
                QDomElement eValue = findChildByTagName(eChild, "Value");
                if (!eKey.isNull() && !eValue.isNull() && !eValue.text().isNull()) {
                  if (eKey.text() == "Notes") {
                    ds.notes = eValue.text();
                  }
                  else if (eKey.text() == "Title") {
                    ds.domainName = eValue.text();
                  }
                  else if (eKey.text() == "URL") {
                    ds.url = eValue.text();
                  }
                  else if (eKey.text() == "UserName") {
                    ds.userName = eValue.text();
                  }
                  else if (eKey.text() == "Password") {
                    ds.legacyPassword = eValue.text();
                  }
                }
              }
              else if (eChild.tagName() == "Times") {
                QDomElement eCreated = findChildByTagName(eChild, "CreationTime");
                if (!eCreated.isNull())
                  ds.createdDate = QDateTime::fromString(eCreated.text(), "yyyy-MM-ddThh:mm:ssZ");
                QDomElement eModified = findChildByTagName(eChild, "LastModificationTime");
                if (!eModified.isNull())
                  ds.createdDate = QDateTime::fromString(eModified.text(), "yyyy-MM-ddThh:mm:ssZ");
                QDomElement eExpires = findChildByTagName(eChild, "Expires");
                if (!eExpires.isNull() && eExpires.text() == "True") {
                  QDomElement eExpiryTime = findChildByTagName(eChild, "ExpiryTime");
                  if (!eExpiryTime.isNull())
                    ds.expiryDate = QDateTime::fromString(eExpiryTime.text(), "yyyy-MM-ddThh:mm:ssZ");
                }
              }
              child = child.nextSibling();
            }
            d->domains.append(ds);
          }
        }
      }
      if (e.hasChildNodes()) {
        parseXml(e, level + 1);
      }
      n = n.nextSibling();
    }
  }
}


bool KeePass2XmlReader::isOpen(void) const
{
  return d_ptr->xmlFile.isOpen();
}


bool KeePass2XmlReader::isValid(void) const
{
  return d_ptr->ok;
}


QString KeePass2XmlReader::errorString(void) const
{
  return d_ptr->errorString;
}


QString KeePass2XmlReader::xmlErrorString(void) const
{
  return d_ptr->xmlErrorString;
}


int KeePass2XmlReader::xmlErrorLine(void) const
{
  return d_ptr->xmlErrorLine;
}


int KeePass2XmlReader::xmlErrorColumn(void) const
{
  return d_ptr->xmlErrorColumn;
}


DomainSettingsList KeePass2XmlReader::domains(void) const
{
  return d_ptr->domains;
}
