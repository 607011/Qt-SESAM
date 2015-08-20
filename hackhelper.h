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


#ifndef __SUBSTITUTION_H_
#define __SUBSTITUTION_H_

#include <QtGlobal>
#include <QChar>
#include <QList>
#include <QString>


typedef QList<int> PositionList;

class CharacterPositions {
public:
  CharacterPositions(void)
  { /* ... */ }
  CharacterPositions(const CharacterPositions &o)
    : mPositions(o.mPositions)
    , mCh(o.mCh)
  { /* ... */ }
  CharacterPositions(int pos, QChar ch)
  {
    mPositions.append(pos);
    mCh = ch;
  }
  const PositionList &positions(void) const
  {
    return mPositions;
  }
  const QChar &character(void) const
  {
    return mCh;
  }
  void append(int pos)
  {
    mPositions.append(pos);
  }

private:
  PositionList mPositions;
  QChar mCh;
};



class PositionTable {
public:
  PositionTable(void)
    : mStrSize(0)
  { /* ... */ }
  PositionTable(const QString &str)
  {
    mStrSize = str.size();
    for (int spos = 0; spos < mStrSize; ++spos) {
      const QChar &ch = str.at(spos);
      const int pos = indexOf(ch);
      if (pos == INVALID_POSITION) {
        mSubst.append(CharacterPositions(spos, ch));
      }
      else {
        mSubst[pos].append(spos);
      }
    }
  }
  int size(void) const
  {
    return mSubst.size();
  }
  const CharacterPositions &at(int idx) const
  {
    return mSubst.at(idx);
  }
  bool operator==(const PositionTable &other) const
  {
    if (mSubst.size() != other.size())
      return false;
    for (int i = 0; i < mSubst.size(); ++i) {
      if (mSubst.at(i).positions() != other.at(i).positions())
        return false;
    }
    return true;
  }
  QString substitute(const PositionTable &newPositions, const QString &charSet) const
  {
    QString result(charSet.size(), QChar('\0'));
    for (int i = 0; i < newPositions.size(); ++i)
      result[charSet.indexOf(newPositions.at(i).character())] = mSubst.at(i).character();
    return result;
  }
  quint64 permutations(void) const
  {
    quint64 f = 1;
    for (int i = 0; i < mSubst.size(); ++i)
      f *= factorial(mSubst.at(i).positions().size());
    return factorial(mStrSize) / f;
  }

private:
  static const int INVALID_POSITION = -1;
  QList<CharacterPositions> mSubst;
  quint64 mStrSize;

private: // methods
  int indexOf(const QChar &ch) const {
    for (int i = 0; i < mSubst.size(); ++i)
      if (mSubst.at(i).character() == ch)
        return i;
    return INVALID_POSITION;
  }
  static quint64 factorial(unsigned int x)
  {
    quint64 f = 1;
    for (int i = 2; i <= x; ++i)
      f *= i;
    return f;
  }
};


QDebug operator<<(QDebug debug, const PositionTable &sub)
{
    QDebugStateSaver saver(debug);
    (void)saver;
    debug.nospace() << "PositionTable{\n";
    for (int i = 0; i < sub.size(); ++i) {
      const CharacterPositions &s = sub.at(i);
      debug.nospace() << s.character() << " @ " << s.positions() << "\n";
    }
    debug.nospace() << "}";
    return debug;
}

#endif // __SUBSTITUTION_H_

