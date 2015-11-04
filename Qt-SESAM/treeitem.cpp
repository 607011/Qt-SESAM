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

TreeItem::TreeItem(const DomainSettings &data, TreeItem *parent)
{
  m_parentItem = parent;
  m_itemData = data;
}


TreeItem::~TreeItem()
{
  qDeleteAll(m_childItems);
}


void TreeItem::appendChild(TreeItem *item)
{
  m_childItems.append(item);
}


TreeItem *TreeItem::child(int row)
{
  return m_childItems.value(row);
}


int TreeItem::childCount(void) const
{
  return m_childItems.count();
}


int TreeItem::columnCount(void) const
{
  return m_itemData.count();
}


QVariant TreeItem::data(int column) const
{
  switch (column) {
  case 0:
    return m_itemData.domainName;
    break;
  case 1:
    return m_itemData.userName;
    break;
  }
}


TreeItem *TreeItem::parentItem(void)
{
  return m_parentItem;
}


int TreeItem::row(void) const
{
  if (m_parentItem)
    return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
  return 0;
}
