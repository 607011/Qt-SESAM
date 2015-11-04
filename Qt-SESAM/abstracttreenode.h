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


#ifndef __ABSTRACTTREENODE_H_
#define __ABSTRACTTREENODE_H_

#include <QList>
#include <QVariant>
#include "domainsettings.h"


class AbstractTreeNode
{
public:
  explicit AbstractTreeNode(AbstractTreeNode *parentItem = Q_NULLPTR);
  ~AbstractTreeNode();

  void appendChild(AbstractTreeNode *child);
  AbstractTreeNode *child(int row);
  int childCount(void) const;
  int columnCount(void) const;
  virtual QVariant data(int column) const;
  int row(void) const;
  AbstractTreeNode *parentItem(void);

  enum NodeType {
    GroupType,
    LeafType
  };

  virtual NodeType type(void) const = 0;

protected:
  AbstractTreeNode *mParentItem;
  QList<AbstractTreeNode*> mChildItems;

private:
};
#endif // __ABSTRACTTREENODE_H_
