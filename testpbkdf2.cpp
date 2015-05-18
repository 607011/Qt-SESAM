#include "testpbkdf2.h"
#include "password.h"

TestPBKDF2::TestPBKDF2(QObject *parent)
  : QObject(parent)
{
  // ...
}

void TestPBKDF2::simple1(void)
{
  Password pwd;
  pwd.generate(PasswordParam("ct.de", "pepper", "test", "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.", 10, 4096));
  QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
  QVERIFY(pwd.key() == "YBVUH=sN/3");
}

void TestPBKDF2::simple2(void)
{
  Password pwd;
  pwd.generate(PasswordParam("MyFavoriteDomain", "pepper", "foobar", "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ", 16, 8192));
  QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
  QVERIFY(pwd.key() == "wLUwoQvKzBaYXbme");
}

void TestPBKDF2::pin(void)
{
  Password pwd;
  pwd.generate(PasswordParam("Bank", "pepper", "reallysafe", "0123456789", 4, 1));
  QVERIFY(pwd.hexKey() == "55b5f5cdd9bf2845e339650b4f6e1398cf7fe9ceed087eb5f5bc059882723579fc8ec27443417cf33c9763bafac6277fbe991bf27dd0206e78f7d9dfd574167f");
  QVERIFY(pwd.key() == "7809");
}
