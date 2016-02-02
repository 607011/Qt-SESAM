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

#ifndef __MASTERPASSWORDDIALOG_H_
#define __MASTERPASSWORDDIALOG_H_

#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QEvent>
#include <QScopedPointer>

namespace Ui {
class MasterPasswordDialog;
}

class MasterPasswordDialogPrivate;

class MasterPasswordDialog : public QDialog
{
  Q_OBJECT
public:
  explicit MasterPasswordDialog(QWidget *parent = Q_NULLPTR);
  ~MasterPasswordDialog();

  void invalidatePassword(void);
  void setRepeatPassword(bool);
  bool repeatedPasswordEntry(void) const;
  QString masterPassword(void) const;

public slots:
  virtual void reject(void);

signals:
  void closing(void);

protected:
  void showEvent(QShowEvent*);
  void closeEvent(QCloseEvent*);
  void changeEvent(QEvent *);

private slots:
  void okClicked(void);
  void checkPasswords(void);

private:
  Ui::MasterPasswordDialog *ui;

  QScopedPointer<MasterPasswordDialogPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MasterPasswordDialog)
  Q_DISABLE_COPY(MasterPasswordDialog)
};

#endif // __MASTERPASSWORDDIALOG_H_
