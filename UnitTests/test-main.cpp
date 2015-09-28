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


#include "pbkdf2.h"
#include "password.h"
#include "crypter.h"
#include "exporter.h"
#include "domainsettings.h"

#include <QDebug>
#include <QDir>
#include <QMessageAuthenticationCode>
#include <QtTest/QTest>


class TestSESAM : public QObject
{
  Q_OBJECT
private slots:
  void qt_hmac_1(void)
  {
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha512, "S3cr3t");
    hmac.addData("foo");
    QVERIFY(hmac.result() == QByteArray::fromHex("5576f3f707395cf33509379094424ff0743c990bf6da8b705baedb3aa25870a27069b177acbfe2bbd6780c6f7c8f735b0b4dcfcad3257a95b3f46c8b61435a3a"));
  }

  void qt_hmac_2(void)
  {
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha512, "secret");
    hmac.addData("message");
    QVERIFY(hmac.result() == QByteArray::fromHex("1bba587c730eedba31f53abb0b6ca589e09de4e894ee455e6140807399759adaafa069eec7c01647bb173dcb17f55d22af49a18071b748c5c2edd7f7a829c632"));
  }

  void qt_hmac_empty_message(void)
  {
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha512, "secret");
    QVERIFY(hmac.result() == QByteArray::fromHex("b0e9650c5faf9cd8ae02276671545424104589b3656731ec193b25d01b07561c27637c2d4d68389d6cf5007a8632c26ec89ba80a01c77a6cdd389ec28db43901"));
  }

  void qt_hmac_empty_key(void)
  {
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha512, "");
    hmac.addData("message");
    QVERIFY(hmac.result() == QByteArray::fromHex("08fce52f6395d59c2a3fb8abb281d74ad6f112b9a9c787bcea290d94dadbc82b2ca3e5e12bf2277c7fedbb0154d5493e41bb7459f63c8e39554ea3651b812492"));
  }

  void base64(void)
  {
    QVERIFY(QByteArray::fromBase64(DomainSettings::DefaultSalt_base64) == "pepper");
    QVERIFY(DomainSettings::DefaultSalt_base64 == QString("pepper").toUtf8().toBase64());
  }

  void pbkdf2(void)
  {
    PBKDF2 pbkdf2(QString("message").toUtf8(), QString("pepper").toUtf8(), 3, QCryptographicHash::Sha512);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("2646f9ccb58d21406815bafc62245771bf80aaa080a633ff1bdd660eb44f369a89da48fb041c5551a118de20cfb8b96b92e7a9945425ba889e9ad645614522eb"));
  }

  void pbkdf2_empty_salt(void)
  {
    PBKDF2 pbkdf2(QString("message").toUtf8(), QByteArray(), 3, QCryptographicHash::Sha512);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("b8ec13cfc9b9d49ca1143018ce8413a962c09c0063f30a466df802897475c57f268d91cc568ac1b6a9f19b1a0db10f30058fb7a453b2675010ef2b5f96487ad3"));
  }

  void pbkdf2_empty_message(void)
  {
    PBKDF2 pbkdf2(SecureByteArray(), QString("pepper").toUtf8(), 3, QCryptographicHash::Sha512);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("9dd331fc67421e1dce619cbbb517170e2dc325491d3426425630c4c01fd0eca8d8f535d6b0555a2aa43efbc9141e3dd7edaef8b1278ac34eabfc2db735d992ee"));
  }

  void pbkdf2_long_message(void)
  {
    PBKDF2 pbkdf2(QString("ThisMessageIsLongerThanSixtyFourCharactersWhichLeadsToTheSituationThatTheMessageHasToBeHashedWhenCalculatingTheHmac").toUtf8(), QString("pepper").toUtf8(), 3, QCryptographicHash::Sha512);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("efc8e734ed5b5657ac220046754b7d1dbea00983f13209b1ec1d0e418e98807cba1026d3ed3fa2a09dfa43c074447bf4777e70e4999d29d2c2f84dc51502a195"));
  }

  void pbkdf2_sha384(void)
  {
    PBKDF2 pbkdf2(QString("message").toUtf8(), QString("salt").toUtf8(), 3, QCryptographicHash::Sha384);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("dcbeb0b99a4cf4d1c9c1e8f630f3aa8637c8906f1c3e1c78fb4f462b160df20f7435bdd6a904dd3c3ede7ff04bc53e90"));
  }

  void pbkdf2_sha256(void)
  {
    PBKDF2 pbkdf2(QString("message").toUtf8(), QString("salt").toUtf8(), 3, QCryptographicHash::Sha256);
    QVERIFY(pbkdf2.derivedKey() == QByteArray::fromHex("db78c5091444940f9642fce519097ee7adfeb338fd6970855135539020b53fad"));
  }

  void pwdgen_simple_1(void)
  {
    DomainSettings ds;
    ds.domainName = "ct.de";
    ds.usedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.";
    ds.iterations = 4096;
    ds.length = 10;
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("test");
    QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
    QVERIFY(pwd.key() == "YBVUH=sN/3");
  }

  void pwdgen_simple_password_2(void)
  {
    DomainSettings ds;
    ds.domainName = "MyFavoriteDomain";
    ds.usedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ";
    ds.iterations = 8192;
    ds.length = 16;
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("foobar");
    QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
    QVERIFY(pwd.key() == "wLUwoQvKzBaYXbme");
  }

  void pwdgen_pin(void)
  {
    DomainSettings ds;
    ds.domainName = "Bank";
    ds.usedCharacters = "0123456789";
    ds.iterations = 1;
    ds.length = 4;
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("reallysafe");
    QVERIFY(pwd.hexKey() == "55b5f5cdd9bf2845e339650b4f6e1398cf7fe9ceed087eb5f5bc059882723579fc8ec27443417cf33c9763bafac6277fbe991bf27dd0206e78f7d9dfd574167f");
    QVERIFY(pwd.key() == "7809");
  }

  void pwdgen_binary_salt_and_password(void)
  {
    DomainSettings ds;
    ds.domainName = "Foo";
    ds.usedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ";
    ds.iterations = 1024;
    ds.length = 32;
    static uchar salt[256] = { 219, 234, 34, 57, 151, 182, 99, 40, 180, 243, 133, 33, 191, 250, 4, 226, 107, 19, 132, 160, 29, 73, 184, 194, 38, 109, 104, 126, 43, 108, 215, 45, 253, 44, 204, 83, 229, 220, 146, 136, 110, 0, 232, 25, 212, 23, 205, 157, 91, 179, 200, 248, 102, 17, 101, 164, 113, 130, 27, 141, 79, 177, 47, 123, 36, 210, 183, 65, 64, 140, 72, 134, 207, 100, 233, 35, 211, 131, 75, 186, 70, 156, 105, 21, 66, 216, 241, 169, 15, 144, 111, 122, 181, 135, 118, 39, 228, 37, 188, 26, 14, 147, 49, 95, 2, 244, 119, 154, 171, 218, 77, 187, 3, 127, 217, 22, 129, 69, 185, 206, 152, 5, 115, 106, 6, 96, 227, 255, 125, 155, 18, 240, 238, 55, 32, 128, 84, 89, 252, 172, 28, 67, 225, 214, 121, 237, 93, 148, 59, 139, 87, 97, 158, 199, 98, 94, 68, 247, 41, 86, 13, 142, 196, 197, 195, 82, 78, 116, 60, 209, 7, 9, 174, 189, 150, 221, 175, 224, 213, 145, 92, 231, 170, 208, 81, 254, 76, 246, 198, 71, 203, 192, 124, 167, 137, 222, 31, 249, 58, 190, 11, 20, 10, 168, 46, 235, 88, 245, 114, 74, 176, 163, 1, 223, 8, 193, 30, 54, 166, 52, 24, 85, 90, 162, 117, 236, 120, 178, 143, 63, 159, 173, 161, 61, 48, 42, 103, 80, 251, 56, 202, 53, 153, 201, 51, 230, 242, 112, 149, 239, 138, 165, 16, 62, 12, 50 };
    static uchar password[256] = { 136, 152, 37, 207, 212, 209, 100, 16, 137, 118, 4, 205, 115, 26, 21, 19, 99, 30, 156, 31, 102, 35, 130, 114, 22, 191, 32, 248, 155, 60, 120, 44, 167, 107, 197, 123, 27, 138, 171, 133, 186, 69, 122, 58, 70, 147, 51, 62, 164, 236, 61, 81, 72, 3, 218, 11, 134, 24, 230, 227, 54, 43, 124, 168, 113, 84, 144, 217, 9, 184, 163, 64, 195, 34, 173, 5, 8, 192, 91, 63, 40, 12, 6, 140, 74, 125, 52, 196, 110, 46, 229, 198, 220, 247, 131, 202, 177, 216, 246, 106, 161, 219, 146, 41, 226, 154, 28, 148, 117, 238, 18, 92, 135, 231, 180, 75, 211, 141, 94, 67, 185, 160, 23, 105, 187, 47, 53, 253, 95, 232, 242, 169, 240, 181, 172, 139, 116, 1, 179, 178, 80, 143, 20, 7, 243, 101, 252, 176, 228, 48, 224, 0, 206, 50, 249, 104, 89, 88, 199, 78, 17, 33, 200, 194, 132, 83, 97, 57, 56, 222, 183, 214, 68, 250, 73, 108, 237, 71, 29, 221, 251, 98, 204, 45, 245, 121, 255, 193, 182, 79, 14, 254, 190, 15, 153, 127, 215, 119, 111, 2, 225, 90, 223, 233, 213, 36, 151, 59, 210, 49, 159, 241, 10, 86, 188, 162, 109, 65, 42, 235, 103, 128, 234, 157, 189, 77, 201, 25, 82, 166, 203, 239, 96, 38, 244, 175, 39, 142, 112, 170, 55, 93, 165, 76, 13, 208, 150, 126, 85, 66, 158, 87, 145, 129, 174, 149 };
    ds.salt_base64 = QByteArray(reinterpret_cast<char*>(salt), 256).toBase64();
    Password pwd(ds);
    pwd.generate(SecureByteArray(reinterpret_cast<char*>(password), 256));
    QVERIFY(pwd.hexKey() == "e217c512b32d61f94bccad89b9c79012d073f4c0960854803a6115aa928f5b823d3bcd167872d4df102450f4dc26d82c6fa6666f749f82b2ec12593edb6ba2b0");
    QVERIFY(pwd.key() == "XQCjlTFKFWBAsmgYgzTwJdbFPjCyykCl");
  }

  void crypter_encrypt_decrypt_no_padding(void)
  {
    SecureByteArray key = Crypter::randomBytes(Crypter::AESKeySize);
    SecureByteArray IV = Crypter::randomBytes(Crypter::AESBlockSize);
    QByteArray data = QByteArray(1024 * Crypter::AESBlockSize, 'A');
    QByteArray cipher = Crypter::encrypt(key, IV, data, CryptoPP::StreamTransformationFilter::NO_PADDING);
    QByteArray plain = Crypter::decrypt(key, IV, cipher, CryptoPP::StreamTransformationFilter::NO_PADDING);
    QVERIFY(plain.size() == data.size());
    QVERIFY(plain == data);
  }

  void crypter_encrypt_decrypt_pkcs_padding(void)
  {
    static const int nExtra = 3;
    SecureByteArray key = Crypter::randomBytes(Crypter::AESKeySize);
    SecureByteArray IV = Crypter::randomBytes(Crypter::AESBlockSize);
    QByteArray data = QByteArray(7 * Crypter::AESBlockSize + nExtra, 'B');
    QByteArray cipher = Crypter::encrypt(key, IV, data, CryptoPP::StreamTransformationFilter::PKCS_PADDING);
    QByteArray plain = Crypter::decrypt(key, IV, cipher, CryptoPP::StreamTransformationFilter::PKCS_PADDING);
    QVERIFY(plain.size() == data.size());
    QVERIFY(plain == data);
  }

  void crypter_make_key(void)
  {
    SecureByteArray masterPassword = QString("7h15p455w0rd15m0r37h4n53cr37").toUtf8();
    QByteArray salt = QString("pepper").toUtf8();
    SecureByteArray key = Crypter::makeKeyFromPassword(masterPassword, salt);
    QVERIFY(key.size() == Crypter::AESKeySize);
    QVERIFY(key.toBase64() == "T8LlrPN1aqDlsIJVFA19r0RlZRpj7LuY8xbFnk3Tx8M=");
  }

  void crypter_make_key_iv(void)
  {
    SecureByteArray masterPassword = QString("7h15p455w0rd15m0r37h4n53cr37").toUtf8();
    QByteArray salt = QString("this is my salt.").toUtf8();
    SecureByteArray key;
    SecureByteArray IV;
    Crypter::makeKeyAndIVFromPassword(masterPassword, salt, key, IV);
    QVERIFY(key.length() == Crypter::AESKeySize);
    QVERIFY(key.toBase64() == "vGoCE/dCUIpQFfPEHnh+qY9HLTeMNPFPMlA9dz5snjs=");
    QVERIFY(IV.length() == Crypter::AESBlockSize);
    QVERIFY(IV.toBase64() == "mHg3BMpk9vwK+eY1YJWAKg==");
  }

  void crypter_encode_decode(void)
  {
    SecureByteArray masterPassword = QString("7h15p455w0rd15m0r37h4n53cr37").toUtf8();
    QByteArray salt = Crypter::generateSalt();
    SecureByteArray key;
    SecureByteArray IV;
    Crypter::makeKeyAndIVFromPassword(masterPassword, salt, key, IV);
    QVERIFY(key.length() == Crypter::AESKeySize);
    QVERIFY(IV.length() == Crypter::AESBlockSize);

    SecureByteArray KGK = Crypter::generateKGK();
    QByteArray data = Crypter::randomBytes(1024);
    QByteArray cipher = Crypter::encode(key, IV, salt, KGK, data, true);

    SecureByteArray KGK2;
    QByteArray plain = Crypter::decode(masterPassword, cipher, true, KGK2);
    QVERIFY(plain == data);
    QVERIFY(KGK == KGK2);
  }

  void export_import(void)
  {
    QString filename = QDir::tempPath() + "/qt-sesam-unit-test.pem";
    SecureString pwd("wonderful password");
    Exporter ex(filename);
    const SecureByteArray &original = Crypter::generateKGK();
    ex.write(original, pwd);
    const SecureByteArray &recovered = ex.read(pwd);
    QVERIFY(original.size() == recovered.size());
    QVERIFY(original == recovered);
  }
};

QTEST_GUILESS_MAIN(TestSESAM)
#include "test-main.moc"
