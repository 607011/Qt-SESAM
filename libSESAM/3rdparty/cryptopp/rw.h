#ifndef CRYPTOPP_RW_H
#define CRYPTOPP_RW_H

/** \file
	This file contains classes that implement the
	Rabin-Williams signature schemes as defined in IEEE P1363.
*/

#include "cryptlib.h"
#include "integer.h"
#include "pubkey.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
class CRYPTOPP_DLL RWFunction : public TrapdoorFunction, public PublicKey
{
	typedef RWFunction ThisClass;

public:
	void Initialize(const Integer &n)
		{m_n = n;}

	void BERDecode(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	void Save(BufferedTransformation &bt) const
		{DEREncode(bt);}
	void Load(BufferedTransformation &bt)
		{BERDecode(bt);}

	Integer ApplyFunction(const Integer &x) const;
	Integer PreimageBound() const {return ++(m_n>>1);}
	Integer ImageBound() const {return m_n;}

	bool Validate(RandomNumberGenerator &rng, unsigned int level) const;
	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	void AssignFrom(const NameValuePairs &source);

	const Integer& GetModulus() const {return m_n;}
	void SetModulus(const Integer &n) {m_n = n;}

protected:
	Integer m_n;
};

//! _
class CRYPTOPP_DLL InvertibleRWFunction : public RWFunction, public TrapdoorFunctionInverse, public PrivateKey
{
	typedef InvertibleRWFunction ThisClass;

public:
	InvertibleRWFunction() : m_precompute(false) {}

	void Initialize(const Integer &n, const Integer &p, const Integer &q, const Integer &u);
	// generate a random private key
	void Initialize(RandomNumberGenerator &rng, unsigned int modulusBits)
		{GenerateRandomWithKeySize(rng, modulusBits);}

	void BERDecode(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	void Save(BufferedTransformation &bt) const
		{DEREncode(bt);}
	void Load(BufferedTransformation &bt)
		{BERDecode(bt);}

	Integer CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const;

	// GeneratibleCryptoMaterial
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const;
	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	void AssignFrom(const NameValuePairs &source);
	/*! parameters: (ModulusSize) */
	void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &alg);

	const Integer& GetPrime1() const {return m_p;}
	const Integer& GetPrime2() const {return m_q;}
	const Integer& GetMultiplicativeInverseOfPrime2ModPrime1() const {return m_u;}

	void SetPrime1(const Integer &p) {m_p = p;}
	void SetPrime2(const Integer &q) {m_q = q;}
	void SetMultiplicativeInverseOfPrime2ModPrime1(const Integer &u) {m_u = u;}

	virtual bool SupportsPrecomputation() const {return true;}
	virtual void Precompute(unsigned int unused = 0) {PrecomputeTweakedRoots();}
	virtual void Precompute(unsigned int unused = 0) const {PrecomputeTweakedRoots();}

	virtual void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
	virtual void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

protected:
	void PrecomputeTweakedRoots() const;

protected:
	Integer m_p, m_q, m_u;

	mutable Integer m_pre_2_9p, m_pre_2_3q, m_pre_q_p;
	mutable bool m_precompute;
};

//! RW
struct RW
{
	static std::string StaticAlgorithmName() {return "RW";}
	typedef RWFunction PublicKey;
	typedef InvertibleRWFunction PrivateKey;
};

//! RWSS
template <class STANDARD, class H>
struct RWSS : public TF_SS<STANDARD, H, RW>
{
};

NAMESPACE_END

#endif
