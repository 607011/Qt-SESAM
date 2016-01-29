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

class DomainTreeModelPrivate {
public:
  DomainTreeModelPrivate(void)
    : rootItem(Q_NULLPTR)
  { /* ... */ }
  ~DomainTreeModelPrivate()
  {
    SafeDelete(rootItem);
  }
  GroupNode *rootItem;
};


DomainTreeModel::DomainTreeModel(QObject *parent)
  : QAbstractItemModel(parent)
  , d_ptr(new DomainTreeModelPrivate)
{
  /* ... */
}


DomainTreeModel::~DomainTreeModel()
{
  { /* ... */ }
}


GroupNode *DomainTreeModel::findChild(const QString &name, GroupNode *node)
{
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


GroupNode *DomainTreeModel::addToHierarchy(const QStringList &groups, GroupNode *node)
{
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


QModelIndex DomainTreeModel::populate(const DomainSettingsList &domainSettingsList)
{
  Q_D(DomainTreeModel);
  SafeRenew<GroupNode*>(d->rootItem, new GroupNode);
  foreach (DomainSettings ds, domainSettingsList) {
    if (!ds.deleted) {
      GroupNode *node = addToHierarchy(ds.groupHierarchy, d->rootItem);
      node->appendChild(new DomainNode(ds, node));
    }
  }
  AbstractTreeNode *childItem = d->rootItem->child(0);
  return childItem != Q_NULLPTR
      ? createIndex(0, 0, childItem)
      : QModelIndex();
}


void DomainTreeModel::addNewGroup(const QModelIndex &index)
{
    AbstractTreeNode *parentNode = this->node(index);
    if (parentNode != Q_NULLPTR) {
      GroupNode *groupNode = new GroupNode("New group", parentNode);
      parentNode->appendChild(groupNode);
    }
}


QStringList DomainTreeModel::getGroupHierarchy(const QModelIndex &index)
{
  Q_D(DomainTreeModel);
  QStringList groups = QStringList();
  if (index.isValid()) {
    GroupNode *groupNode = NULL;
    AbstractTreeNode *currentNode = this->node(index);
    if (currentNode->type() == AbstractTreeNode::LeafType) {
      groupNode = reinterpret_cast<GroupNode*> (currentNode->parentItem());
    } else {
      groupNode = reinterpret_cast<GroupNode*> (currentNode);
    }
    while ((groupNode) && (groupNode != d->rootItem)) {
      groups.prepend(groupNode->name());
      groupNode = reinterpret_cast<GroupNode*> (groupNode->parentItem());
    }
  }
  return groups;
}


void DomainTreeModel::removeDomain(const QModelIndex &index)
{
  if (index.isValid()) {
    AbstractTreeNode *currentNode = this->node(index);
    if (currentNode->type() == AbstractTreeNode::LeafType) {
      GroupNode *groupNode = reinterpret_cast<GroupNode*> (currentNode->parentItem());
      groupNode->removeChild(currentNode);
    }
  }
}


int DomainTreeModel::addDomain(const DomainSettings &ds)
{
  Q_D(DomainTreeModel);
  GroupNode *nodeGroup = addToHierarchy(ds.groupHierarchy, d->rootItem);
  DomainNode *nodeDomain = new DomainNode(ds, nodeGroup);
  nodeGroup->appendChild(nodeDomain);
  return nodeDomain->row();
}


void DomainTreeModel::appendDomains(GroupNode *groupNode, DomainSettingsList *domains)
{
  for (int row = 0; row < groupNode->childCount(); ++row) {
    AbstractTreeNode *child = groupNode->child(row);
    if (child->type() == AbstractTreeNode::GroupType) {
      appendDomains(reinterpret_cast<GroupNode*> (child), domains);
    }
    else if (child->type() == AbstractTreeNode::LeafType) {
      DomainNode *domainNode = reinterpret_cast<DomainNode*> (child);
      DomainSettings ds = domainNode->itemData();
      domains->append(ds);
    }
  }
}


DomainSettingsList DomainTreeModel::getAllDomains()
{
  Q_D(DomainTreeModel);
  DomainSettingsList domains;
  appendDomains(d->rootItem, &domains);
  return domains;
}


void DomainTreeModel::replaceGroupName(QString oldName, QString newName, GroupNode *node)
{
  for (int row = 0; row < node->childCount(); ++row) {
    AbstractTreeNode *child = node->child(row);
    if (child->type() == AbstractTreeNode::GroupType) {
      replaceGroupName(oldName, newName, reinterpret_cast<GroupNode*> (child));
    }
    else if (child->type() == AbstractTreeNode::LeafType) {
      DomainNode *domainNode = reinterpret_cast<DomainNode*> (child);
      DomainSettings ds = domainNode->itemData();
      ds.replaceGroupName(oldName, newName);
      domainNode->changeDomainSettings(ds);
    }
  }
}

int DomainTreeModel::columnCount(const QModelIndex &parent) const
{
  // don't know where this is set, but I only want one column
  return 1;
  /*
  return parent.isValid()
      ? reinterpret_cast<DomainNode*>(parent.internalPointer())->columnCount()
      : d_ptr->rootItem->columnCount();
      */
}


DomainNode *DomainTreeModel::node(const QModelIndex &index) const
{
  return index.isValid()
      ? reinterpret_cast<DomainNode*>(index.internalPointer())
      : Q_NULLPTR;
}


QVariant DomainTreeModel::data(const QModelIndex &index, int role) const
{
  QVariant data = QVariant();
  if (index.isValid()) {
    AbstractTreeNode *item = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
      if (item->type() == AbstractTreeNode::LeafType) {
        data = item->data(index.column());
      }
      if (index.column() == 0) {
        data = item->data(0);
      }
    }
    else if ((role == Qt::EditRole) && (item->type() == AbstractTreeNode::GroupType)) {
      GroupNode *groupNode = reinterpret_cast<GroupNode*> (item);
      data = groupNode->name();
    }
  }
  return data;
}


bool DomainTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool result = false;
  if (role == Qt::EditRole) {
    AbstractTreeNode *item = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
    if (item->type() == AbstractTreeNode::GroupType) {
      GroupNode *groupNode = reinterpret_cast<GroupNode*> (item);
      QString newName = value.toString();
      if (groupNode->name() != newName) {
        replaceGroupName(groupNode->name(), newName, groupNode);
        groupNode->setName(value.toString());
        emit groupNameChanged();
        result = true;
      }
    }
  }
  return result;
}


Qt::ItemFlags DomainTreeModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
  if (index.isValid()) {
    itemFlags = itemFlags | Qt::ItemIsDragEnabled;
    AbstractTreeNode *item = reinterpret_cast<AbstractTreeNode*>(index.internalPointer());
    if (item->type() == AbstractTreeNode::GroupType) {
      itemFlags = itemFlags | Qt::ItemIsEditable;
    }
  }
  return itemFlags;
}


QVariant DomainTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0: {
      return tr("Domain");
    }
      /*
    case 1: {
      return tr("User");
    }
    case 2: {
      return tr("URL");
    }
    case 3: {
      return tr("Tags");
    }
    */
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
        : parentItem = d_ptr->rootItem;
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
    return parentItem == d_ptr->rootItem
        ? QModelIndex()
        : createIndex(parentItem->row(), 0, parentItem);
  }
  return QModelIndex();
}


int DomainTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0) {
    return 0;
  }
  AbstractTreeNode *parentItem = parent.isValid()
      ? reinterpret_cast<AbstractTreeNode*>(parent.internalPointer())
      : d_ptr->rootItem;
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
  return data->hasFormat("application/json") || column == 0;
}


bool DomainTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  }
  if (action == Qt::IgnoreAction) {
    return true;
  }

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

  DomainSettingsList newItems;
  int rows = 0;
  QByteArray encodedData = data->data("application/json");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  while (!stream.atEnd()) {
    QByteArray data;
    stream >> data;
    newItems << DomainSettings::fromJson(data);
    qDebug() << newItems.last();
    ++rows;
  }

  insertRows(beginRow, rows, QModelIndex());
  foreach (DomainSettings ds, newItems) {
    QModelIndex idx = index(beginRow, 0, QModelIndex());
    setData(idx, ds.toJson());
    ++beginRow;
  }

  return true;
}
