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

#include "treeitem.h"

TreeItem::TreeItem(TreeItem *parentItem)
  : mParentItem(parentItem)
{
  /* ... */
}

TreeItem::TreeItem(const DomainSettings &data, TreeItem *parentItem)
  : mParentItem(parentItem)
  , mItemData(data)
{
  /* ... */
}


TreeItem::~TreeItem()
{
  qDeleteAll(mChildItems);
}


void TreeItem::appendChild(TreeItem *item)
{
  mChildItems.append(item);
}


TreeItem *TreeItem::child(int row)
{
  return mChildItems.value(row);
}


int TreeItem::childCount(void) const
{
  return mChildItems.count();
}


int TreeItem::columnCount(void) const
{
  return 4;
}


QVariant TreeItem::data(int column) const
{
  switch (column) {
  case 0:
    return mItemData.domainName;
  case 1:
    return mItemData.userName;
  case 2:
    return mItemData.url;
  case 3:
    return mItemData.groupHierarchy.join(QChar('/'));
  default:
    break;
  }
  return QString("<invalid>");
}


TreeItem *TreeItem::parentItem(void)
{
  return mParentItem;
}


int TreeItem::row(void) const
{
  if (mParentItem)
    return mParentItem->mChildItems.indexOf(const_cast<TreeItem*>(this));
  return 0;
}
