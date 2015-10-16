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
#include <QCryptographicHash>
#include <QThread>
#include <QtConcurrent>
#include <QElapsedTimer>

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
    , doAbortSpeedTest(false)
    , hashesPerSec(-1)
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
  bool doAbortSpeedTest;
  QFuture<void> speedTestFuture;
  QTimer speedTestTimer;
  QTimer speedTestAbortionTimer;
  qreal hashesPerSec;
};


EasySelectorWidget::EasySelectorWidget(QWidget *parent)
  : QWidget(parent)
  , d_ptr(new EasySelectorWidgetPrivate)
{
  Q_D(EasySelectorWidget);
  setMinimumSize(100, 100);
  setMaximumHeight(250);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setCursor(Qt::PointingHandCursor);
  QObject::connect(this, SIGNAL(speedTestFinished(qreal)), SLOT(onSpeedTestEnd(qreal)), Qt::QueuedConnection);
  QObject::connect(&d->speedTestAbortionTimer, SIGNAL(timeout()), SLOT(onSpeedTestAbort()));
  QObject::connect(&d->speedTestTimer, SIGNAL(timeout()), SLOT(onSpeedTestBegin()));
  d->speedTestTimer.setSingleShot(true);
  d->speedTestTimer.start(500);
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
  for (int y = 0; y < nY; ++y)
    for (int x = 0; x < nX; ++x)
      p.fillRect(QRect(x * xs, d->background.height() - y * ys - ys, xs, ys),
                 red2yellow2green(qLn(passwordStrength(x + d->minLength, y)) / 40).darker(168));
  p.setBrush(Qt::transparent);
  p.setPen(QPen(QBrush(QColor(0, 0, 0, 128)), 1));
  for (int x = 0; x <= nX; ++x)
    p.drawLine(xs * x, 0, xs * x, d->background.height());
  for (int y = 0; y <= nY; ++y)
    p.drawLine(0, ys * y, d->background.width(), ys * y);
}


static QString makeCrackDuration(qreal secs)
{
  QString crackDuration;
  if (secs < 1e-6) {
    crackDuration = QObject::tr("< 1 microsecond");
  }
  else if (secs < 1e-3) {
    crackDuration = QObject::tr("< 1 millisecond");
  }
  else if (secs < 1) {
    crackDuration = QObject::tr("~ %1 milliseconds").arg(qRound(1e3 * secs));
  }
  else if (secs < 60) {
    crackDuration = QObject::tr("~ %1 seconds").arg(qRound(secs));
  }
  else if (secs < 60 * 60) {
    crackDuration = QObject::tr("~ %1 minutes").arg(qRound(secs / 60));
  }
  else if (secs < 60 * 60 * 24) {
    crackDuration = QObject::tr("~ %1 hours").arg(qRound(secs / 60 / 60));
  }
  else if (secs < 60 * 60 * 24 * 365.24) {
    crackDuration = QObject::tr("~ %1 days").arg(qRound(secs / 60 / 60 / 24));
  }
  else {
    crackDuration = QObject::tr("~ %1 years").arg(secs / 60 / 60 / 24 / 365.24, 0, 'g', 2);
  }
  return crackDuration;
}


bool EasySelectorWidget::tooltipTextAt(const QPoint &pos, QString &helpText) const
{
  if (d_ptr->background.isNull())
    return false;
  const int xs = d_ptr->background.width() / (d_ptr->maxLength - d_ptr->minLength + 1);
  const int ys = d_ptr->background.height() / (d_ptr->maxComplexity + 1);
  const int length = pos.x() / xs + d_ptr->minLength;
  const int complexity = Password::MaxComplexity - pos.y() / ys;
  QString crackDuration = makeCrackDuration(tianhe2Secs(length, complexity));
  QString myCrackDuration = makeCrackDuration(mySecs(length, complexity));
  helpText = tr("%1 characters,\n"
                "est. crack time w/ Tianhe-2: %2,\n"
                "on your computer: %3")
      .arg(length)
      .arg(crackDuration)
      .arg(d_ptr->hashesPerSec < 0 ? tr("calculating ...") : myCrackDuration);
  return (d_ptr->minLength <= length) && (length <= d_ptr->maxLength);
}


qreal EasySelectorWidget::sha1Secs(int length, int complexity, qreal sha1PerSec)
{
  const QBitArray &ba = Password::deconstructedComplexity(complexity);
  int charCount = 0;
  if (ba.at(Password::TemplateDigits))
    charCount += Password::Digits.count();
  if (ba.at(Password::TemplateLowercase))
    charCount += Password::LowerChars.count();
  if (ba.at(Password::TemplateUppercase))
    charCount += Password::UpperChars.count();
  if (ba.at(Password::TemplateExtra))
    charCount += Password::ExtraChars.count();
  static const int Iterations = 1;
  const qreal perms = qPow(charCount, length);
  const qreal t2secs = perms * Iterations / sha1PerSec;
  return 0.5 * t2secs;

}


qreal EasySelectorWidget::tianhe2Secs(int length, int complexity)
{
  return sha1Secs(length, complexity, 767896613647.0);
}


qreal EasySelectorWidget::mySecs(int length, int complexity) const
{
  return sha1Secs(length, complexity, d_ptr->hashesPerSec);
}


qreal EasySelectorWidget::passwordStrength(int length, int complexity)
{
  const qreal t2y = tianhe2Secs(length, complexity) / 60 / 60 / 24 / 365.25;
  return t2y * 1e8;
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


void EasySelectorWidget::onSpeedTestBegin(void)
{
  Q_D(EasySelectorWidget);
  d->speedTestAbortionTimer.setSingleShot(true);
  d->speedTestAbortionTimer.start(3000);
  d->speedTestFuture = QtConcurrent::run(this, &EasySelectorWidget::speedTest);
}


void EasySelectorWidget::onSpeedTestEnd(qreal hashesPerSec)
{
  Q_D(EasySelectorWidget);
  d->hashesPerSec = hashesPerSec;
}


void EasySelectorWidget::onSpeedTestAbort(void)
{
  Q_D(EasySelectorWidget);
  d->doAbortSpeedTest = true;
}


void EasySelectorWidget::speedTest(void)
{
  Q_D(EasySelectorWidget);
  static const int BufSize = 512 / 8;
  QByteArray data(BufSize, static_cast<char>(0));
  QCryptographicHash hash(QCryptographicHash::Sha1);
  QElapsedTimer t;
  t.start();
  qint64 n = 0;
  while (!d->doAbortSpeedTest && t.elapsed() < 5000) {
    for (QByteArray::iterator d = data.begin(); d != data.end(); ++d)
      *d = qrand() & 0xff;
    hash.addData(data);
    hash.result();
    hash.reset();
    ++n;
  }
  int coreCount = QThread::idealThreadCount() > 0 ? QThread::idealThreadCount() : 1;
  qreal hashesPerSec = n * coreCount * 1e9 / t.nsecsElapsed();
  emit speedTestFinished(hashesPerSec);
}
