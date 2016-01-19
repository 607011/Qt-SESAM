/*

    Copyright (c) 2016 Egbert van der Haring

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

#include "passwordsafereader.h"
#include "domainsettings.h"
#include "domainsettingslist.h"
#include "util.h"

#include <QDebug>
#include <QFile>
#include <QVector>


class PasswordSafeReaderPrivate {
public:
    PasswordSafeReaderPrivate(void)
        : ok(true)
        , errorLine(-1)
        , errorColumn(-1)
        , groupNames(10)
    { /* ... */ }
    ~PasswordSafeReaderPrivate()
    { /* ... */ }
    QFile dataFile;
    bool ok;
    int errorLine;
    int errorColumn;
    QString dataErrorString;
    QString errorString;
    DomainSettingsList domains;
    QVector<QString> groupNames;
};


PasswordSafeReader::PasswordSafeReader(const QString &filename)
    : d_ptr(new PasswordSafeReaderPrivate)
{
    Q_D(PasswordSafeReader);
    d->dataFile.setFileName(filename);
    d->ok = d->dataFile.open(QIODevice::ReadOnly);
    d->errorString = d->ok ? QString() : d->dataFile.errorString();
    if (d->ok) {
        d->ok = parse();
    }
    d->dataFile.close();
}


PasswordSafeReader::~PasswordSafeReader()
{
    /* ... */
}


bool PasswordSafeReader::parse()
{
    Q_D(PasswordSafeReader);
    bool valid = true;
    bool firstLine = true;
    static const int MaxLineSize = 2048;
    char buf[MaxLineSize];
    while (valid && (!d->dataFile.atEnd())) {
        DomainSettings ds;
        const qint64 bytesRead = d->dataFile.readLine(buf, MaxLineSize);
        const QString line = QByteArray(buf, bytesRead).trimmed();
        QStringList fields = line.split('\t');
        if (firstLine) {
            // skip first line
            firstLine = false;
        } else {
            ds.group = fields[0];
            ds.domainName = fields[0];
            ds.domainName.append(" [");
            ds.domainName.append(fields[1]);
            ds.domainName.append("]");
            ds.userName = fields[1];
            ds.url = fields[3];
            ds.createdDate = QDateTime::fromString(fields[5],"yyyy/MM/dd hh:mm:ss");
            ds.modifiedDate = QDateTime::fromString(fields[6],"yyyy/MM/dd hh:mm:ss");
            if (ds.modifiedDate.isNull()) {
                ds.modifiedDate = ds.createdDate;
            }
            ds.expiryDate = QDateTime::fromString(fields[8],"yyyy/MM/dd hh:mm:ss");
            ds.notes = fields[16];
            if (ds.notes[0] == '"') {
                ds.notes = ds.notes.remove(0,1); // remove start quote
                ds.notes.chop(1);                // remove end quote
                ds.notes.replace("»", "\n");
            }
            ds.legacyPassword.append(fields[2]); // = doesn't work
            d->domains.append(ds);
        }
    }
    return valid;
}


bool PasswordSafeReader::isOpen(void) const
{
    return d_ptr->dataFile.isOpen();
}


bool PasswordSafeReader::isValid(void) const
{
    return d_ptr->ok;
}


QString PasswordSafeReader::errorString(void) const
{
    return d_ptr->errorString;
}


QString PasswordSafeReader::dataErrorString(void) const
{
    return d_ptr->dataErrorString;
}


int PasswordSafeReader::errorLine(void) const
{
    return d_ptr->errorLine;
}


int PasswordSafeReader::errorColumn(void) const
{
    return d_ptr->errorColumn;
}


DomainSettingsList PasswordSafeReader::domains(void) const
{
    return d_ptr->domains;
}
