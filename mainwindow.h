/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>

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

#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>
#include <QString>
#include <QFuture>
#include <QMovie>
#include <QRegExpValidator>
#include <QCloseEvent>
#include <QSettings>
#include <QCompleter>
#include <QMutex>

#include "domainsettings.h"
#include "passwordgenerator.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *);

private slots:
  void updatePassword(void);
  void updateUsedCharacters(void);
  void copyPasswordToClipboard(void);
  void onPasswordGenerated(QString, QString);
  void customCharacterSetCheckBoxToggled(bool);
  void customCharacterSetChanged(void);
  void updateValidator(void);
  void saveCurrentSettings(void);
  void domainSelected(const QString &);
  void newDomain(void);
  void setDirty(void);
  void about(void);
  void aboutQt(void);

signals:
  void passwordGenerated(QString, QString);

private: // methods
  void saveSettings(void);
  void restoreSettings(void);
  void saveDomainSettings(const QString &, const DomainSettings &);
  void loadSettings(const QString &domain);
  void generatePassword(void);
  void stopPasswordGeneration(void);
  void updateWindowTitle(void);

private:
  Ui::MainWindow *ui;
  QSettings mSettings;
  qreal mElapsed;
  QFuture<void> mPasswordGeneratorFuture;
  QMovie mLoaderIcon;
  bool mCustomCharacterSetDirty;
  bool mParameterSetDirty;
  QRegExpValidator mValidator;
  bool mAutoIncreaseIterations;
  QCompleter *mCompleter;
  bool mQuitHashing;
  PasswordGenerator mPasswordGenerator;
};

#endif // __MAINWINDOW_H_
