#include "testpbkdf2.h"
#include "passwordgenerator.h"

TestPBKDF2::TestPBKDF2(QObject *parent)
  : QObject(parent)
{
  // ...
}

void TestPBKDF2::simple(void)
{
  PasswordGenerator generator;
  QString key;
  QByteArray hexKey;
  bool doQuit = false;
  qreal elapsed = 0;
  generator.generate("ct.de", "pepper", "test", "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.", 10, 4096, doQuit, elapsed, key, hexKey);
  QVERIFY(hexKey == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
  QVERIFY(key == "YBVUH=sN/3");
}
