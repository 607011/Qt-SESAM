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

#ifndef __TREEMODEL_H_
#define __TREEMODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QStringList>
#include "domainsettingslist.h"

class AbstractTreeNode;
class GroupNode;

class TreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit TreeModel(QObject *parent = Q_NULLPTR);
  ~TreeModel();

  void setData(const DomainSettingsList &);
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  AbstractTreeNode *node(const QModelIndex &index);

private:
  static GroupNode *findChild(const QString &name, GroupNode *node);
  static GroupNode *addToHierarchy(const QStringList &groups, GroupNode *node);

private: // member variables
  GroupNode *mRootItem;
};

#endif // __TREEMODEL_H_
