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
#include "password.h"
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

const int EasySelectorWidget::DefaultMinLength = 4;
const int EasySelectorWidget::DefaultMaxLength = 36;
const int EasySelectorWidget::DefaultMaxComplexity = 6;

class EasySelectorWidgetPrivate {
public:
  EasySelectorWidgetPrivate(void)
    : buttonDown(false)
    , length((EasySelectorWidget::DefaultMaxLength - EasySelectorWidget::DefaultMinLength) / 2)
    , complexity(EasySelectorWidget::DefaultMaxComplexity / 2)
    , oldLength(length)
    , oldComplexity(complexity)
    , minLength(EasySelectorWidget::DefaultMinLength)
    , maxLength(EasySelectorWidget::DefaultMaxLength)
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
  const int nY = d->maxComplexity + 1;
  const int xs = d->background.width() / nX;
  const int ys = d->background.height() / nY;
  const int clampedX = clamp(pos.x(), 0, d->background.width() - xs);
  const int clampedY = clamp(pos.y(), 0, d->background.height() - ys);
  d->length = d->minLength + clampedX / xs;
  d->complexity = d->maxComplexity - clampedY / ys;
  update();
}


void EasySelectorWidget::setLength(int length)
{
  Q_D(EasySelectorWidget);
  d->length = length;
  if (d->length != d->oldLength) {
    emit valuesChanged(d->length, d->complexity);
    d->oldLength = d->length;
  }
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
    if (d->length != d->oldLength || d->complexity != d->oldComplexity) {
      emit valuesChanged(d->length, d->complexity);
      d->oldLength = d->length;
      d->oldComplexity = d->complexity;
    }
  }
}


void EasySelectorWidget::mousePressEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  d->buttonDown = e->button() == Qt::LeftButton;
  if (d->buttonDown) {
    setMousePos(e->pos());
    if (d->length != d->oldLength || d->complexity != d->oldComplexity) {
      emit valuesChanged(d->length, d->complexity);
      d->oldLength = d->length;
      d->oldComplexity = d->complexity;
    }
  }
}


void EasySelectorWidget::mouseReleaseEvent(QMouseEvent *e)
{
  Q_D(EasySelectorWidget);
  if (e->button() == Qt::LeftButton) {
    d->buttonDown = false;
    if (d->length != d->oldLength || d->complexity != d->oldComplexity) {
      emit valuesChanged(d->length, d->complexity);
      d->oldLength = d->length;
      d->oldComplexity = d->complexity;
    }
  }
}


void EasySelectorWidget::paintEvent(QPaintEvent *)
{
  Q_D(EasySelectorWidget);
  if (d->background.isNull())
    return;
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity + 1;
  const int xs = d->background.width() / nX;
  const int ys = d->background.height() / nY;
  QPainter p(this);
  p.drawPixmap(QPoint(0, 0), d->background);
  p.setBrush(QColor(255, 255, 255, 208));
  p.setPen(Qt::transparent);
  p.drawRect(QRect(QPoint(xs * (d->length - d->minLength) + 1,
                          d->background.height() - ys * (d->complexity + 1)),
                   QSize(xs - 1, ys - 1)));
}


void EasySelectorWidget::resizeEvent(QResizeEvent *)
{
  Q_D(EasySelectorWidget);
  redrawBackground();
}


void EasySelectorWidget::redrawBackground(void)
{
  Q_D(EasySelectorWidget);
  if (width() == 0 || height() == 0) {
    d->background = QPixmap();
    return;
  }
  QSize sz = size();
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity + 1;
  const int xs = sz.width() / nX;
  const int ys = sz.height() / nY;
  sz = QSize(xs * nX + 1, ys * nY + 1);
  d->background = QPixmap(sz);
  QPainter p(&d->background);
  QLinearGradient gradient(0, sz.height(), sz.width(), 0);
  gradient.setColorAt(0.0, QColor(255, 0, 0, 255).darker());
  const qreal stretch = qreal(Password::DefaultMaxLength) / d->maxLength;
  const qreal y = clamp(0.5 * stretch, 0.0, 0.999);
  const qreal g = clamp(1.0 * stretch, 0.0, 1.000);
  gradient.setColorAt(y, QColor(255, 255, 0, 255).darker());
  gradient.setColorAt(g, QColor(0, 255, 0, 255).darker());
  p.fillRect(QRect(QPoint(0, 0), sz), gradient);
  p.setBrush(Qt::transparent);
  p.setPen(QPen(QBrush(QColor(0, 0, 0, 128)), 1));
  for (int x = 0; x <= nX; ++x)
    p.drawLine(xs * x, 0, xs * x, d->background.height());
  for (int y = 0; y <= nY; ++y)
    p.drawLine(0, ys * y, d->background.width(), ys * y);
}


void EasySelectorWidget::setMinLength(int minLength)
{
  Q_D(EasySelectorWidget);
  d->minLength = minLength;
  redrawBackground();
  update();
}

void EasySelectorWidget::setMaxLength(int maxLength)
{
  Q_D(EasySelectorWidget);
  d->maxLength = maxLength;
  redrawBackground();
  update();
}
