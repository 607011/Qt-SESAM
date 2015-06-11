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

#ifndef __NEWCREDENTIALSDIALOG_H_
#define __NEWCREDENTIALSDIALOG_H_

#include <QWidget>

namespace Ui {
class NewCredentialsDialog;
}

class NewCredentialsDialog : public QWidget
{
  Q_OBJECT

public:
  explicit NewCredentialsDialog(QWidget *parent = nullptr);
  ~NewCredentialsDialog();

private:
  Ui::NewCredentialsDialog *ui;
};

#endif // __NEWCREDENTIALSDIALOG_H_
