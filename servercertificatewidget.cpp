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
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "servercertificatewidget.h"
#include "ui_servercertificatewidget.h"

ServerCertificateWidget::ServerCertificateWidget(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::ServerCertificateWidget)
{
  ui->setupUi(this);
  QObject::connect(ui->acceptPushButton, SIGNAL(pressed()), SLOT(accept()));
  QObject::connect(ui->rejectPushButton, SIGNAL(pressed()), SLOT(reject()));
}


ServerCertificateWidget::~ServerCertificateWidget()
{
  delete ui;
}


void ServerCertificateWidget::setServerSocket(const QSslSocket &sslSocket)
{
  // clearLayout(ui->formLayout);
  qDebug() << "ServerCertificateWidget::setServerSocket()";
  const QSslCipher &cipher = sslSocket.sessionCipher();
  ui->formLayout->addRow(tr("<B>Authentication</B>"), new QLabel(cipher.authenticationMethod()));
  ui->formLayout->addRow(tr("<B>Encryption</B>"), new QLabel(cipher.encryptionMethod()));
  ui->formLayout->addRow(tr("<B>Key Exchange Method</B>"), new QLabel(cipher.keyExchangeMethod()));
  ui->formLayout->addRow(tr("<B>Cipher Name</B>"), new QLabel(cipher.name()));
  ui->formLayout->addRow(tr("<B>Protocol</B>"), new QLabel(cipher.protocolString()));
  ui->formLayout->addRow(tr("<B>Supported Bits</B>"), new QLabel(QString("%1").arg(cipher.supportedBits())));
  ui->formLayout->addRow(tr("<B>Used Bits</B>"), new QLabel(QString("%1").arg(cipher.usedBits())));

  QTreeWidget *treeWidget = new QTreeWidget;
  treeWidget->setColumnCount(2);
  treeWidget->setHeaderLabels(QStringList({tr("Serial Number"), QString()}));
  foreach (QSslCertificate cert, sslSocket.peerCertificateChain()) {
    qDebug() << cert;
    QTreeWidgetItem *rootItem = new QTreeWidgetItem;
    treeWidget->addTopLevelItem(rootItem);
    rootItem->setText(0, tr("Serial number"));
    rootItem->setText(1, QString(cert.serialNumber()));
    QList<QTreeWidgetItem*> items;
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Version"), QString(cert.version())})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Effective date"), cert.effectiveDate().toString()})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Expiry date"), cert.expiryDate().toString()})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Common name"), cert.issuerInfo(QSslCertificate::CommonName).join(", ")})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Organization"), cert.issuerInfo(QSslCertificate::Organization).join(", ")})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Locality"), cert.issuerInfo(QSslCertificate::LocalityName).join(", ")})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("Organizational unit"), cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(", ")})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("State or province"), cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(", ")})));
    items.append(new QTreeWidgetItem(
                   (QTreeWidget*)nullptr,
                   QStringList({tr("E-mail address"),cert.issuerInfo(QSslCertificate::EmailAddress).join(", ")})));
    // ui->formLayout->addRow(tr("Common Name"), new QLabel(cert.subjectInfo(QSslCertificate::CommonName).join(", ")));
    // ui->formLayout->addRow(tr("Organization"), new QLabel(cert.subjectInfo(QSslCertificate::Organization).join(", ")));
    // ui->formLayout->addRow(tr("Locality Name"), new QLabel(cert.subjectInfo(QSslCertificate::LocalityName).join(", ")));
    // ui->formLayout->addRow(tr("Organizational Unit Name"), new QLabel(cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join(", ")));
    // ui->formLayout->addRow(tr("State Or Province Name"), new QLabel(cert.subjectInfo(QSslCertificate::StateOrProvinceName).join(", ")));
    rootItem->addChildren(items);
  }
  ui->verticalLayout->addWidget(treeWidget);
}


void ServerCertificateWidget::clearLayout(QLayout *layout)
{
  Q_ASSERT(layout != nullptr);
  QLayoutItem *item;
  while ((item = layout->takeAt(0)) != nullptr) {
    QLayout *subLayout = item->layout();
    QWidget *widget = item->widget();
    if (subLayout != nullptr) {
      subLayout->removeItem(item);
      clearLayout(subLayout);
    }
    else if (widget != nullptr) {
      widget->hide();
      delete widget;
    }
    else {
      delete item;
    }
  }
  delete layout;
}
