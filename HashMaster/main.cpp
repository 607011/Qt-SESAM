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

#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "ripemd.h"
#include "hex.h"
#include "sha.h"
#include "ccm.h"
#include "misc.h"
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "md4.h"
#include "md5.h"

#ifdef WIN32
#include <windows.h>
#endif

struct MatchPathSeparator {
  bool operator()(char ch) const {
    return ch == '\\';
  }
};


std::string basename(const std::string &pathname)
{
  return std::string(std::find_if(pathname.rbegin(), pathname.rend(), MatchPathSeparator()).base(), pathname.end());
}


int main(int argc, char *argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: HashMaster <filename>" << std::endl;
    return 1;
  }

  auto processFile = [](std::string fname)
  {
    int rc = 0;
    std::string fname_base = basename(fname);
    std::ifstream file(fname, std::ios::binary);
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::ofstream out(fname + ".txt");
    if (out.is_open()) {
      out << "*" << fname_base << std::endl;

      auto dump = [&out](std::string algo, byte *digest, size_t digestLen)
      {
        CryptoPP::HexEncoder encoder(nullptr, false);
        std::string hexDigest;
        encoder.Attach(new CryptoPP::StringSink(hexDigest));
        encoder.Put(digest, digestLen);
        encoder.MessageEnd();
        std::cout << algo << " " << hexDigest << std::endl;
        if (out.is_open())
          out << algo << " " << hexDigest << std::endl;
      };

      std::cout << "*" << fname_base << std::endl;
      byte *buffer = new byte[(unsigned int)size];
      if (file.read(reinterpret_cast<char*>(buffer), size)) {

        {
          byte digest[CryptoPP::Weak::MD4::DIGESTSIZE];
          CryptoPP::Weak::MD4 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("MD4:      ", digest, CryptoPP::Weak::MD4::DIGESTSIZE);
        }

        {
          byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
          CryptoPP::Weak::MD5 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("MD5:      ", digest, CryptoPP::Weak::MD5::DIGESTSIZE);
        }

        {
          byte digest[CryptoPP::SHA1::DIGESTSIZE];
          CryptoPP::SHA1 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("SHA1:     ", digest, CryptoPP::SHA1::DIGESTSIZE);
        }

        {
          byte digest[CryptoPP::SHA256::DIGESTSIZE];
          CryptoPP::SHA256 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("SHA256:   ", digest, CryptoPP::SHA256::DIGESTSIZE);
        }

        {
          byte digest[CryptoPP::SHA512::DIGESTSIZE];
          CryptoPP::SHA512 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("SHA512:   ", digest, CryptoPP::SHA512::DIGESTSIZE);
        }

        {
          byte digest[CryptoPP::RIPEMD160::DIGESTSIZE];
          CryptoPP::RIPEMD160 hash;
          hash.CalculateDigest(digest, buffer, (size_t)size);
          dump("RIPEMD160:", digest, CryptoPP::RIPEMD160::DIGESTSIZE);
        }

      }
      else {
        rc = 2;
      }

      delete[] buffer;
      out.close();
    }
    else {
      rc = 3;
    }

    file.close();
    return rc;
  };

  std::list<std::string> fileList;
  const std::string fname(argv[1]);
  WIN32_FIND_DATAA findData;
  HANDLE ff = FindFirstFileA(fname.c_str(), &findData);
  if (ff != INVALID_HANDLE_VALUE) {
    if (findData.cFileName != nullptr) {
      fileList.push_back(findData.cFileName);
    }
    while (FindNextFileA(ff, &findData)) {
      if (findData.cFileName != nullptr)
        fileList.push_back(findData.cFileName);
    }
  }

  int rc = 0;
  for (std::list<std::string>::const_iterator i = fileList.cbegin(); i != fileList.cend(); ++i) {
    rc = processFile(*i);
    if (rc != 0)
      break;
  }

  return rc;
}
