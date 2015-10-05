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
#include <QLinearGradient>
#include <QGradientStop>
#include <QPoint>
#include <QPixmap>

class EasySelectorWidgetPrivate {
public:
  EasySelectorWidgetPrivate(void)
    : buttonDown(false)
    , length((EasySelectorWidget::DefaultMaxLength - EasySelectorWidget::DefaultMinLength) / 2)
    , complexity((EasySelectorWidget::DefaultMaxComplexity - EasySelectorWidget::DefaultMinComplexity) / 2)
    , oldLength(length)
    , oldComplexity(complexity)
    , minLength(EasySelectorWidget::DefaultMinLength)
    , maxLength(EasySelectorWidget::DefaultMaxLength)
    , minComplexity(EasySelectorWidget::DefaultMinComplexity)
    , maxComplexity(EasySelectorWidget::DefaultMaxComplexity)
  { /* ... */ }
  ~EasySelectorWidgetPrivate()
  { /* ... */ }
  bool buttonDown;
  int length;
  int complexity;
  int oldLength;
  int oldComplexity;
  int minLength;
  int maxLength;
  int minComplexity;
  int maxComplexity;
  QPixmap background;
};


EasySelectorWidget::EasySelectorWidget(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new EasySelectorWidgetPrivate)
{
  setMinimumSize(100, 100);
  setMaximumHeight(250);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setCursor(Qt::PointingHandCursor);
}


EasySelectorWidget::~EasySelectorWidget()
{
  /* ... */
}


void EasySelectorWidget::setMousePos(const QPoint &pos)
{
  Q_D(EasySelectorWidget);
  if (d->background.isNull())
    return;
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity - d->minComplexity + 1;
  const int xs = d->background.width() / nX;
  const int ys = d->background.height() / nY;
  const int clampedX = clamp(pos.x(), 0, d->background.width() - xs);
  const int clampedY = clamp(pos.y(), 0, d->background.height() - ys);
  d->length = d->minLength + clampedX / xs;
  d->complexity = d->maxComplexity - (d->minComplexity + clampedY / ys);
  update();
}


void EasySelectorWidget::setLength(int length)
{
  Q_D(EasySelectorWidget);
  d->length = length;
  update();
}


int EasySelectorWidget::length(void) const
{
  return d_ptr->length;
}


void EasySelectorWidget::setComplexity(int complexity)
{
  Q_D(EasySelectorWidget);
  d->complexity = complexity;
  update();
}


int EasySelectorWidget::complexity(void) const
{
  return d_ptr->complexity;
}


void EasySelectorWidget::mouseMoveEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  if (d->buttonDown) {
    setMousePos(e->pos());
    if (d->length != d->oldLength || d->complexity != d->oldComplexity)
      emit valuesChanged(d->length, d->complexity);
  }
}


void EasySelectorWidget::mousePressEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  d->buttonDown = e->button() == Qt::LeftButton;
  if (d->buttonDown) {
    setMousePos(e->pos());
    if (d->length != d->oldLength || d->complexity != d->oldComplexity)
      emit valuesChanged(d->length, d->complexity, d->oldLength, d->oldComplexity);
    d->oldLength = d->length;
    d->oldComplexity = d->complexity;
  }
}


void EasySelectorWidget::mouseReleaseEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  if (e->button() == Qt::LeftButton) {
    d->buttonDown = false;
    emit valuesChanged(d->length, d->complexity, d->oldLength, d->oldComplexity);
  }
}


void EasySelectorWidget::paintEvent(QPaintEvent *)
{
  Q_D(EasySelectorWidget);
  if (d->background.isNull())
    return;
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity - d->minComplexity + 1;
  const int xs = d->background.width() / nX;
  const int ys = d->background.height() / nY;
  QPainter p(this);
  p.drawPixmap(QPoint(0, 0), d->background);
  p.setBrush(QColor(255, 255, 255, 192));
  p.setPen(Qt::transparent);
  p.drawRect(QRect(QPoint(xs * (d->length - d->minLength),
                          d->background.height() - ys * (d->complexity - d->minComplexity + 1)),
                   QSize(xs, ys)));
}


void EasySelectorWidget::resizeEvent(QResizeEvent *e)
{
  Q_D(EasySelectorWidget);
  QSize size = e->size();
  if (size.width() == 0 || size.height() == 0) {
    d->background = QPixmap();
    return;
  }
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity - d->minComplexity + 1;
  const int xs = size.width() / nX;
  const int ys = size.height() / nY;
  size = QSize(xs * nX, ys * nY);
  d->background = QPixmap(size);
  QPainter p(&d->background);
  QLinearGradient gradient(0, size.height(), size.width(), 0);
  gradient.setColorAt(0.0, QColor(255, 0, 0, 255).darker());
  gradient.setColorAt(0.5, QColor(255, 255, 0, 255).darker());
  gradient.setColorAt(1.0, QColor(0, 255, 0, 255).darker());
  p.fillRect(0, 0, size.width(), size.height(), gradient);
  p.setBrush(Qt::transparent);
  p.setPen(QPen(QBrush(QColor(255, 255, 255, 160)), 1));
  for (int x = 0; x < nX; ++x)
    p.drawLine(xs * x, 0, xs * x, d->background.height());
  for (int y = 0; y < nY; ++y)
    p.drawLine(0, ys * y, d->background.width(), ys * y);
  setSizeIncrement(xs, ys);
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
