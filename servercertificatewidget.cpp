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
  const QSslCipher &cipher = sslSocket.sessionCipher();

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Encryption"), new QLabel(cipher.name()));
  formLayout->addRow(tr("Protocol"), new QLabel(cipher.protocolString()));
  formLayout->addRow(tr("Supported bits"), new QLabel(QString("%1").arg(cipher.supportedBits())));
  formLayout->addRow(tr("Used bits"), new QLabel(QString("%1").arg(cipher.usedBits())));

  QGroupBox *groupBox = new QGroupBox(tr("SSL parameters"));
  groupBox->setLayout(formLayout);

  QTreeWidget *treeWidget = new QTreeWidget;
  treeWidget->setColumnCount(2);
  treeWidget->setHeaderLabels(QStringList({tr("Serial Number"), QString()}));
  foreach (QSslCertificate cert, sslSocket.peerCertificateChain()) {
    // qDebug() << cert.toText();
    // const QSslKey &publicKey = cert.publicKey();
    QTreeWidgetItem *rootItem = new QTreeWidgetItem;
    treeWidget->addTopLevelItem(rootItem);
    rootItem->setText(0, QString(cert.serialNumber()));
    rootItem->setText(1, QString());
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
                   QStringList({
                                 tr("DN"),
                                 QString("CN=%1,OU=%2,O=%3,L=%4,ST=%5")
                                 .arg(cert.issuerInfo(QSslCertificate::CommonName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::Organization).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::LocalityName).join(", "))
                                 .arg(cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(", "))
                               }
                               )));
    rootItem->addChildren(items);
  }

  QBoxLayout *vLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  vLayout->addWidget(groupBox);
  vLayout->addWidget(treeWidget);

  if (ui->scrollArea->layout() != nullptr)
    delete ui->scrollArea->layout();
  ui->scrollArea->setLayout(vLayout);
}
