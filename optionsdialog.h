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

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
  Q_OBJECT

  Q_PROPERTY(QByteArray serverCredentials READ serverCredentials)
  Q_PROPERTY(QString validator READ syncFilename WRITE setSyncFilename)
  Q_PROPERTY(bool useSyncServer READ useSyncServer WRITE setUseSyncServer)
  Q_PROPERTY(bool useSyncFile READ useSyncFile WRITE setUseSyncFile)
  Q_PROPERTY(QString serverCertificateFilename READ serverCertificateFilename WRITE setServerCertificateFilename)
  Q_PROPERTY(QList<QSslCertificate> serverCertificates READ serverCertificates)
  Q_PROPERTY(QList<QByteArray> serverCertificatesPEM READ serverCertificatesPEM)
  Q_PROPERTY(QString serverUsername READ serverUsername WRITE setServerUsername)
  Q_PROPERTY(QString serverPassword READ serverPassword WRITE setServerPassword)
  Q_PROPERTY(QString writeUrl READ writeUrl WRITE setWriteUrl)
  Q_PROPERTY(QString readUrl READ readUrl WRITE setReadUrl)
  Q_PROPERTY(int masterPasswordInvalidationTimeMins READ masterPasswordInvalidationTimeMins WRITE setMasterPasswordInvalidationTimeMins)


public:
  explicit OptionsDialog(QWidget *parent = nullptr);
  ~OptionsDialog();

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

  QString serverCertificateFilename(void) const;
  void setServerCertificateFilename(QString);

  const QList<QSslCertificate> &serverCertificates(void) const;
  QList<QByteArray> serverCertificatesPEM(void) const;

  QString writeUrl(void) const;
  void setWriteUrl(QString);

  QString readUrl(void) const;
  void setReadUrl(QString);

  QByteArray serverCredentials(void) const;

  int masterPasswordInvalidationTimeMins(void) const;
  void setMasterPasswordInvalidationTimeMins(int minutes);


private slots:
  void chooseSyncFile(void);
  void chooseCertFile(void);
  void okClicked(void);

private:
  Ui::OptionsDialog *ui;

  QList<QSslCertificate> mServerCertificates;
};

#endif // __OPTIONSDIALOG_H_
