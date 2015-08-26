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
#include <QList>

namespace Ui {
class OptionsDialog;
}

class OptionsDialogPrivate;


class OptionsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit OptionsDialog(QWidget *parent = nullptr);
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
  const QSslCertificate &selfSignedCertificate(void) const;
  QSslCertificate serverCertificate(void) const;
  const QList<QSslError> &sslErrors(void) const;
  const QList<QSslError> &ignoredSslErrors(void) const;

  QString writeUrl(void) const;
  void setWriteUrl(QString);

  QString readUrl(void) const;
  void setReadUrl(QString);

  int saltLength(void) const;
  void setSaltLength(int);

  QByteArray serverCredentials(void) const;

  int masterPasswordInvalidationTimeMins(void) const;
  void setMasterPasswordInvalidationTimeMins(int minutes);

signals:
  void updatedServerCertificates(void);

private slots:
  void chooseSyncFile(void);
  void okClicked(void);
  void onEncrypted(void);
  void verifySecureConnection(void);
  void sslErrorsOccured(const QList<QSslError> &errors);

private:
  Ui::OptionsDialog *ui;

  QScopedPointer<OptionsDialogPrivate> d_ptr;
  Q_DECLARE_PRIVATE(OptionsDialog)
  Q_DISABLE_COPY(OptionsDialog)
};

#endif // __OPTIONSDIALOG_H_
