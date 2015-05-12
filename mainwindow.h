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
#include <QFuture>
#include <QElapsedTimer>
#include <QMovie>
#include <QRegExpValidator>
#include <QCloseEvent>
#include <QSettings>
#include <QMap>
#include <QCompleter>

namespace Ui {
class MainWindow;
}


struct DomainSettings {
  DomainSettings(void)
    : useLowerCase(true)
    , useUpperCase(true)
    , useDigits(true)
    , useExtra(true)
    , useCustom(false)
    , iterations(4096)
    , salt("This is my salt. There are many like it, but this one is mine.")
    , validator("^(?=.*\d)(?=.*[a-z])(?=.*[A-Z])[a-zA-Z0-9]+$", Qt::CaseSensitive, QRegExp::RegExp2)
  { /* ... */ }
  bool useLowerCase;
  bool useUpperCase;
  bool useDigits;
  bool useExtra;
  bool useCustom;
  QString customCharacters;
  int iterations;
  QString salt;
  QRegExp validator;
};


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
  void onPasswordGenerated(QString);
  void customCharacterSetCheckBoxToggled(bool);
  void customCharacterSetChanged(void);
  void updateValidator(void);
  void saveCurrentSettings(void);

signals:
  void passwordGenerated(QString);

private: // methods
  void saveSettings(void);
  void saveDomainSettings(QSettings &, const QString &, const DomainSettings &);
  void restoreSettings(void);
  void generatePassword(void);

private:
  Ui::MainWindow *ui;
  QElapsedTimer mElapsedTimer;
  qreal mElapsed;
  QFuture<void> mPasswordGeneratorFuture;
  QMovie mLoaderIcon;
  bool mCustomCharacterSetDirty;
  QRegExpValidator mValidator;
  bool mAutoIncreaseIterations;
  QMap<QString, DomainSettings> mDomainParam;
  QCompleter *mCompleter;
};

#endif // __MAINWINDOW_H_
