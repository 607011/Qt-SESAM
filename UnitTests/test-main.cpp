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

#include "../ctSESAM/pbkdf2.h"
#include "../ctSESAM/domainsettings.h"

#include <QMessageAuthenticationCode>
#include <QtTest/QTest>


class TestSESAM : public QObject
{
  Q_OBJECT
private slots:
  void hmac1(void);
  void simple0(void);
  void simple1(void);
  void simple2(void);
  void pin(void);
};

void TestSESAM::hmac1(void)
{
  QMessageAuthenticationCode hmac(QCryptographicHash::Sha512, "S3cr3t");
  hmac.addData("foo");
  QVERIFY(hmac.result() == QByteArray::fromHex("5576f3f707395cf33509379094424ff0743c990bf6da8b705baedb3aa25870a27069b177acbfe2bbd6780c6f7c8f735b0b4dcfcad3257a95b3f46c8b61435a3a"));
}


void TestSESAM::simple0(void)
{
  QVERIFY(QByteArray::fromBase64(DomainSettings::DefaultSalt_base64) == "pepper");
  QVERIFY(DomainSettings::DefaultSalt_base64 == QString("pepper").toUtf8().toBase64());
}

void TestSESAM::simple1(void)
{
  PBKDF2 pwd;
  DomainSettings ds;
  ds.domainName = "ct.de";
  ds.usedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.";
  ds.iterations = 4096;
  ds.length = 10;
  ds.salt_base64 = QString("pepper").toUtf8().toBase64();
  pwd.setDomainSettings(ds);
  pwd.generate("test", QCryptographicHash::Sha512);
  QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
  QVERIFY(pwd.key() == "YBVUH=sN/3");
}

void TestSESAM::simple2(void)
{
  PBKDF2 pwd;
  DomainSettings ds;
  ds.domainName = "MyFavoriteDomain";
  ds.usedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ";
  ds.iterations = 8192;
  ds.length = 16;
  ds.salt_base64 = QString("pepper").toUtf8().toBase64();
  pwd.setDomainSettings(ds);
  pwd.generate("foobar", QCryptographicHash::Sha512);
  QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
  QVERIFY(pwd.key() == "wLUwoQvKzBaYXbme");
}

void TestSESAM::pin(void)
{
  PBKDF2 pwd;
  DomainSettings ds;
  ds.domainName = "Bank";
  ds.usedCharacters = "0123456789";
  ds.iterations = 1;
  ds.length = 4;
  ds.salt_base64 = QString("pepper").toUtf8().toBase64();
  pwd.setDomainSettings(ds);
  pwd.generate("reallysafe", QCryptographicHash::Sha512);
  QVERIFY(pwd.hexKey() == "55b5f5cdd9bf2845e339650b4f6e1398cf7fe9ceed087eb5f5bc059882723579fc8ec27443417cf33c9763bafac6277fbe991bf27dd0206e78f7d9dfd574167f");
  QVERIFY(pwd.key() == "7809");
}



QTEST_MAIN(TestSESAM)
#include "test-main.moc"


