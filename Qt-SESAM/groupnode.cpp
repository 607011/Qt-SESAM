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

#include <QDebug>
#include "groupnode.h"
#include "abstracttreenode.h"

GroupNode::GroupNode(const QString &name, AbstractTreeNode *parentItem)
  : AbstractTreeNode(parentItem)
  , mName(name)
{
  /* ... */
}


GroupNode::~GroupNode()
{
  /* ... */
}


const QString &GroupNode::name(void) const
{
  return mName;
}


QVariant GroupNode::data(int column) const
{
  switch (column)
  {
  case 0:
    return name();
  default:
    break;
  }
  return QVariant();
}


AbstractTreeNode::NodeType GroupNode::type(void) const
{
  return GroupType;
}
