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

#ifndef PASSWORDSAFEREADER_H
#define PASSWORDSAFEREADER_H

#include <QScopedPointer>
#include <QDomElement>
#include <QString>

#include "domainsettingslist.h"

class PasswordSafeReaderPrivate;

class PasswordSafeReader
{
public:
    PasswordSafeReader(const QString &filename);
    ~PasswordSafeReader();
    bool isOpen(void) const;
    bool isValid(void) const;
    QString errorString(void) const;
    QString dataErrorString(void) const;
    int errorColumn(void) const;
    int errorLine(void) const;
    DomainSettingsList domains(void) const;

  private:
    QScopedPointer<PasswordSafeReaderPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PasswordSafeReader)
    Q_DISABLE_COPY(PasswordSafeReader)

private: // methods
    bool parse();
};

#endif // PASSWORDSAFEREADER_H



