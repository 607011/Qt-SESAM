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

#include "testpbkdf2.h"
#include "password.h"
#include "domainsettings.h"

TestPBKDF2::TestPBKDF2(QObject *parent)
  : QObject(parent)
{
  // ...
}


void TestPBKDF2::simple(void)
{
  QVERIFY(QByteArray::fromBase64(DomainSettings::DefaultSaltBase64) == "pepper");
  QVERIFY(DomainSettings::DefaultSaltBase64 == QString("pepper").toUtf8().toBase64());
}

void TestPBKDF2::simple1a(void)
{
  Password pwd;
  pwd.generate(PasswordParam("ct.de", "test", QString("pepper").toUtf8(), "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.", 10, 4096));
  QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
  QVERIFY(pwd.key() == "YBVUH=sN/3");
}

void TestPBKDF2::simple1b(void)
{
  Password pwd;
  pwd.generate(PasswordParam("ct.de", "test", "pepper", "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.", 10, 4096));
  QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
  QVERIFY(pwd.key() == "YBVUH=sN/3");
}

void TestPBKDF2::simple2(void)
{
  Password pwd;
  pwd.generate(PasswordParam("MyFavoriteDomain", "foobar", "pepper", "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ", 16, 8192));
  QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
  QVERIFY(pwd.key() == "wLUwoQvKzBaYXbme");
}

void TestPBKDF2::pin(void)
{
  Password pwd;
  pwd.generate(PasswordParam("Bank", "reallysafe", "pepper", "0123456789", 4, 1));
  QVERIFY(pwd.hexKey() == "55b5f5cdd9bf2845e339650b4f6e1398cf7fe9ceed087eb5f5bc059882723579fc8ec27443417cf33c9763bafac6277fbe991bf27dd0206e78f7d9dfd574167f");
  QVERIFY(pwd.key() == "7809");
}

void TestPBKDF2::aesKey(void)
{
  Password pwd;
  pwd.generate(PasswordParam(QByteArray(), "cttest", "pepper", "0123456789", 13, 4096));
  QVERIFY(pwd.hexKey() == "21af996b86af855aa11b832ee9dd241459334f322568a80d855091ce3a9a2e30bd4944a243451252bf43048f0ca9dc2591998ae6c6bc9e6dec00242b242dbb8d");
  static const int KeySize = 256 / 8;
  char AES[KeySize];
  pwd.extractAESKey(AES, KeySize);
  QVERIFY(QByteArray(AES, KeySize).toHex() == "21af996b86af855aa11b832ee9dd241459334f322568a80d855091ce3a9a2e30");
}
