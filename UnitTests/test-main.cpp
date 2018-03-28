/*

    Copyright (c) 2015-2018 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

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

  void pwdgen_simple_password_1(void)
  {
    DomainSettings ds;
    ds.domainName = "ct.de";
    ds.extraCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ0123456789#!\"§$%&/()[]{}=-_+*<>;:.";
    ds.iterations = 4096;
    ds.passwordTemplate = "oxxxxxxxxx";
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("test");
    QVERIFY(pwd.hexKey() == "f4d54b303b21ee3d8bff9c1eae6f66d90db58c0a5cc770eee322cc59d4dec65793bf8f5dec717fd1404bbfacf59befa68c4ad9168bfeaa6a9e28b326a76a82bb");
    QVERIFY(pwd.password() == "YBVUH=sN/3");
  }

  void pwdgen_simple_password_2(void)
  {
    DomainSettings ds;
    ds.domainName = "MyFavoriteDomain";
    ds.extraCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ";
    ds.iterations = 8192;
    ds.passwordTemplate = "oxxxxxxxxxxxxxxx";
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("foobar");
    QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
    QVERIFY(pwd.password() == "wLUwoQvKzBaYXbme");
  }

  void pwdgen_simple_password_3(void)
  {
    DomainSettings ds;
    ds.domainName = "MyFavoriteDomain";
    ds.extraCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRTUVWXYZ";
    ds.iterations = 8192;
    ds.passwordTemplate = "oxxxxxxxxxxxxxxx";
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("foobar");
    QVERIFY(pwd.hexKey() == "cb0ae7b2b7fc969770a9bfc1eef3a9afd02d2b28d6d8e9cb324f41a31392a0f800ea7e2e43e847537ceb863a16a869d5e4dd6822cf3be0206440eff97dc2001c");
    QVERIFY(pwd.password() == "wLUwoQvKzBaYXbme");
    QVERIFY(pwd.remix() == "wLUwoQvKzBaYXbme");
  }

  void pwdgen_simple_password_1_tpl(void)
  {
    DomainSettings ds;
    ds.domainName = "FooBar";
    ds.extraCharacters = "#!\"$%&/()[]{}=-_+*<>;:.";
    ds.iterations = 4096;
    ds.passwordTemplate = "xxoxAxxxxxxxxxaxx";
    ds.salt_base64 = QString("blahfasel").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("test");
    QVERIFY(pwd.hexKey() == "4e9e2503556bda7ad06cf45cab4490213becd3473845a868900fb61fa17d1c448496d11987c4446d8007562029cce7f176eda4157604012a44e42add594a524e");
    QVERIFY(pwd.password() == "pU)VUfgJ-Ws*wgzzE");
  }

  void pwdgen_simple_password_2_tpl(void)
  {
    DomainSettings ds;
    ds.domainName = "FooBar";
    ds.iterations = 8192;
    ds.passwordTemplate = "xxaxxx";
    ds.salt_base64 = QString("blahfasel").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("test");
    QVERIFY(pwd.hexKey() == "309d504d68dc921dcece9d10c14b406673715f15782032d64229b4b42336c8ec860cd9b945104824ce43720b3a088828843df4029fdb8b2314f8b5129c815949");
    QVERIFY(pwd.password() == "baeloh");
  }

  void pwdgen_simple_password_3_tpl(void)
  {
    DomainSettings ds;
    ds.domainName = "FooBar";
    ds.iterations = 8192;
    ds.passwordTemplate = "xxAxxx";
    ds.salt_base64 = QString("blahfasel").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("test");
    QVERIFY(pwd.hexKey() == "309d504d68dc921dcece9d10c14b406673715f15782032d64229b4b42336c8ec860cd9b945104824ce43720b3a088828843df4029fdb8b2314f8b5129c815949");
    QVERIFY(pwd.password() == "BAELOH");
  }

  void pwdgen_simple_password_4_tpl(void)
  {
    DomainSettings ds;
    ds.domainName = "FooBar";
    ds.iterations = 8192;
    ds.extraCharacters = "0123456789abcdef";
    ds.passwordTemplate = "xxxxxxxxxxxxxxxxxxxxxxxoxxxx";
    ds.salt_base64 = QString("SALT").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("MY_T0P_5ecr57_PA55W0RD ;-)");
    QVERIFY(pwd.hexKey() == "4993fd21600977c6f56b833eed223dda9b1bd34294afd1db4925553099cce402abda7000a22d2cfda152afcf8a3a142e55ce57a9597434a39d05ccd93a853626");
    QVERIFY(pwd.password() == "626358a39dcc50d93a4347959a75");
  }

  void pwdgen_pin(void)
  {
    DomainSettings ds;
    ds.domainName = "Bank";
    ds.extraCharacters = "0123456789";
    ds.iterations = 1;
    ds.passwordTemplate = "oxxx";
    ds.salt_base64 = QString("pepper").toUtf8().toBase64();
    Password pwd(ds);
    pwd.generate("reallysafe");
    QVERIFY(pwd.hexKey() == "55b5f5cdd9bf2845e339650b4f6e1398cf7fe9ceed087eb5f5bc059882723579fc8ec27443417cf33c9763bafac6277fbe991bf27dd0206e78f7d9dfd574167f");
    QVERIFY(pwd.password() == "7809");
  }

  void complexity(void)
  {
    for (int cv = 0; cv < Password::MaxComplexityValue; ++cv) {
      const Password::Complexity c = Password::Complexity::fromValue(cv);
      const int reCV = c.value();
      QVERIFY(cv == reCV);
    }
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
