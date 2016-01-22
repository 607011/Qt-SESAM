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
#include <QResizeEvent>
#include <QMoveEvent>
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
#include <QMessageBox>
#include <QLabel>
#include <QModelIndex>

#include "global.h"
#include "domainsettings.h"
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
  explicit MainWindow(bool forceStart, QWidget *parent = Q_NULLPTR);
  ~MainWindow();

  virtual QSize sizeHint(void) const;
  virtual QSize minimumSizeHint(void) const;

private:
  typedef enum _Type {
    SyncPeerFile = 0x00000001,
    SyncPeerServer = 0x00000002,
    LastPeerName
  } SyncPeer;

private slots:
  void onLogin(void);
  void onMessageFromTcpClient(QJsonDocument);
  void onUserChanged(QString);
  void onURLChanged(QString);
  void onUsedCharactersChanged(void);
  void onExtraCharactersChanged(QString);
  void onPasswordLengthChanged(int);
  void onIterationsChanged(int);
  void onGroupChanged(QString);
  void onTagChanged(QString);
  void onAddGroup(void);
  void onDomainViewClicked(const QModelIndex &);
  void onDomainViewDoubleClicked(const QModelIndex &);
  void onSaltChanged(QString);
  void onDeleteChanged(bool);
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
  void onDomainSelected(QString);
  void onEasySelectorValuesChanged(int, int);
  void onPasswordTemplateChanged(const QString &);
  void masterPasswordInvalidationTimeMinsChanged(int);
  void onRevert(void);
  void renewSalt(void);
  void onRenewSalt(void);
  void cancelPasswordGeneration(void);
  void stopPasswordGeneration(void);
  void changeMasterPassword(void);
  void nextChangeMasterPasswordStep(void);
  void setDirty(bool dirty = true);
  void openURL(void);
  void onForcedPush(void);
  void onMigrateDomainSettingsToExpert(void);
  void onSync(void);
  void syncWith(SyncPeer syncPeer, const QByteArray &baDomains);
  void onExpandableCheckBoxStateChanged(void);
  void onTabChanged(int idx);
  void clearClipboard(void);
  void shrink(void);
  void about(void);
  void aboutQt(void);
  void enterMasterPassword(void);
  void onMasterPasswordEntered(void);
  void clearAllSettings(void);
  void lockApplication(void);
  void invalidatePassword(bool reenter = true);
  void showHide(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);
  void saveSettings(void);
  void sslErrorsOccured(QNetworkReply*, const QList<QSslError> &);
  void onDeleteFinished(QNetworkReply*);
  void onReadFinished(QNetworkReply*);
  void onWriteFinished(QNetworkReply*);
  void cancelServerOperation(void);
  void removeOutdatedBackupFiles(void);
#if HACKING_MODE_ENABLED
  void hackLegacyPassword(void);
#endif
  QFuture<void> &generateSaltKeyIV(void);
  void onGenerateSaltKeyIV(void);
  void onExportKGK(void);
  void onImportKGK(void);
  void onImportKeePass2XmlFile(void);
  void onImportPasswordSafeFile(void);
  void onBackupFilesRemoved(bool ok);
  void onBackupFilesRemoved(int);

signals:
  void passwordGenerated(void);
  void saltKeyIVGenerated(void);
  void backupFilesDeleted(int);
  void backupFilesDeleted(bool);

protected:
  void closeEvent(QCloseEvent *);
  void changeEvent(QEvent *);
  void resizeEvent(QResizeEvent*);
  bool event(QEvent *);
  bool eventFilter(QObject *obj, QEvent *event);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> d_ptr;
  Q_DECLARE_PRIVATE(MainWindow)
  Q_DISABLE_COPY(MainWindow)

private: // methods
  QMessageBox::StandardButton saveYesNoCancel(void);
  void resetAllFieldsExceptDomainComboBox(void);
  void resetAllFields(void);
  bool restoreSettings(void);
  void saveDomainSettings(DomainSettings ds);
  void saveAllDomainDataToSettings(void);
  bool restoreDomainDataFromSettings(void);
  void copyDomainSettingsToGUI(const DomainSettings &ds);
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
  void writeBackupFile(void);
  void createEmptySyncFile(void);
  void syncWithFile(void);
  void beginSyncWithServer(void);
  int findDomainInComboBox(const QString &domain) const;
  int findDomainInComboBox(const QString &domain, int lo, int hi) const;
  bool domainComboboxContains(const QString &domain) const;
  void applyComplexity(int complexity);
  void setTemplateAndUsedCharacters(void);
  QString usedCharacters(void);
  void applyTemplateStringToGUI(const QByteArray &);
  void updateCheckableLabel(QLabel *, bool checked);
  QString selectAlternativeDomainNameFor(const QString &domainName);
  void warnAboutDifferingKGKs(void);
  void convertToLegacyPassword(DomainSettings &ds);
  QString selectAlternativeDomainNameFor(const QString &domainName, const QStringList &domainNameList);
  QString collectedSyncData(void);
  bool wipeFile(const QString &filename);
  void cleanupAfterMasterPasswordChanged(void);
  void prepareExit(void);
  void removeOutdatedBackupFilesThread(void);
};

#endif // __MAINWINDOW_H_
