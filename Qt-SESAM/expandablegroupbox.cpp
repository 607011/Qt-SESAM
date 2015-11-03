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


#include "expandablegroupbox.h"
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

class ExpandableGroupboxPrivate {
public:
  ExpandableGroupboxPrivate(void)
    : layout(Q_NULLPTR)
    , groupBox(Q_NULLPTR)
    , title(Q_NULLPTR)
    , button(Q_NULLPTR)
    , expanded(true)
  { /* ... */ }
  ~ExpandableGroupboxPrivate(void)
  { /* ... */ }
  QVBoxLayout *layout;
  QGroupBox *groupBox;
  QLabel *button;
  QLabel *title;
  bool expanded;
};



ExpandableGroupbox::ExpandableGroupbox(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new ExpandableGroupboxPrivate)
{
  Q_D(ExpandableGroupbox);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setObjectName("ExpandableGroupbox");
  d->layout = new QVBoxLayout(this);
  d->layout->setContentsMargins(0, 17, 0, 0);
  QWidget::setLayout(d->layout);
  d->groupBox = new QGroupBox(this);
  d->title = new QLabel(this);
  d->title->move(13, -4);
  d->button = new QLabel(this);
  d->button->setCursor(Qt::PointingHandCursor);
  d->button->setObjectName("button");
  d->button->move(2, -4);
  d->button->installEventFilter(this);
  d->layout->addWidget(d->groupBox);
  d->layout->addStretch(0);
  collapse();
}


ExpandableGroupbox::~ExpandableGroupbox()
{
  /* ... */
}


void ExpandableGroupbox::expand(void)
{
  Q_D(ExpandableGroupbox);
  d->expanded = true;
  static const QPixmap ExpandedPixmap(":/images/expanded.png");
  d->button->setPixmap(ExpandedPixmap);
  d->groupBox->setMaximumHeight(QWIDGETSIZE_MAX);
  d->groupBox->adjustSize();
  emit expansionStateChanged();
}


void ExpandableGroupbox::collapse(void)
{
  Q_D(ExpandableGroupbox);
  d->expanded = false;
  static const QPixmap CollapsedPixmap(":/images/collapsed.png");
  d->button->setPixmap(CollapsedPixmap);
  d->groupBox->setMaximumHeight(11);
  d->groupBox->adjustSize();
  emit expansionStateChanged();
}


void ExpandableGroupbox::toggle(void)
{
  Q_D(ExpandableGroupbox);
  if (d->expanded)
    collapse();
  else
    expand();
}


bool ExpandableGroupbox::expanded(void) const
{
  return d_ptr->expanded;
}


void ExpandableGroupbox::setExpanded(bool doExpand)
{
  if (doExpand)
    expand();
  else
    collapse();
}


void ExpandableGroupbox::setTitle(const QString &title)
{
  Q_D(ExpandableGroupbox);
  d->title->setText(title);
}


void ExpandableGroupbox::setLayout(QLayout *layout)
{
  Q_D(ExpandableGroupbox);
  d->groupBox->setLayout(layout);
}


bool ExpandableGroupbox::eventFilter(QObject *obj, QEvent *event)
{
  switch (event->type()) {
  case QEvent::MouseButtonPress:
    if (obj->objectName() == "button") {
      toggle();
    }
    break;
  default:
    break;
  }
  return QObject::eventFilter(obj, event);
}
