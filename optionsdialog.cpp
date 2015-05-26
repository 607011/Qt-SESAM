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

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include <QFileDialog>

OptionsDialog::OptionsDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::OptionsDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->okPushButton, SIGNAL(pressed()), SLOT(accept()));
  QObject::connect(ui->cancelPushButton, SIGNAL(pressed()), SLOT(reject()));
  QObject::connect(ui->chooseSyncFilePushButton, SIGNAL(pressed()), SLOT(chooseFile()));
}


OptionsDialog::~OptionsDialog()
{
  delete ui;
}


QString OptionsDialog::syncFilename(void) const
{
  return ui->syncFileLineEdit->text();
}


void OptionsDialog::chooseFile(void)
{
  QString lastDir;
  QString chosenFile = QFileDialog::getOpenFileName(this, tr("Choose sync file"), lastDir);
  ui->syncFileLineEdit->setText(chosenFile);
}
