#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ProgressDialog)
{
  ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
  delete ui;
}


void ProgressDialog::setText(QString text)
{
  ui->whatLabel->setText(text);
}


void ProgressDialog::setRange(int _min, int _max)
{
  ui->progressBar->setRange(_min, _max);

}


void ProgressDialog::setMinimum(int value)
{
  ui->progressBar->setMinimum(value);

}


void ProgressDialog::setMaximum(int value)
{
  ui->progressBar->setMaximum(value);
}


void ProgressDialog::setValue(int value)
{
  ui->progressBar->setValue(value);
}
