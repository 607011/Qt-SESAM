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

#ifndef __PROGRESSDIALOG_H_
#define __PROGRESSDIALOG_H_

#include <QDialog>
#include <QString>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ProgressDialog(QWidget *parent = Q_NULLPTR);
  ~ProgressDialog();

protected:
  void showEvent(QShowEvent *);

public slots:
  void setText(QString);
  void setRange(int, int);
  void setMinimum(int);
  void setMaximum(int);
  void setValue(int);

signals:
  void cancelled(void);

private:
  Ui::ProgressDialog *ui;
};

#endif // __PROGRESSDIALOG_H_
