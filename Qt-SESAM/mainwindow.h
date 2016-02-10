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
#include <QJsonDocument>
#include <QImage>
#include <QTableWidgetItem>

#include "global.h"
#include "password.h"
#include "domainsettings.h"
#include "domainsettingslist.h"
#include "pbkdf2.h"
#include "securebytearray.h"

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

  static int const EXIT_CODE_RESTART_APP;
  static QString defaultLocale(void);

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
  void onTagChanged(QString);
  void onCustomContextMenu(const QPoint &point);
  void onAddGroup(void);
  void onEditGroup(void);
  void onDomainViewClicked(const QModelIndex &);
  void onDomainViewDoubleClicked(const QModelIndex &);
  void onGroupNameChanged();
  void onSaltChanged(QString);
  void updatePassword(void);
  void copyUsernameToClipboard(void);
  void copyPasswordToClipboard(void);
  void copyGeneratedPasswordToClipboard(void);
  void copyLegacyPasswordToClipboard(void);
  void onServerCertificatesUpdated(const QList<QSslCertificate> &certs);
  void showOptionsDialog(void);
  void onPasswordGenerated(void);
  void onPasswordGenerationAborted(void);
  void onPasswordGenerationStarted(void);
  void saveCurrentDomainSettings(void);
  void deleteCurrentDomainSettings(void);
  void onLegacyPasswordChanged(QString);
  void onNotesChanged(void);
  void onDomainTextChanged(const QString &);
  void onEasySelectorValuesChanged(int passwordLength, int complexityValue);
  void onExportAllDomainSettingAsJSON(void);
  void onExportAllLoginDataAsClearText(void);
  void onExportCurrentSettingsAsQRCode(void);
  void onPasswordTemplateChanged(const QString &);
  void masterPasswordInvalidationTimeMinsChanged(int);
  void onShuffleUsername(void);
  void onNewDomain(void);
  void onRevert(void);
  void renewSalt(void);
  void onRenewSalt(void);
  void cancelPasswordGeneration(void);
  void stopPasswordGeneration(void);
  void changeMasterPassword(void);
  void nextChangeMasterPasswordStep(void);
  void setDirty(bool dirty);
  void openURL(void);
  void onForcedPush(void);
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
  void onMasterPasswordClosing(void);
  void clearAllSettings(void);
  void lockApplication(void);
  void invalidateMasterPassword(bool reenter = true);
  void showHide(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);
  void saveSettings(void);
  void saveUiSettings(void);
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
  void onGeneratedSaltKeyIV(void);
  void onExportKGK(void);
  void onImportKGK(void);
  void onImportKeePass2XmlFile(void);
  void onImportPasswordSafeFile(void);
  void onBackupFilesRemoved(bool ok);
  void onBackupFilesRemoved(int);
  void onSelectLanguage(QAction *);
  void onAttachFile(void);

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
  void createLanguageMenu(void);
  void setLanguage(const QString &);
  QMessageBox::StandardButton saveYesNoCancel(void);
  void resetAllFieldsExceptDomainName(void);
  void resetAllFields(void);
  bool restoreSettings(void);
  void restoreLanguageSettings(void);
  void saveDomainSettings(DomainSettings ds);
  void saveAllDomainDataToSettings(void);
  bool restoreDomainDataFromSettings(void);
  void copyDomainSettingsToGUI(const DomainSettings ds);
  void generatePassword(void);
  void updateWindowTitle(void);
  void wrongPasswordWarning(int errCode, QString errMsg);
  void restartInvalidationTimer(void);
  void generateSaltKeyIVThread(void);
  DomainSettings collectedDomainSettings() const;
  QByteArray cryptedRemoteDomains(void);
  void mergeLocalAndRemoteData(void);
  void writeToRemote(SyncPeer syncPeer);
  void sendToSyncServer(const QByteArray &cipher);
  void writeToSyncFile(const QByteArray &cipher);
  void writeBackupFile(void);
  void createEmptySyncFile(void);
  void syncWithFile(void);
  void beginSyncWithServer(void);
  void applyComplexity(int complexityValue);
  void setTemplate(void);
  void applyTemplateStringToGUI(const QString &);
  void updateCheckableLabel(QLabel *, bool checked);
  void warnAboutDifferingKGKs(void);
  void convertToLegacyPassword(DomainSettings &ds);
  QString collectedSyncData(void);
  QString selectAlternativeDomainNameFor(const QString &domainName, const QStringList &domainNameList);
  void saveSyncDataToSettings(void);
  bool wipeFile(const QString &filename);
  void cleanupAfterMasterPasswordChanged(void);
  void prepareExit(void);
  void removeOutdatedBackupFilesThread(void);
  QImage currentDomainSettings2QRCode(void) const;
  bool validCredentials(void) const;
  void attachFile(const QString &filename);
  void setAttachments(const QVariantMap &attachments);
  int attachmentRow(const QString &filename) const;
  bool attachmentExists(const QString &filename) const;
  void saveAttachmentAs(const QTableWidgetItem *);
  void deleteAttachment(const QTableWidgetItem *);
  void restoreUiSettings(void);
  bool restoreSyncSettings(void);
  void appendAttachmentToTable(const QString &filename, const QByteArray &contents);
  void executeAttachmentContextMenu(QEvent *event);
  void dragEnterAttachmentWidget(QEvent *event);
};

#endif // __MAINWINDOW_H_
