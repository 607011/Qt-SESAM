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


#ifndef __EASYSELECTORWIDGET_H_
#define __EASYSELECTORWIDGET_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScopedPointer>

class EasySelectorWidgetPrivate;

class EasySelectorWidget : public QWidget
{
  Q_OBJECT
public:
  explicit EasySelectorWidget(QWidget *parent = nullptr);
  ~EasySelectorWidget();
  QSize minimumSizeHint(void ) { return QSize(180, 180); }
  QSize sizeHint(void) { return QSize(320, 240); }

  void setMouseX(int);
  void setMouseY(int);

  int length(void) const;
  int complexity(void) const;

protected:
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

signals:
  void valuesChanged(int length, int complexity);

public slots:
  void setMinLength(int);
  void setMaxLength(int);

private:
  QScopedPointer<EasySelectorWidgetPrivate> d_ptr;
  Q_DECLARE_PRIVATE(EasySelectorWidget)
  Q_DISABLE_COPY(EasySelectorWidget)

  static const int DefaultMinLength = 4;
  static const int DefaultMaxLength = 40;
  static const int DefaultMinComplexity = 0;
  static const int DefaultMaxComplexity = 7;

private: // methods
};

#endif // __EASYSELECTORWIDGET_H_
