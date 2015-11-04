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


#ifndef __TREEITEM_H_
#define __TREEITEM_H_

#include <QList>
#include <QVariant>
#include "domainsettings.h"


class TreeItem
{
public:
  explicit TreeItem(TreeItem *parentItem = Q_NULLPTR);
  TreeItem(const DomainSettings &data, TreeItem *parentItem = Q_NULLPTR);
  ~TreeItem();

  void appendChild(TreeItem *child);
  TreeItem *child(int row);
  int childCount(void) const;
  int columnCount(void) const;
  QVariant data(int column) const;
  int row(void) const;
  TreeItem *parentItem(void);

private:
  QList<TreeItem*> mChildItems;
  DomainSettings mItemData;
  TreeItem *mParentItem;
};
#endif // __TREEITEM_H_
