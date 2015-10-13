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


#ifndef __CHANGEMASTERPASSWORDDIALOG_H_
#define __CHANGEMASTERPASSWORDDIALOG_H_

#include <QWidget>
#include <QDialog>
#include <QShowEvent>
#include <QScopedPointer>
#include <QString>


namespace Ui {
class ChangeMasterPasswordDialog;
}

class ChangeMasterPasswordDialogPrivate;

class ChangeMasterPasswordDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ChangeMasterPasswordDialog(QWidget *parent = Q_NULLPTR);
  ~ChangeMasterPasswordDialog();
  void invalidate(void);
  QString oldPassword(void) const;
  QString newPassword(void) const;
  void setPasswordFilename(const QString &filename = QString());

private slots:
  void okClicked(void);
  void comparePasswords(void);

protected:
  void showEvent(QShowEvent *);


private:
  Ui::ChangeMasterPasswordDialog *ui;
  QScopedPointer<ChangeMasterPasswordDialogPrivate> d_ptr;
  Q_DECLARE_PRIVATE(ChangeMasterPasswordDialog)
  Q_DISABLE_COPY(ChangeMasterPasswordDialog)
};

#endif // __CHANGEMASTERPASSWORDDIALOG_H_
