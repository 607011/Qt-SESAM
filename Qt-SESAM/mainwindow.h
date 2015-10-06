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
#include <QFuture>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>
#include <QSystemTrayIcon>
#include <QNetworkReply>
#include <QList>
#include <QSslError>
#include <QEvent>

#include "global.h"
#include "domainsettingslist.h"
#include "pbkdf2.h"

namespace Ui {
class MainWindow;
}


class MainWindowPrivate;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(bool forceStart, QWidget *parent = nullptr);

  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *);
  void changeEvent(QEvent *);

private:
  typedef enum _Type {
    SyncPeerFile = 0x00000001,
    SyncPeerServer = 0x00000002,
    LastPeerName
  } SyncPeer;

private slots:
  void updatePassword(void);
  void copyUsernameToClipboard(void);
  void copyGeneratedPasswordToClipboard(void);
  void copyLegacyPasswordToClipboard(void);
  void onOptionsAccepted(void);
  void onServerCertificatesUpdated(const QList<QSslCertificate> &certs);
  void showOptionsDialog(void);
  void onPasswordGenerated(void);
  void onPasswordGenerationAborted(void);
  void onPasswordGenerationStarted(void);
  void saveCurrentDomainSettings(void);
  void onLegacyPasswordChanged(QString legacyPassword);
  void onDomainTextChanged(const QString &);
  void onDomainSelected(const QString &);
  void onEasySelectorValuesChanged(int, int);
  void onEasySelectorValuesChanged(int, int, int, int);
  void renewSalt(void);
  void onRenewSalt(void);
  void onNewDomain(void);
  void cancelPasswordGeneration(void);
  void stopPasswordGeneration(void);
  void changeMasterPassword(void);
  void nextChangeMasterPasswordStep(void);
  void setDirty(bool dirty = true);
  void onURLChanged(void);
  void openURL(void);
  void sync(void);
  void syncWith(SyncPeer syncPeer, const QByteArray &baDomains);
  void clearClipboard(void);
  void about(void);
  void aboutQt(void);
  void enterMasterPassword(void);
  void onMasterPasswordEntered(void);
  void clearAllSettings(void);
  void lockApplication(void);
  void drawCountdown(void);
  void invalidatePassword(bool reenter = true);
  void showHide(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);
  void saveSettings(void);
  void sslErrorsOccured(QNetworkReply*, const QList<QSslError> &);
  void onDeleteFinished(QNetworkReply*);
  void onReadFinished(QNetworkReply*);
  void onWriteFinished(QNetworkReply*);
  void cancelServerOperation(void);
#if HACKING_MODE_ENABLED
  void hackLegacyPassword(void);
#endif
  void hideActivityIcons(void);
  void createFullDump(void);
  QFuture<void> &generateSaltKeyIV(void);
  void onGenerateSaltKeyIV(void);
#ifdef WIN32
  void onPasted(void);
#endif

signals:
  void passwordGenerated(void);
  void saltKeyIVGenerated(void);

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

private: // methods
  int checkSaveOnDirty(void);
  void resetAllFieldsExceptDomainComboBox(void);
  void resetAllFields(void);
  bool restoreSettings(void);
  void saveAllDomainDataToSettings(void);
  bool restoreDomainDataFromSettings(void);
  void copyDomainSettingsToGUI(const QString &domain);
  void generatePassword(void);
  void updateWindowTitle(void);
  void makeDomainComboBox(void);
  void wrongPasswordWarning(int errCode, QString errMsg);
  void restartInvalidationTimer(void);
  void generateSaltKeyIVThread(void);
  DomainSettings collectedDomainSettings(void) const;
  QByteArray cryptedRemoteDomains(void);
  void mergeLocalAndRemoteData(void);
  void writeToRemote(SyncPeer syncPeer);
  void sendToSyncServer(const QByteArray &cipher);
  void writeToSyncFile(const QByteArray &cipher);
  void writeBackupFile(const QByteArray &binaryDomainData);
  bool syncToServerEnabled(void) const;
  bool syncToFileEnabled(void) const;
  int findDomainInComboBox(const QString &domain) const;
  int findDomainInComboBox(const QString &domain, int lo, int hi) const;
  bool domainComboboxContains(const QString &domain) const;
  void createTemplate_v3(void);
  QString usedCharacters_v3(void);
  void analyzeTemplate_v3(const QByteArray &);
};

#endif // __MAINWINDOW_H_
