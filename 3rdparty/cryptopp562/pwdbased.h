// pwdbased.h - written and placed in the public domain by Wei Dai
// removed unnecessary code (ola@ct.de)

#ifndef CRYPTOPP_PWDBASED_H
#define CRYPTOPP_PWDBASED_H

#include "cryptlib.h"
#include "hmac.h"
#include "integer.h"

NAMESPACE_BEGIN(CryptoPP)

//! abstract base class for password based key derivation function
class PasswordBasedKeyDerivationFunction
{
public:
	virtual size_t MaxDerivedKeyLength() const =0;
	virtual bool UsesPurposeByte() const =0;
	//! derive key from password
	/*! If timeInSeconds != 0, will iterate until time elapsed, as measured by ThreadUserTimer
		Returns actual iteration count, which is equal to iterations if timeInSeconds == 0, and not less than iterations otherwise. */
  virtual unsigned int DeriveKey(byte *derived, size_t derivedLen, const byte *password, size_t passwordLen, const byte *salt, size_t saltLen, unsigned int iterations, bool &doQuit) const = 0;
};

//! PBKDF2 from PKCS #5, T should be a HashTransformation class
template <class T>
class PKCS5_PBKDF2_HMAC : public PasswordBasedKeyDerivationFunction
{
public:
	size_t MaxDerivedKeyLength() const {return 0xffffffffU;}	// should multiply by T::DIGESTSIZE, but gets overflow that way
	bool UsesPurposeByte() const {return false;}
  unsigned int DeriveKey(byte *derived, size_t derivedLen, const byte *password, size_t passwordLen, const byte *salt, size_t saltLen, unsigned int iterations, bool &doQuit) const;
};

template <class T>
unsigned int PKCS5_PBKDF2_HMAC<T>::DeriveKey(byte *derived, size_t derivedLen, const byte *password, size_t passwordLen, const byte *salt, size_t saltLen, unsigned int iterations, bool &doQuit) const
{
	assert(derivedLen <= MaxDerivedKeyLength());
  assert(iterations > 0);

  if (iterations == 0)
		iterations = 1;

	HMAC<T> hmac(password, passwordLen);
	SecByteBlock buffer(hmac.DigestSize());

  unsigned int i = 1;
  while (derivedLen > 0 && !doQuit) {
		hmac.Update(salt, saltLen);
    for (unsigned int j = 0; j < 4; ++j) {
			byte b = byte(i >> ((3-j)*8));
			hmac.Update(&b, 1);
		}
		hmac.Final(buffer);

		size_t segmentLen = STDMIN(derivedLen, buffer.size());
		memcpy(derived, buffer, segmentLen);

    for (unsigned int j = 1; j < iterations; ++j) {
			hmac.CalculateDigest(buffer, buffer, buffer.size());
			xorbuf(derived, buffer, segmentLen);
		}

		derived += segmentLen;
		derivedLen -= segmentLen;
    ++i;
	}

  return doQuit ? 0 : iterations;
}


NAMESPACE_END

#endif
