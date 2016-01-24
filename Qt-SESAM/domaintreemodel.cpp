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

#include <QDataStream>

#include "groupnode.h"
#include "domainnode.h"
#include "domaintreemodel.h"
#include "util.h"


DomainTreeModel::DomainTreeModel(QObject *parent)
  : QAbstractItemModel(parent)
  , mRootItem(Q_NULLPTR)
{
  /* ... */
}


DomainTreeModel::~DomainTreeModel()
{
  SafeDelete(mRootItem);
}


GroupNode *DomainTreeModel::findChild(const QString &name, GroupNode *node) {
  GroupNode *foundChild = Q_NULLPTR;
  for (int row = 0; row < node->childCount(); ++row) {
    AbstractTreeNode *child = node->child(row);
    if (child->type() == AbstractTreeNode::GroupType) {
      GroupNode *groupNode = reinterpret_cast<GroupNode*>(child);
      if (groupNode->name() == name) {
        foundChild = groupNode;
        break;
      }
    }
  }
  return foundChild;
}


GroupNode *DomainTreeModel::addToHierarchy(const QStringList &groups, GroupNode *node) {
  GroupNode *parentNode = node;
  foreach (QString groupName, groups) {
    GroupNode *childNode = findChild(groupName, parentNode);
    if (childNode == Q_NULLPTR) {
      childNode = new GroupNode(groupName, parentNode);
      parentNode->appendChild(childNode);
    }
    parentNode = childNode;
  }
  return parentNode;
}


void DomainTreeModel::populate(const DomainSettingsList &domainSettingsList)
{
  SafeRenew<GroupNode*>(mRootItem, new GroupNode);
  foreach (DomainSettings ds, domainSettingsList) {
    if (!ds.deleted) {
      GroupNode *node = addToHierarchy(ds.groupHierarchy, mRootItem);
      node->appendChild(new DomainNode(ds, node));
    }
  }
}


int DomainTreeModel::columnCount(const QModelIndex &parent) const
{
  return parent.isValid()
      ? reinterpret_cast<AbstractTreeNode*>(parent.internalPointer())->columnCount()
      : mRootItem->columnCount();
}


DomainNode *DomainTreeModel::node(const QModelIndex &index) const
{
  return index.isValid()
      ? reinterpret_cast<DomainNode*>(index.internalPointer())
      : Q_NULLPTR;
}


QVariant DomainTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();
  AbstractTreeNode *item = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
  if (item->type() == AbstractTreeNode::LeafType)
    return item->data(index.column());
  if (index.column() == 0)
    return item->data(0);
  return QVariant();
}


Qt::ItemFlags DomainTreeModel::flags(const QModelIndex &index) const
{
  const Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
  return index.isValid()
      ? Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags
      : Qt::ItemIsDropEnabled | defaultFlags;
}


QVariant DomainTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: {
      return tr("Domain");
    }
    case 1: {
      return tr("User");
    }
    case 2: {
      return tr("URL");
    }
    case 3: {
      return tr("Group");
    }
    default: {
      break;
    }
    }
  }
  return QVariant();
}


QModelIndex DomainTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (hasIndex(row, column, parent)) {
    AbstractTreeNode *parentItem = parent.isValid()
        ? reinterpret_cast<AbstractTreeNode*>(parent.internalPointer())
        : parentItem = mRootItem;
    AbstractTreeNode *childItem = parentItem->child(row);
    return childItem != Q_NULLPTR
        ? createIndex(row, column, childItem)
        : QModelIndex();
  }
  return QModelIndex();
}


QModelIndex DomainTreeModel::parent(const QModelIndex &index) const
{
  if (index.isValid()) {
    AbstractTreeNode *childItem = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
    AbstractTreeNode *parentItem = childItem->parentItem();
    return parentItem == mRootItem
        ? QModelIndex()
        : createIndex(parentItem->row(), 0, parentItem);
  }
  return QModelIndex();
}


int DomainTreeModel::rowCount(const QModelIndex &parent) const
{
  AbstractTreeNode *parentItem;
  if (parent.column() > 0)
    return 0;
  if (!parent.isValid()) {
    parentItem = mRootItem;
  }
  else {
    parentItem = reinterpret_cast<AbstractTreeNode*>(parent.internalPointer());
  }
  return parentItem->childCount();
}


Qt::DropActions DomainTreeModel::supportedDropActions(void) const
{
  return Qt::MoveAction;
}


QStringList DomainTreeModel::mimeTypes(void) const
{
  static const QStringList MimeTypes = (QStringList() << "application/json");
  return MimeTypes;
}


QMimeData *DomainTreeModel::mimeData(const QModelIndexList &indexes) const
{
  QMimeData *mimeData = new QMimeData;
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);
  foreach (const QModelIndex &index, indexes) {
    if (index.isValid()) {
      const DomainNode *const n = node(index);
      stream << n->itemData().toJson();
    }
  }
  mimeData->setData("application/json", encodedData);
  return mimeData;
}


bool DomainTreeModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(parent);
  if (!data->hasFormat("application/json"))
    return false;
  if (column > 0)
    return false;
  return true;
}


bool DomainTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (!canDropMimeData(data, action, row, column, parent))
    return false;
  if (action == Qt::IgnoreAction)
    return true;
  int beginRow;
  if (row != -1) {
    beginRow = row;
  }
  else if (parent.isValid()) {
    beginRow = parent.row();
  }
  else {
    beginRow = rowCount(QModelIndex());
  }
  QByteArray encodedData = data->data("application/json");
  qDebug() << encodedData;

//  QDataStream stream(&encodedData, QIODevice::ReadOnly);
//  QStringList newItems;
//  int rows = 0;
//  while (!stream.atEnd()) {
//    QString text;
//    stream >> text;
//    newItems << text;
//    ++rows;
//  }
//  insertRows(beginRow, rows, QModelIndex());
//  foreach (const QString &text, newItems) {
//    QModelIndex idx = index(beginRow, 0, QModelIndex());
//    setData(idx, text);
//    ++beginRow;
//  }
  return true;
}
