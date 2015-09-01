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

#include "progressdialog.h"
#include "ui_progressdialog.h"

// TODO: kick ProgressDialog, use QProgressDialog ;-)
ProgressDialog::ProgressDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ProgressDialog)
{
  ui->setupUi(this);
  QObject::connect(ui->cancelPushButton, SIGNAL(clicked(bool)), this, SIGNAL(cancelled()));
  QObject::connect(ui->closePushButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
}


ProgressDialog::~ProgressDialog()
{
  delete ui;
}


void ProgressDialog::showEvent(QShowEvent *e)
{
  QDialog::showEvent(e);
  ui->closePushButton->hide();
  ui->cancelPushButton->show();
}


void ProgressDialog::setText(QString text)
{
  ui->whatLabel->setText(text);
}


void ProgressDialog::setRange(int lo, int hi)
{
  ui->progressBar->setRange(lo, hi);
  ui->progressBar->show();
}


void ProgressDialog::setMinimum(int value)
{
  ui->progressBar->setMinimum(value);
  ui->progressBar->show();
}


void ProgressDialog::setMaximum(int value)
{
  ui->progressBar->setMaximum(value);
  ui->progressBar->show();
}


void ProgressDialog::setValue(int value)
{
  ui->progressBar->setValue(value);
  if (value >= ui->progressBar->maximum()) {
    ui->cancelPushButton->hide();
    ui->closePushButton->show();
    ui->progressBar->hide();
  }
}
