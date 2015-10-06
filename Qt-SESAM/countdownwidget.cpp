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


#include "countdownwidget.h"

#include <QDebug>
#include <QTimer>
#include <QSize>
#include <QImage>
#include <QPainter>

class CountdownWidgetPrivate {
public:
  CountdownWidgetPrivate(void)
  { /* ... */ }
  ~CountdownWidgetPrivate()
  { /* ... */ }
  int timeoutMs;
  QTimer masterPasswordInvalidationTimer;
  QTimer countdown;
};


const int CountdownWidget::UpdateIntervalMs = 5 * 1000;
const QSize CountdownWidget::DefaultSize = QSize(16, 16);

CountdownWidget::CountdownWidget(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new CountdownWidgetPrivate)
{
  Q_D(CountdownWidget);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resize(DefaultSize);
  QObject::connect(&d->countdown, SIGNAL(timeout()), SLOT(tick()));
  QObject::connect(&d->masterPasswordInvalidationTimer, SIGNAL(timeout()), SIGNAL(timeout()));
}


CountdownWidget::~CountdownWidget()
{ /* ... */ }


void CountdownWidget::start(int timeoutMs)
{
  Q_D(CountdownWidget);
  d->countdown.setInterval(UpdateIntervalMs);
  d->countdown.start();
  d->timeoutMs = timeoutMs;
  d->masterPasswordInvalidationTimer.setSingleShot(true);
  d->masterPasswordInvalidationTimer.setTimerType(Qt::VeryCoarseTimer);
  d->masterPasswordInvalidationTimer.start(timeoutMs);
  tick();
}


void CountdownWidget::stop(void)
{
  Q_D(CountdownWidget);
  d->countdown.stop();
  d->masterPasswordInvalidationTimer.stop();
  update();
}


int CountdownWidget::remainingTime(void) const
{
  return d_ptr->masterPasswordInvalidationTimer.remainingTime();
}


void CountdownWidget::tick(void)
{
  const int secs = remainingTime() / 1000;
  const QString &t = (secs <= 60)
      ? tr("%1 seconds").arg(secs)
      : tr("<%1 minutes").arg(1 + secs / 60);
  setToolTip(tr("Application will be locked in %1.").arg(t));
  update();
}


void CountdownWidget::paintEvent(QPaintEvent *)
{
  Q_D(CountdownWidget);
  QImage image(size(), QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  QPainter p(&image);
  p.setOpacity(0.6);
  p.setRenderHint(QPainter::Antialiasing);
  p.setCompositionMode(QPainter::CompositionMode_Source);
  p.setBrush(Qt::transparent);
  static const QRect boundingRect(2, 2, 12, 12);
  static const int Twelve = 16 * 90;
  p.setPen(QPen(QBrush(remainingTime() < 60 * 1000 ? Qt::red : Qt::black), 1.0));
  const int angle = 16 * 360 * remainingTime() / d->timeoutMs;
  if (angle > 16 * (360 - 10))
    p.drawEllipse(boundingRect);
  else
    p.drawPie(boundingRect, Twelve, -angle);
  p.end();
  QPainter tp(this);
  tp.drawImage(0, 0, image);
}


