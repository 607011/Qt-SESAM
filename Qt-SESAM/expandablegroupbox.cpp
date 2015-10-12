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
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

class ExpandableGroupboxPrivate {
public:
  ExpandableGroupboxPrivate(QWidget *parent)
    : layout(new QVBoxLayout(parent))
    , contents(new QFrame(parent))
    , title(new QLabel(parent))
    , button(new QLabel(parent))
    , expanded(true)
  { /* ... */ }
  ~ExpandableGroupboxPrivate(void)
  { /* ... */ }
  QVBoxLayout *layout;
  QFrame *contents;
  QLabel *title;
  QLabel *button;
  bool expanded;
};

static const QString ExpandCollapseButton = "expandCollapseButton";


ExpandableGroupbox::ExpandableGroupbox(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new ExpandableGroupboxPrivate(this))
{
  Q_D(ExpandableGroupbox);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  setObjectName("ExpandableGroupbox");
  d->layout->setContentsMargins(0, 20, 0, 0);
  QWidget::setLayout(d->layout);
  d->contents = new QFrame(this);
  d->contents->setFrameShape(QFrame::StyledPanel);
  d->title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  d->title->move(12, -5);
  d->button->setCursor(Qt::PointingHandCursor);
  d->button->setObjectName(ExpandCollapseButton);
  d->button->installEventFilter(this);
  d->button->move(0, -5);
  d->layout->addWidget(d->contents);
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
  d->contents->setMaximumHeight(QWIDGETSIZE_MAX);
  d->contents->adjustSize();
  emit expansionStateChanged();
}


void ExpandableGroupbox::collapse(void)
{
  Q_D(ExpandableGroupbox);
  d->expanded = false;
  static const QPixmap CollapsedPixmap(":/images/collapsed.png");
  d->button->setPixmap(CollapsedPixmap);
  d->contents->setMaximumHeight(0);
  d->contents->adjustSize();
  adjustSize();
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
  d->contents->setLayout(layout);
}


bool ExpandableGroupbox::eventFilter(QObject *obj, QEvent *event)
{
  switch (event->type()) {
  case QEvent::MouseButtonPress:
    if (obj->objectName() == ExpandCollapseButton) {
      toggle();
    }
    break;
  default:
    break;
  }
  return QObject::eventFilter(obj, event);
}
