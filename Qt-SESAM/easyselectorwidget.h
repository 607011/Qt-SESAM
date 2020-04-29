/*

    Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

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


#ifndef __EASYSELECTORWIDGET_H_
#define __EASYSELECTORWIDGET_H_

#include <QWidget>
#include <QScreen>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QString>
#include <QPoint>
#include <QScopedPointer>

class EasySelectorWidgetPrivate;

class EasySelectorWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EasySelectorWidget(QWidget *parent = Q_NULLPTR);
  ~EasySelectorWidget();

  void setMousePos(const QPoint &);
  void setLength(int length);
  void setComplexityValue(int complexityValue);
  int length(void) const;
  int complexityValue(void) const;
  void setExtraCharacters(const QString &extraChars);

protected:
  void mouseMoveEvent(QMouseEvent*) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent*) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent*) Q_DECL_OVERRIDE;
  void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent*) Q_DECL_OVERRIDE;
  bool event(QEvent*) Q_DECL_OVERRIDE;
  QSize minimumSizeHint(void) const Q_DECL_OVERRIDE;

signals:
  void valuesChanged(int newLength, int newComplexity);
  void speedTestFinished(qreal);

public slots:
  void setMinLength(int);
  void setMaxLength(int);
  void onSpeedTestBegin(void);
  void onSpeedTestEnd(qreal hashesPerSec);
  void onSpeedTestAbort(void);
  void onScreenChanged(QScreen *screen);

private:
  QScopedPointer<EasySelectorWidgetPrivate> d_ptr;
  Q_DECLARE_PRIVATE(EasySelectorWidget)
  Q_DISABLE_COPY(EasySelectorWidget)

  static const int DefaultMinLength;
  static const int DefaultMaxLength;

private: // methods
  void speedTest(void);
  void redrawBackground(void);
  bool tooltipTextAt(const QPoint &pos, QString &helpText) const;
  qreal tianhe2Secs(int length, int complexityValue) const;
  qreal passwordStrength(int length, int complexityValue) const;
  qreal sha1Secs(int length, int complexityValue, qreal sha1PerSec) const;
  qreal mySecs(int length, int complexityValue) const;
};

#endif // __EASYSELECTORWIDGET_H_
