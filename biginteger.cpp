#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <iomanip>

#include "biginteger.h"

//
// actual functions
//
BigInteger::BigInteger()
{
	BigInteger(0);
}

BigInteger::BigInteger(const int& input)
{
	int temp = input;

	// register the sign of default value
	if(temp == 0)
		sign = ZERO;
	else if(temp > 0)
		sign = POSITIVE;
	else
		sign = NEGATIVE;

	while(temp != 0)
	{
		storage.push_back(temp%BigInteger::Base);
		temp /= BigInteger::Base;
	}
}

BigInteger::BigInteger(const std::string& input)
{
	std::string::const_reverse_iterator iterator = input.rbegin(), lastPosition = input.rend();

	// parse the sign from string
	switch(input[0])
	{
		case '-':
			sign = NEGATIVE;
			lastPosition -= 1;
			break;
		case '0':
			if(input.length() == 1)
			{
				sign = ZERO;
				return;
			}
		default:
			sign = POSITIVE;
	}

	// iterate through the characters
	int counter = 0, newgroup = 0, buffer = 0;
	for(iterator = input.rbegin(); iterator != lastPosition; ++iterator)
	{
		// skip the null character
		if(*iterator == '\0')
			continue;

		// push the new group if counter full
		if(counter == BaseMagnitude10)
		{
			storage.push_back(newgroup);
			counter = newgroup = 0;
		}

		if(!std::isdigit(*iterator))
			throw "BigInteger::BigInteger(const std::string&) -> not a digit";

		buffer = (int)(*iterator-'0');
		for(int i=0; i<counter; i++)
			buffer *= 10;
		newgroup += buffer;

		// increment counter to advance digits in the group
		counter++;
	}

	// push the remaining result into the storage
	if(newgroup != 0)
		storage.push_back(newgroup);
}

BigInteger::BigInteger(const BigInteger& input)
{
	operator = (input);
}

BigInteger::BigInteger(const std::vector<BigInteger::BaseType>& input)
{
	std::cout << "received a vector with size " << input.size() << std::endl;

	// assume the sign is positive
	sign = BigInteger::POSITIVE;

	// direct copy the vector
	std::vector<BigInteger::BaseType>::const_iterator iterator;
	for(iterator = input.begin(); iterator != input.end(); ++iterator)
		storage.push_back(*iterator);

	std::cout << "copy complete" << std::endl;
	std::cout << "current storage size " << storage.size() << std::endl;

	operator << (std::cout, *this);
}

// unary operator
inline BigInteger BigInteger::operator - () const
{
	BigInteger result = *this;

	if(result.sign == POSITIVE)
		result.sign = NEGATIVE;
	else if(result.sign == NEGATIVE)
		result.sign = POSITIVE;

	return result;
}

// binary operator: arithmetic
inline BigInteger BigInteger::operator + (const BigInteger& rhs) const
{
	BigInteger result;
	result.add(*this, rhs);
	return result;
}

inline BigInteger& BigInteger::operator ++ ()
{
	add(*this, CONSTANT_1);
	return *this;
}

inline BigInteger BigInteger::operator ++ (int)
{
	BigInteger result(*this);
	operator ++ ();
	return result;
}
	
inline BigInteger& BigInteger::operator -- ()
{
	subtract(*this, CONSTANT_1);
	return *this;
}

inline BigInteger BigInteger::operator -- (int)
{
	BigInteger result(*this);
	operator -- ();
	return result;
}

inline BigInteger BigInteger::operator - (const BigInteger& rhs) const
{
	BigInteger result;
	result.subtract(*this, rhs);
	return result;
}

inline BigInteger BigInteger::operator * (const BigInteger& rhs) const
{
	BigInteger result;
	result.multiply(*this, rhs);
	return result;
}

inline BigInteger BigInteger::operator / (const BigInteger& rhs) const
{
	BigInteger result;
	result.divide(*this, rhs);
	return result;
}

inline BigInteger BigInteger::operator % (const BigInteger& rhs) const
{
	BigInteger result;
	result.modulus(*this, rhs);
	return result;
}

// binary operator: arithmetic (continue)
inline void BigInteger::operator += (const BigInteger& rhs)
{
	add(*this, rhs);
}

inline void BigInteger::operator -= (const BigInteger& rhs)
{
	subtract(*this, rhs);
}	
inline void BigInteger::operator *= (const BigInteger& rhs) 
{
	multiply(*this, rhs);
}

inline void BigInteger::operator /= (const BigInteger& rhs) 
{
	divide(*this, rhs);
}

inline void BigInteger::operator %= (const BigInteger& rhs)
{
	modulus(*this, rhs);
}

// binary operator: comparison
bool BigInteger::operator > (const BigInteger& rhs) const
{
	return compare(*this, rhs) == BigInteger::GREATER;
}

bool BigInteger::operator == (const BigInteger& rhs) const
{
	return compare(*this, rhs) == BigInteger::EQUAL;
}

bool BigInteger::operator < (const BigInteger& rhs) const
{
	return compare(*this, rhs) == BigInteger::LESS;
}

bool BigInteger::operator >= (const BigInteger& rhs) const
{
	return operator > (rhs) || operator == (rhs);
}

bool BigInteger::operator != (const BigInteger& rhs) const
{
	return !operator == (rhs);
}

bool BigInteger::operator <= (const BigInteger& rhs) const
{
	return operator < (rhs) || operator == (rhs);
}

// binary operator: stream and memroy operation
void BigInteger::operator = (const BigInteger& rhs)
{
	// copy sign
	sign = rhs.sign;

	// wipe the storage
	storage.clear();

	// reserve the memory space for faster copy speed
	storage.reserve(rhs.storage.size());

	// start performing deep copy
	std::vector<BigInteger::BaseType>::const_iterator iterator;
	for(iterator = rhs.storage.begin(); iterator != rhs.storage.end(); ++iterator)
		storage.push_back(*iterator);
}

std::ostream& operator << (std::ostream& stream, const BigInteger& rhs)
{
	switch(rhs.sign)
	{
		case BigInteger::NEGATIVE:
			stream << '-';
			break;
		case BigInteger::ZERO:
			stream << '0';
			return stream;
		#ifdef FORCE_SHOW_POSITIVE
		case BigInteger::POSITIVE:
			stream << '+';
			break;
		#endif
	}

	// print the first group without padding
	std::cout << rhs.storage.back();

	// reverse iterate the groups and print them out
	std::vector<BigInteger::BaseType>::const_reverse_iterator iterator = rhs.storage.rbegin();
	for(iterator++; iterator != rhs.storage.rend(); ++iterator)
		stream << std::setfill('0') << std::setw(BigInteger::BaseMagnitude10) << *iterator;

	return stream;
}

//
// support functions
//
void BigInteger::add(const BigInteger& lhs, const BigInteger& rhs)
{
	if(lhs.sign == BigInteger::ZERO)
	{
		operator = (rhs);
		return;
	}
	else if(rhs.sign == ZERO)
	{
		operator = (lhs);
		return;
	}

	// rearrange according to the magnitude
	const BigInteger *lh_obj, *rh_obj;
	if(compareMagnitude(lhs, rhs) == LESS)
	{
		lh_obj = &rhs;
		rh_obj = &lhs;
	}
	else
	{
		lh_obj = &lhs;
		rh_obj = &rhs;
	}
	
	if(lhs.sign == rhs.sign)
	{
		// duplicate the longer one
		operator = (*lh_obj);

		BaseType carry = 0, buffer;
		// iterate through rh_obj, add with lh_obj, and store into *this
		for(std::vector<int>::size_type index = 0; index<rh_obj->storage.size() || carry!=0; index++) 
		{
			// add the carry
			buffer = lh_obj->storage[index] + carry;

			// perform the actual addition
			if(index<rh_obj->storage.size())
				buffer += rh_obj->storage[index];

			// wrap the digit
			carry = buffer/BigInteger::Base;
			buffer %= BigInteger::Base;

			// store back
			storage[index] = buffer;
		}
	}
	else
	{
		if(lh_obj->sign == POSITIVE)
		{
			// +LARGE -SMALL
			subtract(*lh_obj, -(*rh_obj));
		}
		else
		{
			// -LARGE +SMALL

			// subtract as +LARGE +SMALL
			subtract(-(*lh_obj), *rh_obj);
			*this = -(*this);
		}
	}
}

void BigInteger::subtract(const BigInteger& lhs, const BigInteger& rhs)
{
	if(lhs.sign == ZERO)
	{
		operator = (rhs);
		*this = -(*this);
		return;
	}
	else if(rhs.sign == ZERO)
	{
		operator = (lhs);
		return;
	}

	// rearrange according to the magnitude
	const BigInteger *lh_obj, *rh_obj;
	if(compareMagnitude(lhs, rhs) == LESS)
	{
		lh_obj = &rhs;
		rh_obj = &lhs;
	}
	else
	{
		lh_obj = &lhs;
		rh_obj = &rhs;
	}
	
	if(lhs.sign == rhs.sign)
	{
		// duplicate the longer one
		operator = (*lh_obj);

		BaseType carry = 0, buffer;
		bool needCarry;
		// iterate through rh_obj, add with lh_obj, and store into *this
		for(std::vector<int>::size_type index = 0; index<rh_obj->storage.size() || carry!=0; index++) 
		{
			buffer = lh_obj->storage[index];

			needCarry = false;

			// perform the actual subtraction
			if(index < rh_obj->storage.size())
			{
				// wrap first, since base type is unsigned
				if(lh_obj->storage[index] < rh_obj->storage[index])
				{
					buffer += Base;
					needCarry = true;
				}

				buffer -= rh_obj->storage[index] + carry;
			}
			else
			{
				if(lh_obj->storage[index] == 0)
				{
					buffer += Base;
					needCarry = true;
				}

				buffer -= carry;
			}
				
			// store back
			storage[index] = buffer;

			// refresh the carry
			carry = (needCarry)?1:0;
		}
	}
	else
	{
		if(lh_obj->sign == BigInteger::POSITIVE)
		{
			// +LARGE -SMALL
			add(*lh_obj, -(*rh_obj));
		}
		else
		{
			// -LARGE +SMALL

			// add as +LARGE +SMALL
			add(-(*lh_obj), *rh_obj);
			*this = -(*this);
		}
	}

	// negate the result when lhs and rhs are swapped
	if(lh_obj != &lhs)
		*this = -(*this);

	removeTrailingZeros();
}

void BigInteger::multiply(const BigInteger& lhs, const BigInteger& rhs)
{
	// empty the storage for new value
	storage.empty();

	// set as 0 if either of them is 0
	if(lhs.sign==BigInteger::ZERO || rhs.sign==BigInteger::ZERO)
	{
		sign = BigInteger::ZERO;
		return;
	}
	else if((lhs.sign==BigInteger::POSITIVE && rhs.sign==BigInteger::NEGATIVE) ||  
		    (lhs.sign==BigInteger::NEGATIVE && rhs.sign==BigInteger::POSITIVE))
	{
		sign = BigInteger::NEGATIVE;
	}
	else
		sign = BigInteger::POSITIVE;

	const BigInteger *lh_obj, *rh_obj;
	// have the larger one on the left side
	if(compareMagnitude(lhs, rhs) == BigInteger::LESS)
	{
		lh_obj = &rhs;
		rh_obj = &lhs;
	}
	else
	{
		lh_obj = &lhs;
		rh_obj = &rhs;
	}

	// reserve the storage to maximum size
	storage.reserve(lh_obj->storage.size() + rh_obj->storage.size());

	int storageIndex;
	BaseType carry = 0, buffer;
	// start multiplying and push the result back
	for(std::vector<int>::size_type lowerIndex = 0; lowerIndex<rh_obj->storage.size(); lowerIndex++) 
	{
		storageIndex = lowerIndex;
		for(std::vector<int>::size_type upperIndex = 0; upperIndex<lh_obj->storage.size(); upperIndex++) 
		{
			buffer = lh_obj->storage[upperIndex] * rh_obj->storage[lowerIndex] + carry;

			carry = buffer/BigInteger::Base;
			buffer %= BigInteger::Base;

			if(storageIndex < storage.size())
			{
				storage[storageIndex] += buffer;
				carry += storage[storageIndex]/BigInteger::Base;
				storage[storageIndex] %= BigInteger::Base;
			}
			else
				storage.push_back(buffer);

			// increment the storage group
			storageIndex++;
		}

		// wrap the carry
		for(storageIndex = storage.size(); carry != 0; storageIndex++)
		{
			buffer = carry;
			carry = buffer/BigInteger::Base;
			buffer %= BigInteger::Base;

			storage.push_back(buffer);
		}
	}

	// using karatsuba algorithm
	//karatsuba(lhs, rhs);

	//removeTrailingZeros();
}

void BigInteger::divide(const BigInteger& lhs, const BigInteger& rhs)
{
	if (rhs.isZero()) 
		throw "BigInteger::divide -> divide by zero";

	// empty the storage for new value
	storage.empty();

	// case for (0/B) or (A/B while A<B, 0 since the output is a integer)
	if(lhs.sign==BigInteger::ZERO || compareMagnitude(lhs, rhs)==BigInteger::LESS)
	{
		sign = BigInteger::ZERO;
		return;
	}

	*this = lhs;
	BigInteger result(1), buffer = rhs;

	// patching the lhs and rhs to the same length
	for(int repeatTimes = lhs.storage.size()-rhs.storage.size(); repeatTimes>0; repeatTimes--)
	{
		buffer *= 10;
		result *= 10;
	}
	*this -= buffer;

	while(lhs>0)
	{
		*this -= rhs; 
		result++;
	}

	*this = result--;
}

void BigInteger::modulus(const BigInteger& lhs, const BigInteger& rhs)
{

}

void BigInteger::karatsuba(const BigInteger& lhs, const BigInteger& rhs)
{

}

BigInteger::Compare BigInteger::compare(const BigInteger& lhs, const BigInteger& rhs) const
{
	// compate sign first
	if(lhs.sign > rhs.sign)
		return GREATER;
	else if(lhs.sign < rhs.sign)
		return LESS;

	// compare by magnitude and ajust by the sign
	if(lhs.sign == POSITIVE)
		return compareMagnitude(lhs, rhs);
	else
		return compareMagnitude(rhs, lhs);
}

BigInteger::Compare BigInteger::compareMagnitude(const BigInteger& lhs, const BigInteger& rhs) const
{
	if(lhs.storage.size() > rhs.storage.size())
		return GREATER;
	else if(lhs.storage.size() < rhs.storage.size())
		return LESS;
	else
	{
		BaseType lh_group, rh_group;
		lh_group = lhs.storage.back();
		rh_group = rhs.storage.back();

		if(lh_group > rh_group)
			return GREATER;
		else if(lh_group < rh_group)
			return LESS;
	}

	return EQUAL;
}

inline bool BigInteger::isZero() const
{
	return sign == BigInteger::ZERO;
}

void BigInteger::removeTrailingZeros()
{
	while(storage.back() == 0 && !storage.empty())
		storage.pop_back();

	// set sign flag to zero if the storage is empty
	if(storage.empty())
		sign = ZERO;

	storage.shrink_to_fit();
}

int main()
{
	BigInteger a("100"), b("5"), c;

	std::cout << "a:\t" << a << std::endl;
	std::cout << "b:\t" << b << std::endl;

	std::cout << "a+b:\t" << (a+b) << std::endl;
	std::cout << "a-b:\t" << (a-b) << std::endl;
	std::cout << "a*b:\t" << (a*b) << std::endl;
	std::cout << "a/b:\t" << (a/b) << std::endl;

	return 0;
}