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

#ifndef __OPTIONSDIALOG_H_
#define __OPTIONSDIALOG_H_

#include <QDialog>
#include <QSslCertificate>
#include <QSslSocket>
#include <QSslError>
#include <QNetworkReply>
#include <QList>
#include <QString>
#include <QEvent>

namespace Ui {
class OptionsDialog;
}

class OptionsDialogPrivate;


class OptionsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit OptionsDialog(QWidget *parent = Q_NULLPTR);
  ~OptionsDialog();

  bool syncOnStart(void) const;
  void setSyncOnStart(bool);

  QString syncFilename(void) const;
  void setSyncFilename(const QString &);

  bool useSyncServer(void) const;
  void setUseSyncServer(bool);

  bool useSyncFile(void) const;
  void setUseSyncFile(bool);

  QString serverRootUrl(void) const;
  void setServerRootUrl(QString);

  QString serverUsername(void) const;
  void setServerUsername(QString);

  QString serverPassword(void) const;
  void setServerPassword(QString);

  const QList<QSslCertificate> &serverCertificates(void) const;
  void setServerCertificates(const QList<QSslCertificate> &);
  QSslCertificate serverRootCertificate(void) const;

  void setSecure(bool);
  bool secure(void) const;

  QString writeUrl(void) const;
  void setWriteUrl(QString);

  QString readUrl(void) const;
  void setReadUrl(QString);

  QString deleteUrl(void) const;
  void setDeleteUrl(QString);

  int saltLength(void) const;
  void setSaltLength(int);

  QByteArray httpBasicAuthenticationString(void) const;

  void setWriteBackups(bool);
  bool writeBackups(void) const;

  void setAutoDeleteBackupFiles(bool);
  bool autoDeleteBackupFiles(void) const;

  void setMaxBackupFileAge(int days);
  int maxBackupFileAge(void) const;

  void setLoggingEnabled(bool);
  bool loggingEnabled(void) const;

#ifdef WIN32
  void setSmartLogin(bool);
  bool smartLogin(void) const;
#endif

  int masterPasswordInvalidationTimeMins(void) const;
  void setMasterPasswordInvalidationTimeMins(int minutes);

  QString passwordFilename(void) const;
  void setPasswordFilename(const QString &filename);

  int maxPasswordLength(void) const;
  void setMaxPasswordLength(int);

  int defaultPasswordLength(void) const;
  void setDefaultPasswordLength(int);

  int defaultIterations(void) const;
  void setDefaultIterations(int);

  bool syncToFileEnabled(void) const;
  bool syncToServerEnabled(void) const;

  bool extensiveWipeout(void) const;
  void setExtensiveWipeout(bool checked);

protected:
  void changeEvent(QEvent *);

signals:
  void serverCertificatesUpdated(QList<QSslCertificate>);
  void saltLengthChanged(int);
  void maxPasswordLengthChanged(int);
  void defaultPasswordLengthChanged(int);
  void masterPasswordInvalidationTimeMinsChanged(int);

private slots:
  void chooseSyncFile(void);
  void choosePasswordFile(void);
  void okClicked(void);
  void onEncrypted(QNetworkReply *);
  void checkConnectivity(void);
  void validateHostCertificateChain(void);
  void onReadFinished(QNetworkReply*);
  void sslErrorsOccured(QNetworkReply *, const QList<QSslError> &);
  void onServerRootUrlChanged(QString);

private:
  Ui::OptionsDialog *ui;

  QScopedPointer<OptionsDialogPrivate> d_ptr;
  Q_DECLARE_PRIVATE(OptionsDialog)
  Q_DISABLE_COPY(OptionsDialog)
};

#endif // __OPTIONSDIALOG_H_
