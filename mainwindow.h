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
#include <QMovie>
#include <QCloseEvent>
#include <QSettings>
#include <QCompleter>
#include <QMutex>
#include <QJsonDocument>
#include <QVariantMap>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSystemTrayIcon>

#include "domainsettings.h"
#include "password.h"
#include "credentialsdialog.h"

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
  void changeEvent(QEvent *);

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
  void replyFinished(QNetworkReply*);
  void sync(void);
  void clearClipboard(void);
  void sslErrorsOccured(QNetworkReply*,QList<QSslError>);
  void updateSaveButtonIcon(int frame = 0);
  void about(void);
  void aboutQt(void);
  void enterCredentials(void);
  void credentialsEntered(void);
  void trayIconActivated(QSystemTrayIcon::ActivationReason);

signals:
  void passwordGenerated(void);

private: // methods
  void saveSettings(void);
  void restoreSettings(void);
  void saveDomainSettings(DomainSettings);
  void loadDomainSettings(const QString &domain);
  void generatePassword(void);
  void updateWindowTitle(void);

private:
  static const QString DefaultServerRoot;
  static const QString DefaultWriteUrl;
  static const QString DefaultReadUrl;
  static const QString DefaultDeleteUrl;

  Ui::MainWindow *ui;
  CredentialsDialog *mCredentialsDialog;
  QSettings mSettings;
  QVariantMap mDomains;
  QMovie mLoaderIcon;
  bool mCustomCharacterSetDirty;
  bool mParameterSetDirty;
  bool mAutoIncreaseIterations;
  QCompleter *mCompleter;
  Password mPassword;
  QString mServerRoot;
  QString mWriteUrl;
  QString mReadUrl;
  QString mDeleteUrl;
  QNetworkAccessManager *mNAM;
  QNetworkReply *mReply;
  QDateTime mCreatedDate;
  QDateTime mModifiedDate;
  QString mServerCredentials;
  QSystemTrayIcon mTrayIcon;
};

#endif // __MAINWINDOW_H_
