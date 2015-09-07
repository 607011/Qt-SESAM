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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: HashMaster filename" << std::endl;
    return 1;
  }

  QFileInfo fi(argv[1]);
  if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
    std::cerr << "ERROR: " << fi.fileName().toStdString() << " does not exist or is not a readable file." << std::endl;
    return 2;
  }

  QFile f(argv[1]);
  if (!f.open(QIODevice::ReadOnly)) {
    std::cerr << "ERROR: Cannot read " << fi.fileName().toStdString() << "." << std::endl;
    return 3;
  }

  bool ok = false;
  std::cout << "*" << fi.fileName().toStdString() << std::endl;

  QCryptographicHash md4(QCryptographicHash::Md4);
  ok = md4.addData(&f);
  if (ok) {
    std::cout << "MD4:    " << md4.result().toHex().toStdString() << std::endl;
    f.reset();
  }

  QCryptographicHash md5(QCryptographicHash::Md5);
  ok = md5.addData(&f);
  if (ok) {
    std::cout << "MD5:    " << md5.result().toHex().toStdString() << std::endl;
    f.reset();
  }

  QCryptographicHash sha1(QCryptographicHash::Sha1);
  ok = sha1.addData(&f);
  if (ok) {
    std::cout << "SHA1:   " << sha1.result().toHex().toStdString() << std::endl;
    f.reset();
  }

  QCryptographicHash sha256(QCryptographicHash::Sha256);
  ok = sha256.addData(&f);
  if (ok) {
    std::cout << "SHA256: " << sha256.result().toHex().toStdString() << std::endl;
    f.reset();
  }

  QCryptographicHash sha512(QCryptographicHash::Sha512);
  ok = sha512.addData(&f);
  if (ok) {
    std::cout << "SHA512: " << sha512.result().toHex().toStdString() << std::endl;
    f.reset();
  }

  f.close();
  return 0;
}
