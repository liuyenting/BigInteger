#ifndef BIGINTEGER_H
#define BIGINTEGER_H

#include <iostream>
#include <string>
#include <vector>

class BigInteger
{
	//
	// custom types
	//
private:
	enum Sign { POSITIVE = 1, ZERO = 0, NEGATIVE = -1 };
	enum Compare { GREATER, EQUAL, LESS };
public:
	typedef unsigned int BaseType;
	static const unsigned int Base = 10000;
	// magnitude of the Base value, currently hard coded
	static const unsigned int BaseMagnitude10 = 4;

	//
	// actual functions
	//
protected:
	Sign sign;
	std::vector<BaseType> storage;
	int getPreciseMagnitude() const;
public:
	BigInteger();
	BigInteger(const int&);
	BigInteger(const std::string&);
	BigInteger(const BigInteger&);
	BigInteger(const std::vector<BigInteger::BaseType>&);


	// unary operator
	void operator - ();
	BigInteger operator - () const;

	void operator ++ ();
	void operator ++ (int);
	void operator -- ();
	void operator -- (int);

	// binary operator: arithmetic
	const BigInteger operator + (const BigInteger&) const;
	const BigInteger operator - (const BigInteger&) const;
	const BigInteger operator * (const BigInteger&) const;
	const BigInteger operator / (const BigInteger&) const;
	const BigInteger operator % (const BigInteger&) const;

	// binary operator: arithmetic (continue)
	BigInteger& operator += (const BigInteger&);
	BigInteger& operator -= (const BigInteger&);
	BigInteger& operator *= (const BigInteger&);
	BigInteger& operator *= (const int&);
	BigInteger& operator /= (const BigInteger&);
	BigInteger& operator /= (const int&);
	BigInteger& operator %= (const BigInteger&);

	// binary operator: comparison
	bool operator > (const BigInteger&) const;
	bool operator == (const BigInteger&) const;
	bool operator < (const BigInteger&) const;
	bool operator >= (const BigInteger&) const;
	bool operator != (const BigInteger&) const;
	bool operator <= (const BigInteger&) const;


	// binary operator: stream and memroy operation
	BigInteger& operator = (const BigInteger&);
	BigInteger& operator = (const int&);
	friend std::ostream& operator << (std::ostream&, const BigInteger&);

	bool iseven();
	bool iszero() const;

	//
	// support functions
	//
private:
	void add(const BigInteger&, const BigInteger&);
	void subtract(const BigInteger&, const BigInteger&);
	void multiply(const BigInteger&, const BigInteger&);
	void divide(const BigInteger&, const BigInteger&);
	void modulus(const BigInteger&, const BigInteger&);

	void karatsuba(const BigInteger&, const BigInteger&);

	Compare compare(const BigInteger&, const BigInteger&) const;
	Compare compareMagnitude(const BigInteger&, const BigInteger&) const;

	bool isZero() const;

	void removeTrailingZeros();
};

static BigInteger CONSTANT_10(10);
static BigInteger CONSTANT_1(1);
static BigInteger CONSTANT_0(0);

#endif
