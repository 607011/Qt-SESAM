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
#include <QToolTip>

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
  if (d->length > d->maxLength)
    setMaxLength(d->length);
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
  redrawBackground();
}


bool EasySelectorWidget::event(QEvent *e)
{
  Q_D(EasySelectorWidget);
  if (e->type() == QEvent::ToolTip) {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
    QString helpText;
    if (tooltipTextAt(helpEvent->pos(), helpText))
      QToolTip::showText(helpEvent->globalPos(), helpText);
    else
      QToolTip::hideText();
    return true;
  }
  return QWidget::event(e);;
}


static QColor red2yellow2green(qreal x)
{
  QColor color;
  x = clamp(x, 0.0, 1.0);
  if (x <= 0.5) {
    x *= 2;
    color.setRed(255);
    color.setGreen(255 * x);
  }
  else {
    x = 2 * x - 1;
    color.setRed(255 - 255 * x);
    color.setGreen(255);
  }
  return color;
}


void EasySelectorWidget::redrawBackground(void)
{
  Q_D(EasySelectorWidget);
  if (width() == 0 || height() == 0) {
    d->background = QPixmap();
    return;
  }
  const int nX = d->maxLength - d->minLength + 1;
  const int nY = d->maxComplexity + 1;
  const int xs = width() / nX;
  const int ys = height() / nY;
  d->background = QPixmap(QSize(xs * nX + 1, ys * nY + 1));
  QPainter p(&d->background);
  p.fillRect(QRect(QPoint(0, 0), d->background.size()), Qt::white);
  for (int y = 0; y < nY; ++y) {
    for (int x = 0; x < nX; ++x) {
      qreal strength = passwordStrength(x, y);
      p.fillRect(QRect(x * xs, d->background.height() - y * ys - ys, xs, ys), red2yellow2green(strength).darker(165));
    }
  }
  p.setBrush(Qt::transparent);
  p.setPen(QPen(QBrush(QColor(0, 0, 0, 128)), 1));
  for (int x = 0; x <= nX; ++x)
    p.drawLine(xs * x, 0, xs * x, d->background.height());
  for (int y = 0; y <= nY; ++y)
    p.drawLine(0, ys * y, d->background.width(), ys * y);
}


bool EasySelectorWidget::tooltipTextAt(const QPoint &pos, QString &helpText) const
{
  if (d_ptr->background.isNull())
    return false;
  const int xs = d_ptr->background.width() / (d_ptr->maxLength - d_ptr->minLength + 1);
  const int length = pos.x() / xs + d_ptr->minLength;
  helpText = tr("%1 characters").arg(length);
  return (d_ptr->minLength <= length) && (length <= d_ptr->maxLength);
}


qreal EasySelectorWidget::passwordStrength(int length, int complexity) const
{
  const QBitArray &ba = Password::deconstructedComplexity(complexity);
  int n = 0;
  if (ba.at(Password::TemplateDigits))
    n += Password::Digits.count();
  if (ba.at(Password::TemplateLowercase))
    n += Password::LowerChars.count();
  if (ba.at(Password::TemplateUppercase))
    n += Password::UpperChars.count();
  if (ba.at(Password::TemplateExtra))
    n += Password::ExtraChars.count();
  static const qreal MaxN =
      Password::Digits.count() + Password::LowerChars.count() +
      Password::UpperChars.count() + Password::ExtraChars.count();
  // static const qreal Tianhe2Flops = 33.86e15;
  static const qreal MaxStrength = qPow(MaxN, Password::DefaultLength);
  return qLn(qPow(n, length + d_ptr->minLength) / MaxStrength * 1e5);
}


void EasySelectorWidget::setMinLength(int minLength)
{
  Q_D(EasySelectorWidget);
  d->minLength = minLength;
  if (d->maxLength < d->minLength) {
    setMaxLength(d->minLength);
  }
  else {
    redrawBackground();
    update();
  }
}

void EasySelectorWidget::setMaxLength(int maxLength)
{
  Q_D(EasySelectorWidget);
  d->maxLength = maxLength;
  redrawBackground();
  update();
}
