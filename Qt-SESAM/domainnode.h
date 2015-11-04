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

#ifndef __DOMAINNODE_H_
#define __DOMAINNODE_H_

#include "abstracttreenode.h"
#include "domainsettings.h"


class DomainNode : public AbstractTreeNode
{
public:
  explicit DomainNode(AbstractTreeNode *parentItem = Q_NULLPTR);
  DomainNode(const DomainSettings &data, AbstractTreeNode *parentItem = Q_NULLPTR);
  virtual ~DomainNode();

  virtual QVariant data(int column) const;

  virtual NodeType type(void) const { return LeafType; }

private:
  DomainSettings mItemData;
};

#endif // __DOMAINNODE_H_
