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

#include <QDebug>
#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

#include "util.h"

class Preset {
public:
  typedef QMap<char, QString> TemplateCharacterMap;
  typedef QList<QByteArray> TemplateList;

  Preset(void)
    : mShuffle(true)
  { /* ... */ }

  Preset(const Preset &other)
    : mTemplates(other.mTemplates)
    , mShuffle(other.mShuffle)
  { /* ... */ }

  Preset(const QString &name, const TemplateList &templates, bool shuffle)
    : mTemplates(templates)
    , mShuffle(shuffle)
  { /* ... */ }

  QByteArray randomTemplate(void) const
  {
    const QByteArray &templ = mTemplates.at(qrand() % mTemplates.size());
    return mShuffle ? shuffled(templ) : templ;
  }

  bool isNull(void) const
  {
    return mTemplates.isEmpty();
  }

  const QString &name(void) const
  {
    return mName;
  }

  const TemplateList &templates(void) const
  {
    return mTemplates;
  }

  bool doShuffle(void) const
  {
    return mShuffle;
  }

  static const QString &charSetFor(char);
  static const Preset &presetFor(const QString &name);

  typedef QList<Preset> PresetList;

  static const TemplateCharacterMap TemplateCharacters;
  static const PresetList Presets;

private:
  QString mName;
  TemplateList mTemplates;
  bool mShuffle;
};


#endif // __PRESETS_H_
