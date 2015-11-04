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
#include "treemodel.h"


TreeModel::TreeModel(const DomainSettingsList &data, QObject *parent)
  : QAbstractItemModel(parent)
  , rootItem(new TreeItem)
{
  setupModelData(data, rootItem);
}


TreeModel::~TreeModel()
{
  delete rootItem;
}


int TreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
  else
    return rootItem->columnCount();
}


QVariant TreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  TreeItem *item = reinterpret_cast<TreeItem*>(index.internalPointer());
  return item->data(index.column());
}


Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;
  return QAbstractItemModel::flags(index);
}


QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);
  return QVariant();
}


QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  TreeItem *parentItem;
  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = reinterpret_cast<TreeItem*>(parent.internalPointer());
  TreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();
  TreeItem *childItem = reinterpret_cast<TreeItem*>(index.internalPointer());
  TreeItem *parentItem = childItem->parentItem();
  if (parentItem == rootItem)
    return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}


int TreeModel::rowCount(const QModelIndex &parent) const
{
  TreeItem *parentItem;
  if (parent.column() > 0)
    return 0;
  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = reinterpret_cast<TreeItem*>(parent.internalPointer());
  return parentItem->childCount();
}


void TreeModel::setupModelData(const DomainSettingsList &domainSettingsList, TreeItem *parent)
{
  QList<TreeItem*> parents;
  parents << parent;
  foreach (DomainSettings ds, domainSettingsList) {
    if (!ds.deleted) {
      // TODO: build tree respecting , not flat
      parents.last()->appendChild(new TreeItem(ds, parents.last()));
    }
  }
}
