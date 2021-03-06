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
	operator = (0);
}

BigInteger::BigInteger(const int& input)
{
	operator = (input);
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

// unary operator
void BigInteger::operator - ()
{
	if(sign == BigInteger::POSITIVE)
		sign = BigInteger::NEGATIVE;
	else if(sign == BigInteger::NEGATIVE)
		sign = BigInteger::POSITIVE;
}

BigInteger BigInteger::operator - () const
{
	BigInteger result(*this);
	result.operator - ();
	return result;
}

void BigInteger::operator ++ ()
{
	if(isZero())
	{
		// 0 -> 1
		operator = (1);
		return;
	}
	else if(sign==BigInteger::NEGATIVE && storage.size()==1 && storage[0]==1)
	{
		// -1 -> 0
		operator = (0);
		return;
	}

	// increment for one
	storage[0]++;

	// wrap for all the carries
	BaseType carry = 0;
	for(int index = 0; index<storage.size(); index++)
	{
		storage[index] += carry;
		carry = storage[index]/BigInteger::Base;
		storage[index] %= BigInteger::Base;
	}

	if(carry != 0)
		storage.push_back(carry);
}

void BigInteger::operator ++ (int)
{
	operator ++ ();
}

void BigInteger::operator -- ()
{
	if(isZero())
	{
		// 0 -> -1
		operator = (-1);
		return;
	}
	else if(sign==BigInteger::POSITIVE && storage.size()==1 && storage[0]==1)
	{
		// 1 -> 0
		operator = (0);
		return;
	}

	BaseType carry = 0;

	// decrement for one
	if(storage[0] == 0)
	{
		carry = 1;
		storage[0] = BigInteger::Base-1;
	}
	else
		storage[0]--;

	// wrap for all the carries
	for(int index = 1; index<storage.size(); index++)
	{
		if(storage[index]<carry)
		{
			storage[index] += BigInteger::Base;
			storage[index] -= carry;
			carry = 1;
		}
		else
		{
			storage[index] -= carry;
			carry = 0;
		}
	}

	removeTrailingZeros();
}

void BigInteger::operator -- (int)
{
	operator -- ();
}

// binary operator: arithmetic
const BigInteger BigInteger::operator + (const BigInteger& rhs) const
{
	BigInteger result;
	result.add(*this, rhs);
	return result;
}

const BigInteger BigInteger::operator - (const BigInteger& rhs) const
{
	BigInteger result;
	result.subtract(*this, rhs);
	return result;
}

const BigInteger BigInteger::operator * (const BigInteger& rhs) const
{
	BigInteger result;
	result.multiply(*this, rhs);
	return result;
}

const BigInteger BigInteger::operator / (const BigInteger& rhs) const
{
	BigInteger result;
	result.divide(*this, rhs);
	return result;
}

const BigInteger BigInteger::operator % (const BigInteger& rhs) const
{
	BigInteger result;
	result.modulus(*this, rhs);
	return result;
}

// binary operator: arithmetic (continue)
BigInteger& BigInteger::operator += (const BigInteger& rhs)
{
	//add(*this, rhs);
	operator = (operator + (rhs));
	return *this;
}

BigInteger& BigInteger::operator -= (const BigInteger& rhs)
{
	//subtract(*this, rhs);
	operator = (operator - (rhs));
	return *this;
}
BigInteger& BigInteger::operator *= (const BigInteger& rhs)
{
	//multiply(*this, rhs);
	operator = (operator * (rhs));
	return *this;
}

BigInteger& BigInteger::operator *= (const int& rhs)
{
	int maxIndex = storage.size(), carry = 0;
	for(int index = 0; index < maxIndex; index++)
	{
		storage[index] *= rhs + carry;
		carry = storage[index]/BigInteger::Base;
		storage[index] %= BigInteger::Base;
	}

	if(carry != 0)
		storage.push_back(carry);

	/*
	BigInteger rh_obj(rhs);
	operator = (operator * (rh_obj));
	*/
	
	return *this;
}

BigInteger& BigInteger::operator /= (const BigInteger& rhs)
{
	//divide(*this, rhs);
	operator = (operator / (rhs));
	return *this;
}

BigInteger& BigInteger::operator /= (const int& rhs)
{
	if(rhs==2)
	{
		int carry = 0;
		for(int index = storage.size()-1; index >= 0; index--)
		{
			if(carry == 1)
				storage[index] += BigInteger::Base;

			carry = storage[index]%2;
			storage[index] /= 2;
		}

		removeTrailingZeros();
	}
	else
	{
		BigInteger rh_obj(rhs);
		operator = (operator / (rh_obj));
	}

	return *this;
}

BigInteger& BigInteger::operator %= (const BigInteger& rhs)
{
	//modulus(*this, rhs);
	operator = (operator % (rhs));
	return *this;
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
BigInteger& BigInteger::operator = (const BigInteger& rhs)
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

	return *this;
}

BigInteger& BigInteger::operator = (const int& rhs)
{
	int temp = rhs;

	// empty the storage
	storage.clear();
	storage.reserve(3);

	// register the sign of default value
	if(temp == 0)
		sign = BigInteger::ZERO;
	else if(temp > 0)
		sign = BigInteger::POSITIVE;
	else
	{
		sign = BigInteger::NEGATIVE;
		temp *= -1;
	}

	while(temp > 0)
	{
		storage.push_back(temp%BigInteger::Base);
		temp /= BigInteger::Base;
	}

	return *this;
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
		case BigInteger::POSITIVE:
			#ifdef FORCE_SHOW_POSITIVE
			stream << '+';
			#endif
			break;
		default:
			throw "BigInteger::operator<<(std::ostream&, const BigInteger&) -> Sign undefine";
	}

	// print the first group without padding
	std::cout << rhs.storage.back();

	// reverse iterate the groups and print them out
	std::vector<BigInteger::BaseType>::const_reverse_iterator iterator = rhs.storage.rbegin();
	for(iterator++; iterator != rhs.storage.rend(); ++iterator)
		stream << std::setfill('0') << std::setw(BigInteger::BaseMagnitude10) << *iterator;

	return stream;
}

bool BigInteger::iseven()
{
	if(storage.front()%2==0)
		return true;
	else
		return false;
}

bool BigInteger::iszero() const
{
	return isZero();
}

//
// support functions
//
void BigInteger::add(const BigInteger& lhs, const BigInteger& rhs)
{
	#ifdef DEBUG_ADD
	std::cout << "=====" << std::endl;
	std::cout << "add() called" << std::endl;
	#endif

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

	#ifdef DEBUG_ADD
	std::cout << "lh_obj: " << *lh_obj << "; rh_obj: " << *rh_obj << std::endl;
	#endif

	if(lhs.sign == rhs.sign)
	{
		// duplicate the longer one, and ignore if it's itself
		if(lh_obj != this)
			operator = (*lh_obj);

		BaseType carry = 0, buffer;
		// iterate through rh_obj, add with lh_obj, and store into *this
		for(std::vector<int>::size_type index = 0; index<rh_obj->storage.size(); index++)
		{
			// add the carry
			buffer = lh_obj->storage[index] + rh_obj->storage[index] + carry;

			// wrap the digit
			carry = buffer/BigInteger::Base;
			buffer %= BigInteger::Base;

			// store back
			storage[index] = buffer;
		}

		if(carry > 0)
			storage.push_back(carry);
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
			//*this = -(*this);
			operator - ();
		}
	}

	#ifdef DEBUG_ADD
	std::cout << "add(): result is " << *this << std::endl;
	std::cout << "=====" << std::endl;
	#endif
}

void BigInteger::subtract(const BigInteger& lhs, const BigInteger& rhs)
{
	#ifdef DEBUG_SUBTRACT
	std::cout << "=====" << std::endl;
	std::cout << "subtract() called" << std::endl;
	#endif

	if(lhs.sign == ZERO)
	{
		#ifdef DEBUG_SUBTRACT
		std::cout << "...lhs is ZERO" << std::endl;
		#endif

		operator = (rhs);
		//*this = -(*this);
		operator - ();
		return;
	}
	else if(rhs.sign == ZERO)
	{
		#ifdef DEBUG_SUBTRACT
		std::cout << "...rhs is ZERO" << std::endl;
		#endif

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

	#ifdef DEBUG_SUBTRACT
	std::cout << "lh_obj: " << *lh_obj << "; rh_obj: " << *rh_obj << std::endl;
	#endif

	if(lhs.sign == rhs.sign)
	{
		#ifdef DEBUG_SUBTRACT
		std::cout << "sign EQUAL" << std::endl;
		#endif

		// duplicate the longer one, and ignore if it's itself
		if(lh_obj != this)
			operator = (*lh_obj);

		#ifdef DEBUG_SUBTRACT
		std::cout << "lh_obj copy complete" << std::endl;
		#endif

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
				if(lh_obj->storage[index] < rh_obj->storage[index]+carry)
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
		#ifdef DEBUG_SUBTRACT
		std::cout << "sign DIFFERENT" << std::endl;
		#endif

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
			//*this = -(*this);
			operator - ();
		}
	}

	// negate the result when lhs and rhs are swapped
	if(lh_obj != &lhs)
		operator - ();

	removeTrailingZeros();

	#ifdef DEBUG_SUBTRACT
	std::cout << "=====" << std::endl;
	#endif
}

void BigInteger::multiply(const BigInteger& lhs, const BigInteger& rhs)
{
	#ifdef DEBUG_MULTIPLY
	std::cout << "=====" << std::endl;
	std::cout << "multiply() called" << std::endl;
	#endif

	// empty the storage for new value
	storage.clear();

	// set as 0 if either of them is 0
	if(lhs.isZero() || rhs.isZero())
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

	#ifdef DEBUG_MULTIPLY
	std::cout << "lh_obj: " << *lh_obj << "; rh_obj: " << *rh_obj << std::endl;
	#endif

	// reserve the storage to maximum size
	storage.reserve(lh_obj->storage.size() + rh_obj->storage.size());

	#ifdef DEBUG_MULTIPLY
	std::cout << "storage space reserve complete" << std::endl;
	#endif

	int storageIndex, lh_size = lh_obj->storage.size(), rh_size = rh_obj->storage.size();
	BaseType carry = 0, buffer;
	// start multiplying and push the result back
	for(int lowerIndex = 0; lowerIndex<rh_size; lowerIndex++)
	{
		storageIndex = lowerIndex;
		for(int upperIndex = 0; upperIndex<lh_size; upperIndex++)
		{
			buffer = lh_obj->storage[upperIndex] * rh_obj->storage[lowerIndex] + carry;

			if(storageIndex < storage.size())
				buffer += storage[storageIndex];

			carry = buffer/BigInteger::Base;
			buffer %= BigInteger::Base;

			if(storageIndex < storage.size())
				storage[storageIndex] = buffer;
			else
				storage.push_back(buffer);

			storageIndex++;
		}

		if(carry != 0)
		{
			storage.push_back(carry);
			carry = 0;
		}
	}

	// using karatsuba algorithm
	//karatsuba(lhs, rhs);

	//removeTrailingZeros();

	#ifdef DEBUG_MULTIPLY
	std::cout << "=====" << std::endl;
	#endif
}

void BigInteger::divide(const BigInteger& lhs, const BigInteger& rhs)
{
	#ifdef DEBUG_DIVIDE
	std::cout << "=====" << std::endl;
	std::cout << "divide() called" << std::endl;
	#endif

	if (rhs.isZero())
		throw "BigInteger::divide -> divide by zero";

	#ifdef DEBUG_DIVIDE
	std::cout << "storage emptied" << std::endl;
	#endif

	// case for (0/B) or (A/B while A<B, 0 since the output is a integer)
	if(lhs.isZero() || compareMagnitude(lhs, rhs)==BigInteger::LESS)
	{
		#ifdef DEBUG_DIVIDE
		std::cout << "direct output 0, due to (0/b) or (a/b while a<b)" << std::endl;
		#endif

		operator = (0);
		return;
	}

	BigInteger /*result,*/ temp, lh_buf(lhs), rh_buf(rhs);
	operator = (0);
	Sign signBackup = rh_buf.sign;

	// set the sign, and have lh_buf and rh_buf as positive
	if(lh_buf.sign==rh_buf.sign)
	{
		if(lh_buf.sign==BigInteger::NEGATIVE)
		{
			// negate both side
			-lh_buf;
			-rh_buf;
		}

		sign = BigInteger::POSITIVE;
	}
	else
	{
		#ifdef DEBUG_DIVIDE
		std::cout << "negative result, due to ";
		#endif

		if(lh_buf.sign==BigInteger::NEGATIVE)
		{
			#ifdef DEBUG_DIVIDE
			std::cout << "lh_buf" << std::endl;
			#endif

			-lh_buf;
		}
		else
		{
			#ifdef DEBUG_DIVIDE
			std::cout << "rh_buf" << std::endl;
			#endif

			-rh_buf;
		}

		sign = BigInteger::NEGATIVE;
	}

	#ifdef DEBUG_DIVIDE
	std::cout << "variable initialized" << std::endl;
	#endif

	int magnifier_magnitude = static_cast<int>(BigInteger::BaseMagnitude10), magnifier;
	//int magnifier_magnitude = lh_buf.getPreciseMagnitude() - rh_buf.getPreciseMagnitude(), magnifier;

	// group based elimination
	while(magnifier_magnitude>=0)
	{
		magnifier = 1;
		for(int i = magnifier_magnitude; i > 0; i--)
			magnifier *= 10;
		BigInteger converted_magnifier(magnifier);

		#ifdef DEBUG_DIVIDE
		std::cout << "==>m using magnifier " << converted_magnifier << std::endl;
		#endif

		// reset the template value
		temp = 1;

		// reset rh_buf(with toggled sign) and temporary result
		rh_buf = rhs;
		rh_buf.sign = signBackup;

		if(magnifier_magnitude == 0)
			magnifier_magnitude--;
		else
		{
			bool multiplied;
			for(multiplied = false; rh_buf*converted_magnifier <= lh_buf; rh_buf *= converted_magnifier)
			{
				temp *= converted_magnifier;
				multiplied = true;
			}

			if(multiplied)
			{
				// rh_buf is too large for this magnification rate
				magnifier_magnitude--;
				continue;
			}
		}

		#ifdef DEBUG_DIVIDE
		std::cout << "> patched" << std::endl;
		std::cout << "...rh_buf: " << rh_buf << std::endl;
		#endif

		for(; lh_buf >= rh_buf; lh_buf -= rh_buf)
		{
			#ifdef DEBUG_DIVIDE
			std::cout << "> in the loop" << std::endl;
			std::cout << "...lh_buf: " << lh_buf << std::endl;
			std::cout << "...rh_buf: " << rh_buf << std::endl;
			std::cout << "...temp  : " << temp << std::endl;
			std::cout << "...result: " << result << std::endl;
			#endif

			/*result += temp;*/
			operator += (temp);
		}

		#ifdef DEBUG_DIVIDE
		std::cout << "> outside the loop, before back off" << std::endl;
		std::cout << "...lh_buf: " << lh_buf << std::endl;
		std::cout << "...rh_buf: " << rh_buf << std::endl;
		std::cout << "...temp  : " << temp << std::endl;
		std::cout << "...result: " << result << std::endl;
		#endif

		if(lh_buf.isZero())
		{
			// divided completely, quit the loop immediately
			break;
		}

		
	}

	// copy back the sign
	/*
	result.sign = sign;
	operator = (result);
	*/

	#ifdef DEBUG_DIVIDE
	std::cout << "=====" << std::endl;
	#endif
}

void BigInteger::modulus(const BigInteger& lhs, const BigInteger& rhs)
{
	#ifdef DEBUG_MODULUS
	std::cout << "=====" << std::endl;
	std::cout << "modulus() called" << std::endl;
	#endif

	if(compareMagnitude(lhs, rhs) == BigInteger::LESS)
	{
		#ifdef DEBUG_MODULUS
		std::cout << "direct output" << std::endl;
		#endif

		operator = (lhs);
	}
	else
	{
		#ifdef DEBUG_MODULUS
		std::cout << "lhs/rhs = " << (lhs/rhs) << std::endl;
		std::cout << "rhs*(lhs/rhs) = " << (rhs*(lhs/rhs)) << std::endl;
		#endif

		operator = (lhs - rhs * (lhs/rhs));
	}

	#ifdef DEBUG_MODULUS
	std::cout << "=====" << std::endl;
	#endif
}

void BigInteger::karatsuba(const BigInteger& lhs, const BigInteger& rhs)
{

}

BigInteger::Compare BigInteger::compare(const BigInteger& lhs, const BigInteger& rhs) const
{
	// compate sign first
	if(lhs.sign > rhs.sign)
		return BigInteger::GREATER;
	else if(lhs.sign < rhs.sign)
		return BigInteger::LESS;

	// compare by magnitude and ajust by the sign
	if(lhs.sign == BigInteger::POSITIVE)
		return compareMagnitude(lhs, rhs);
	else
		return compareMagnitude(rhs, lhs);
}

int BigInteger::getPreciseMagnitude() const
{
	int result = (storage.size()-1) * BigInteger::BaseMagnitude10;

	// find out the digit counts for the msg(most significant group)
	int temp = storage.back();
	for(; temp>0; result++, temp/=10);

	return result;
}

BigInteger::Compare BigInteger::compareMagnitude(const BigInteger& lhs, const BigInteger& rhs) const
{
	if(lhs.storage.size() > rhs.storage.size())
		return BigInteger::GREATER;
	else if(lhs.storage.size() < rhs.storage.size())
		return BigInteger::LESS;
	else
	{
		for(int index = lhs.storage.size()-1; index>=0; index--)
		{
			if(lhs.storage[index] > rhs.storage[index])
				return BigInteger::GREATER;
			else if(lhs.storage[index] < rhs.storage[index])
				return BigInteger::LESS;
		}

		return EQUAL;
	}
}

bool BigInteger::isZero() const
{
	return sign==BigInteger::ZERO;
}

void BigInteger::removeTrailingZeros()
{
	while(storage.back() == 0 && !storage.empty())
		storage.pop_back();

	// set sign flag to zero if the storage is empty
	if(storage.empty())
		sign = BigInteger::ZERO;

	storage.shrink_to_fit();
}
