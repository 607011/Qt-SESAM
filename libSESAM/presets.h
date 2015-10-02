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

#ifndef __PRESETS_H_
#define __PRESETS_H_

#include <QObject>
#include <QMap>
#include <QList>
#include <QChar>
#include <QString>

class Preset {
public:
  typedef QMap<QChar, QString> TemplateCharacterMap;
  typedef QList<QByteArray> TemplateType;

  Preset(void)
    : shuffle(true)
  { /* ... */ }
  Preset(const TemplateType &templates, bool shuffle)
    : templates(templates)
    , shuffle(shuffle)
  { /* ... */ }

  typedef QMap<QString, Preset> PresetType;

  static const TemplateCharacterMap TemplateCharacters;
  static const PresetType Presets;

private:
  TemplateType templates;
  bool shuffle;
};


#endif // __PRESETS_H_
