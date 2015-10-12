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
#include <QDebug>
#include <QGroupBox>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

class ExpandableGroupboxPrivate {
public:
  ExpandableGroupboxPrivate(QWidget *parent)
    : layout(new QVBoxLayout(parent))
    , contentsFrame(new QFrame(parent))
    , titleLabel(new QLabel(parent))
    , buttonLabel(new QLabel(parent))
    , expanded(true)
  { /* ... */ }
  ~ExpandableGroupboxPrivate(void)
  { /* ... */ }
  QVBoxLayout *layout;
  QFrame *contentsFrame;
  QLabel *titleLabel;
  QLabel *buttonLabel;
  bool expanded;
};


static const QString ExpandCollapseButton = "expandCollapseButton";
static const QString TitleLabel = "titleLabel";


ExpandableGroupbox::ExpandableGroupbox(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new ExpandableGroupboxPrivate(this))
{
  Q_D(ExpandableGroupbox);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  setObjectName("ExpandableGroupbox");
  d->layout->setContentsMargins(0, 20, 0, 0);
  d->layout->setSizeConstraint(QLayout::SetNoConstraint);
  QWidget::setLayout(d->layout);
  d->contentsFrame->setFrameShape(QFrame::StyledPanel);
  d->titleLabel->setCursor(Qt::PointingHandCursor);
  d->titleLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  d->titleLabel->setObjectName(TitleLabel);
  d->titleLabel->installEventFilter(this);
  d->titleLabel->move(12, 4);
  d->buttonLabel->setCursor(Qt::PointingHandCursor);
  d->buttonLabel->setMaximumSize(11, 11);
  d->buttonLabel->setObjectName(ExpandCollapseButton);
  d->buttonLabel->installEventFilter(this);
  d->buttonLabel->move(0, 6);
  d->layout->addWidget(d->contentsFrame);
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
  d->buttonLabel->setPixmap(ExpandedPixmap);
  d->contentsFrame->setMaximumHeight(QWIDGETSIZE_MAX);
  d->contentsFrame->adjustSize();
  emit expansionStateChanged();
}


void ExpandableGroupbox::collapse(void)
{
  Q_D(ExpandableGroupbox);
  d->expanded = false;
  static const QPixmap CollapsedPixmap(":/images/collapsed.png");
  d->buttonLabel->setPixmap(CollapsedPixmap);
  d->contentsFrame->setMaximumHeight(0);
  d->contentsFrame->adjustSize();
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
  d->titleLabel->setText(title);
  d->titleLabel->adjustSize();
}


void ExpandableGroupbox::setLayout(QLayout *layout)
{
  Q_D(ExpandableGroupbox);
  d->contentsFrame->setLayout(layout);
}


bool ExpandableGroupbox::eventFilter(QObject *obj, QEvent *event)
{
  switch (event->type()) {
  case QEvent::MouseButtonPress:
    if (obj->objectName() == ExpandCollapseButton || obj->objectName() == TitleLabel) {
      toggle();
    }
    break;
  default:
    break;
  }
  return QObject::eventFilter(obj, event);
}
