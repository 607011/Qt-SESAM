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

#ifndef __COUNTDOWNWIDGET_H_
#define __COUNTDOWNWIDGET_H_

#include <QWidget>
#include <QScopedPointer>
#include <QPaintEvent>

class CountdownWidgetPrivate;

class CountdownWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CountdownWidget(QWidget *parent = Q_NULLPTR);
  ~CountdownWidget();
  QSize sizeHint(void) const { return DefaultSize; }
  QSize minimumSizeHint(void) const { return DefaultSize; }

  void stop(void);

  int remainingTime(void) const;

public slots:
  void start(int timeout);

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

signals:
  void timeout(void);

private slots:
  void tick(void);

private: // methods
  void redrawImage(const QSize &);

private:
  QScopedPointer<CountdownWidgetPrivate> d_ptr;
  Q_DECLARE_PRIVATE(CountdownWidget)
  Q_DISABLE_COPY(CountdownWidget)

  static const int UpdateIntervalMs;
  static const QSize DefaultSize;
};

#endif // __COUNTDOWNWIDGET_H_
