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

#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>
#include <QString>
#include <QMovie>
#include <QCloseEvent>
#include <QLineEdit>
#include <QSettings>
#include <QCompleter>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>
#include <QSystemTrayIcon>

#include "domainsettings.h"
#include "password.h"
#include "credentialsdialog.h"
#include "optionsdialog.h"

namespace Ui {
class MainWindow;
}


class MainWindowPrivate;

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
  void onPasswordGenerated(void);
  void onPasswordGenerationAborted(void);
  void onPasswordGenerationStarted(void);
  void customCharacterSetCheckBoxToggled(bool);
  void customCharacterSetChanged(void);
  void updateValidator(void);
  void saveCurrentSettings(void);
  void domainSelected(const QString &);
  void newDomain(void);
  void stopPasswordGeneration(void);
  void setDirty(bool dirty = true);
  void sync(void);
  void clearClipboard(void);
  void about(void);
  void aboutQt(void);
  void enterCredentials(void);
  void credentialsEntered(void);
  void invalidatePassword(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);
  void saveSettings(void);

signals:
  void passwordGenerated(void);
  void reenterCredentials(void);

private: // methods
  void restoreSettings(void);
  void saveDomainDataToSettings(void);
  void saveDomainDataToSettings(DomainSettings);
  void restoreDomainDataFromSettings(void);
  void copyDomainSettingsToGUI(const QString &domain);
  void generatePassword(void);
  void updateWindowTitle(void);
  void zeroize(QLineEdit *);
  template <class T>
  void zeroize(T *, int len);
  void invalidatePassword(QLineEdit*);
  void wrongPasswordWarning(int errCode, QString errMsg);
  QByteArray decode(const QByteArray &, int *errCode = nullptr, QString *errMsg = nullptr);
  QByteArray encode(const QByteArray &, int *errCode = nullptr, QString *errMsg = nullptr);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

};

#endif // __MAINWINDOW_H_
