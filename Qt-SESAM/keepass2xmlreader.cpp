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
#include <QString>
#include <QVector>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

#include <functional>

class KeePass2XmlReaderPrivate {
public:
  KeePass2XmlReaderPrivate(void)
    : xmlFile(Q_NULLPTR)
    , ok(true)
    , xmlErrorLine(-1)
    , xmlErrorColumn(-1)
  { /* ... */ }
  ~KeePass2XmlReaderPrivate()
  { /* ... */ }
  QFile *xmlFile;
  QDomDocument xml;
  bool ok;
  int xmlErrorLine;
  int xmlErrorColumn;
  QString errorString;
  DomainSettingsList domains;
};


KeePass2XmlReader::KeePass2XmlReader(const QString &xmlFilename)
  : d_ptr(new KeePass2XmlReaderPrivate)
{
  Q_D(KeePass2XmlReader);
  d->xmlFile = new QFile(xmlFilename);
  bool ok = d->xmlFile->open(QIODevice::ReadOnly);
  d->errorString = ok ? QString() : d->xmlFile->errorString();
  d->ok = d->xml.setContent(d->xmlFile, &d->errorString, &d->xmlErrorLine, &d->xmlErrorColumn);
  std::function<QDomElement(const QDomElement &root, const QString &tagName)> findChildByTagName;
  findChildByTagName = [](const QDomElement &root, const QString &tagName) {
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
  };

  QVector<QString> groupNames(10);
  std::function<QString(int level)> groupHierarchy;
  groupHierarchy = [&groupNames](int level) {
    QString g;
    for (int l = 2; l <= level; ++l) {
      g.append(groupNames[l]);
      if (l < level)
        g.append("/");
    }
    return g;
  };

  std::function<void(const QDomElement &e, int)> drillDown;
  drillDown = [&](const QDomElement &e, int level) {
    if (level < groupNames.size()) {
      QDomNode n = e.firstChild();
      while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
          if (e.tagName() == "Group") {
            QDomElement groupName = findChildByTagName(e, "Name");
            if (!groupName.isNull()) {
              groupNames[level] = groupName.text();
              qDebug().nospace().noquote() << groupHierarchy(level);
            }
            else {
              qWarning() << "warning: group has no name";
            }
            QDomElement entry = findChildByTagName(e, "Entry");
            if (!entry.isNull()) {
              DomainSettings ds;
              ds.group = groupHierarchy(level);
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
                  if (!eExpires.isNull() && eExpires.text() != "false") {
                    QDomElement eExpiryTime = findChildByTagName(eChild, "Expires");
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
          drillDown(e, level + 1);
        }
        n = n.nextSibling();
      }
    }
  };
  drillDown(d->xml.documentElement(), 0);

  close();
}


KeePass2XmlReader::~KeePass2XmlReader()
{
  /* ... */
}


void KeePass2XmlReader::close(void)
{
  Q_D(KeePass2XmlReader);
  if (d->xmlFile != Q_NULLPTR) {
    d->xmlFile->close();
    SafeDelete(d->xmlFile);
  }
}


bool KeePass2XmlReader::isOpen(void) const
{
  return d_ptr->xmlFile->isOpen();
}


bool KeePass2XmlReader::isValid(void) const
{
  return d_ptr->ok;
}


QString KeePass2XmlReader::errorString(void) const
{
  return d_ptr->errorString;
}


int KeePass2XmlReader::errorLine(void) const
{
  return d_ptr->xmlErrorLine;
}


int KeePass2XmlReader::errorColumn(void) const
{
  return d_ptr->xmlErrorColumn;
}


DomainSettingsList KeePass2XmlReader::domains(void) const
{
  return d_ptr->domains;
}
