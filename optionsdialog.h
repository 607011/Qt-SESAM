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

#ifndef __OPTIONSDIALOG_H_
#define __OPTIONSDIALOG_H_

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OptionsDialog(QWidget *parent = 0);
  ~OptionsDialog();

  QString syncFilename(void) const;

private slots:
  void chooseFile(void);

private:
  Ui::OptionsDialog *ui;
};

#endif // __OPTIONSDIALOG_H_
