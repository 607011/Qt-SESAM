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


#include "easyselectorwidget.h"
#include "util.h"
#include <QDebug>
#include <QSizePolicy>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPoint>

class EasySelectorWidgetPrivate {
public:
  EasySelectorWidgetPrivate(void)
    : buttonDown(false)
    , length((EasySelectorWidget::DefaultMaxLength - EasySelectorWidget::DefaultMinLength) / 2)
    , complexity((EasySelectorWidget::DefaultMaxComplexity - EasySelectorWidget::DefaultMinComplexity) / 2)
    , minLength(EasySelectorWidget::DefaultMinLength)
    , maxLength(EasySelectorWidget::DefaultMaxLength)
    , minComplexity(EasySelectorWidget::DefaultMinComplexity)
    , maxComplexity(EasySelectorWidget::DefaultMaxComplexity)
  { /* ... */ }
  ~EasySelectorWidgetPrivate()
  { /* ... */ }
  int length;
  int complexity;
  bool buttonDown;
  int minLength;
  int maxLength;
  int minComplexity;
  int maxComplexity;
};


EasySelectorWidget::EasySelectorWidget(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new EasySelectorWidgetPrivate)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setCursor(Qt::PointingHandCursor);
}


EasySelectorWidget::~EasySelectorWidget()
{
  /* ... */
}


void EasySelectorWidget::setMouseX(int x)
{
  Q_D(EasySelectorWidget);
  d->length = d->minLength + (d->maxLength - d->minLength) * clamp(x, 0, width()) / width();
  update();
}


void EasySelectorWidget::setMouseY(int y)
{
  Q_D(EasySelectorWidget);
  d->complexity = d->maxComplexity - (d->minComplexity + (d->maxComplexity - d->minComplexity) * clamp(y, 0, height()) / height());
  update();
}


int EasySelectorWidget::length(void) const
{
  return d_ptr->length;
}


int EasySelectorWidget::complexity(void) const
{
  return d_ptr->complexity;
}


void EasySelectorWidget::mouseMoveEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  if (d->buttonDown) {
    setMouseX(e->pos().x());
    setMouseY(e->pos().y());
    emit valuesChanged(d->length, d->complexity);
  }
}


void EasySelectorWidget::mousePressEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  d->buttonDown = e->button() == Qt::LeftButton;
  if (d->buttonDown) {
    setMouseX(e->pos().x());
    setMouseY(e->pos().y());
    emit valuesChanged(d->length, d->complexity);
  }
}


void EasySelectorWidget::mouseReleaseEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  if (e->button() == Qt::LeftButton)
    d->buttonDown = false;
}


void EasySelectorWidget::paintEvent(QPaintEvent *)
{
  Q_D(EasySelectorWidget);
  const int ld = d->maxLength - d->minLength;
  const int cd = d->maxComplexity - d->minComplexity;
  const qreal xo = qreal(width()) / ld;
  const qreal yo = qreal(height()) / cd;
  const qreal xs = (d->length - d->minLength) * width();
  const qreal ys = (d->complexity - d->minComplexity) * height();
  QPainter p(this);
  p.fillRect(rect(), QColor(0, 0, 100, 90));
  p.setBrush(Qt::transparent);
  p.setPen(QPen(QBrush(QColor(0, 0, 100, 190)), 1.0));
  for (int x = d->minLength; x < d->maxLength; ++x)
    p.drawLine(QLineF(xo * (x - d->minLength), 0, xo * (x - d->minLength), height()));
  for (int y = d->minComplexity; y < d->maxComplexity; ++y)
    p.drawLine(QLineF(0, yo * (y - d->minComplexity), width(), yo * (y - d->minComplexity)));
  p.setBrush(QColor(255, 0, 0, 128));
  p.setPen(Qt::transparent);
  p.drawRect(QRectF(xs / ld, height() - ys / cd, xo, yo));
}


void EasySelectorWidget::setMinLength(int minLength)
{
  Q_D(EasySelectorWidget);
  d->minLength = minLength;
  update();
}

void EasySelectorWidget::setMaxLength(int maxLength)
{
  Q_D(EasySelectorWidget);
  d->maxLength = maxLength;
  update();
}
