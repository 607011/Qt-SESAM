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

#include <QDebug>
#include <QList>
#include <QSslCipher>
#include <QSslKey>
#include <QFormLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringRef>

#include "util.h"
#include "servercertificatewidget.h"
#include "ui_servercertificatewidget.h"


ServerCertificateWidget::ServerCertificateWidget(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ServerCertificateWidget)
{
  ui->setupUi(this);
  setWindowIcon(QIcon(":/images/ctSESAM.ico"));
  QObject::connect(ui->acceptPushButton, SIGNAL(pressed()), SLOT(accept()));
  QObject::connect(ui->rejectPushButton, SIGNAL(pressed()), SLOT(reject()));
}


ServerCertificateWidget::~ServerCertificateWidget()
{
  delete ui;
}


void ServerCertificateWidget::setServerSocket(const QSslSocket &sslSocket)
{
  const QSslCipher &cipher = sslSocket.sessionCipher();
  const QString &fingerprint = fingerprintify(sslSocket.peerCertificateChain().last().digest(QCryptographicHash::Sha1));

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Encryption"), new QLabel(cipher.name()));
  formLayout->addRow(tr("Protocol"), new QLabel(cipher.protocolString()));
  formLayout->addRow(tr("Supported bits"), new QLabel(QString("%1").arg(cipher.supportedBits())));
  formLayout->addRow(tr("Used bits"), new QLabel(QString("%1").arg(cipher.usedBits())));

  ui->warningLabel->setText(
        tr("The certificate chain of host \"%1\" contains an officially untrusted certificate with the SHA1 fingerprint %2. "
           "Do you trust it? If yes, click \"Accept\" to import it.")
        .arg(sslSocket.peerName())
        .arg(fingerprint));

  QGroupBox *groupBox = new QGroupBox(tr("SSL parameters"));
  groupBox->setLayout(formLayout);

  QTreeWidget *treeWidget = new QTreeWidget;
  treeWidget->setColumnCount(2);
  treeWidget->setHeaderHidden(true);
  QTreeWidgetItem *firstItem = nullptr;
  QTreeWidgetItem *lastItem = nullptr;
  foreach (QSslCertificate cert, sslSocket.peerCertificateChain()) {
    QTreeWidgetItem *rootItem = new QTreeWidgetItem;
    const QString &fp = fingerprintify(cert.digest(QCryptographicHash::Sha1));
    if (firstItem == nullptr)
      firstItem = rootItem;
    treeWidget->addTopLevelItem(rootItem);
    QString shortFp = fp.mid(0, 21);
    rootItem->setText(0, shortFp + "…");
    rootItem->setText(1, QString());
    QList<QTreeWidgetItem*> items;
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Fingerprint (SHA1)"),
                                fp})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Effective date"),
                                cert.effectiveDate().toString()})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Expiry date"),
                                cert.expiryDate().toString()})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({
                                 tr("Issuer"),
                                 QString("/C=%1/ST=%2/L=%3/O=%4/OU=%5/CN=%6/emailAddress=%7")
                                 .arg(cert.issuerInfo(QSslCertificate::CountryName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::LocalityName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::Organization).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::CommonName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::EmailAddress).join(", "))
                               }
                               )));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({
                                 tr("Subject"),
                                 QString("/C=%1/ST=%2/L=%3/O=%4/OU=%5/CN=%6/emailAddress=%7")
                                 .arg(cert.subjectInfo(QSslCertificate::CountryName).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::StateOrProvinceName).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::LocalityName).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::Organization).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::CommonName).join(", "))
                                 .arg(cert.subjectInfo(QSslCertificate::EmailAddress).join(", "))
                               }
                               )));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Fingerprint (MD5)"),
                                fingerprintify(cert.digest(QCryptographicHash::Md5))})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Fingerprint (SHA256)"),
                                fingerprintify(cert.digest(QCryptographicHash::Sha256))})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Serial Number"),
                                QString(cert.serialNumber())})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Version"), QString(cert.version())})));
    rootItem->addChildren(items);
    lastItem = rootItem;
    if (fp == fingerprint)
      rootItem->setSelected(true);
  }

  if (firstItem != nullptr) {
    treeWidget->expandItem(firstItem);
  }

  if (lastItem != nullptr) {
    treeWidget->expandItem(lastItem);
  }

  treeWidget->resizeColumnToContents(0);
  treeWidget->resizeColumnToContents(1);

  QBoxLayout *vLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  vLayout->addWidget(groupBox);
  vLayout->addWidget(treeWidget);

  if (ui->scrollArea->layout() != nullptr)
    delete ui->scrollArea->layout();
  ui->scrollArea->setLayout(vLayout);
}
