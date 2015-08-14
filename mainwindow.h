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
#include <QNetworkReply>
#include <QList>
#include <QSslError>

#include "global.h"
#include "domainsettingslist.h"
#include "password.h"

namespace Ui {
class MainWindow;
}


class MainWindowPrivate;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void setDomainComboBox(QStringList domainList);
protected:
  void closeEvent(QCloseEvent *);
  void changeEvent(QEvent *);

private:
  typedef enum _Type {
    FileSource,
    ServerSource
  } SyncSource;

private slots:
  void updatePassword(void);
  void copyGeneratedPasswordToClipboard(void);
  void copyLegacyPasswordToClipboard(void);
  void onPasswordGenerated(void);
  void onPasswordGenerationAborted(void);
  void onPasswordGenerationStarted(void);
  void saveCurrentSettings(void);
  void domainSelected(const QString &);
  void newDomain(void);
  void renewSalt(void);
  void onRenewSalt(void);
  void cancelPasswordGeneration(void);
  void stopPasswordGeneration(void);
  void setDirty(bool dirty = true);
  void sync(void);
  void sync(SyncSource, const QByteArray &baDomains);
  void clearClipboard(void);
  void about(void);
  void aboutQt(void);
  void enterMasterPassword(void);
  void masterPasswordEntered(void);
  void invalidatePassword(bool reenter = true);
  void showHide(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);
  void saveSettings(void);
  void sslErrorsOccured(QNetworkReply*, QList<QSslError>);
  void updateSaveButtonIcon(int frame = 0);
  void readFinished(QNetworkReply*);
  void writeFinished(QNetworkReply*);
  void deleteFinished(QNetworkReply*);
  void cancelServerOperation(void);
  void loadCertificate(void);
  void hackLegacyPassword(void);
  void hideActivityIcons(void);
#ifdef WIN32
  void createFullDump(void);
#endif

signals:
  void passwordGenerated(void);
  void badMasterPassword(void);

private: // methods
  void resetAllFields(void);
  bool restoreSettings(void);
  void saveAllDomainDataToSettings(void);
  bool restoreDomainDataFromSettings(void);
  void copyDomainSettingsToGUI(const QString &domain);
  void generatePassword(void);
  void updateWindowTitle(void);
  void wrongPasswordWarning(int errCode, QString errMsg);
  void restartInvalidationTimer(void);
  void unblockUpdatePassword(void);
  void blockUpdatePassword(void);
  bool keyContainsAnyOf(const QString &forcedCharacters);
  bool generatedPasswordIsValid(void);
  void analyzeGeneratedPassword(void);
  DomainSettings collectedDomainSettings(void) const;
  QByteArray encode(const QByteArray &, bool compress, int *errCode = nullptr, QString *errMsg = nullptr);
  QByteArray decode(const QByteArray &, bool uncompress, int *errCode = nullptr, QString *errMsg = nullptr);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

};

#endif // __MAINWINDOW_H_
