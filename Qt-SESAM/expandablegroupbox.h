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


#ifndef __EXPANDABLEGROUPBOX_H_
#define __EXPANDABLEGROUPBOX_H_

#include <QObject>
#include <QEvent>
#include <QString>
#include <QWidget>
#include <QLayout>
#include <QScopedPointer>

class ExpandableGroupboxPrivate;

class ExpandableGroupbox : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool expanded READ expanded WRITE setExpanded)

public:
  ExpandableGroupbox(QWidget *parent = Q_NULLPTR);
  ~ExpandableGroupbox();
  void expand(void);
  void collapse(void);
  void toggle(void);
  bool expanded(void) const;
  void setExpanded(bool doExpand = true);
  void setTitle(const QString &title);
  virtual void setLayout(QLayout*);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

signals:
  void expansionStateChanged(void);

public slots:

private:
  QScopedPointer<ExpandableGroupboxPrivate> d_ptr;
  Q_DECLARE_PRIVATE(ExpandableGroupbox)
  Q_DISABLE_COPY(ExpandableGroupbox)
};

#endif // __EXPANDABLEGROUPBOX_H_
