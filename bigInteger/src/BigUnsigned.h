/*
 * BigUnsigned.h
 *
 *  Created on: 05.01.2016
 *      Author: alexey slovesnov
 */

#ifndef BIGUNSIGNED_H_
#define BIGUNSIGNED_H_

#include <cstdint>
#include <utility>
#include "BigNumberException.h"

namespace NumberFormatter{
	static const unsigned DEFAULT_POSITIONS=0;//no separation by default
	static const char DEFAULT_SEPARATOR=',';

	//formatString("1234567",3,',')="1,234,567"
	std::string formatString(std::string const&,const unsigned positions=DEFAULT_POSITIONS, const char separator=DEFAULT_SEPARATOR);

	/* Note
	 * PRIu64 universal macro
	 * sometimes "%llu" depends on includes
	 * sometimes "%I64u" depends on includes
	 *
	 * #define __STDC_FORMAT_MACROS
	 * #include <inttypes.h>
	 * sprintf(b,"%"PRIu64, v); //works
	 *
	 *
	 * #include <stdio.h>
	 * #define __STDC_FORMAT_MACROS
	 * #include <inttypes.h>
	 * sprintf(b,"%"PRIu64, v); //warning because of #include <stdio.h>
	 *
	 * use universal way
	 */
	std::string uint64_tToString(const uint64_t&);
	std::string uint64_tToHexString(const uint64_t&);
};

class BigUnsigned{
#define BASE_SIZE 4

#if BASE_SIZE==1
	typedef uint8_t base;
	typedef uint16_t doubleBase;
#elif BASE_SIZE==2
	typedef uint16_t base;
	typedef uint32_t doubleBase;
#elif BASE_SIZE==4
	typedef uint32_t base;
	typedef uint64_t doubleBase;
#else
	#error "invalid BASE_SIZE in class BigUnsigned"
#endif

#undef BASE_SIZE

	static const base BASE_MAX=-1;
	static const unsigned BASE_BITS=sizeof(base)*8;

	base*data;
	unsigned size;

	void allocate(unsigned newSize);
	void allocateCopy(unsigned newSize,unsigned copySize);
	void free();

	void removeLeadingZeros();
	void sub(BigUnsigned const& u)const;
	static uint64_t pow10(const unsigned k);

public:

	/**
	 * Note order of functions
	 * f()
	 * f(BigUnsigned const& u)
	 * f(const int& k)
	 * f(const unsigned& k)
	 * f(const int64_t& k)
	 * f(const uint64_t& k)
	 * f(const char* s)
	 * f(const std::string& s)
	 *
	 */

	BigUnsigned():data(NULL),size(0){
	}

	BigUnsigned(BigUnsigned const& u):data(NULL),size(0){
		*this=u;
	}

	BigUnsigned(const int& k):data(NULL),size(0) {
		BIGNUMBER_ASSERT(k>=0,"negative value");
		*this=k;
	}

	BigUnsigned(const unsigned& k):data(NULL),size(0){
		*this=k;
	}

	BigUnsigned(const int64_t& k):data(NULL),size(0){
		BIGNUMBER_ASSERT(k>=0,"negative value");
		*this=k;
	}

	BigUnsigned(const uint64_t& k):data(NULL),size(0){
		*this=k;
	}

	BigUnsigned(const char* s):data(NULL),size(0){
		*this=s;
	}

	BigUnsigned(const std::string& s):data(NULL),size(0){
		*this=s;
	}

	virtual ~BigUnsigned(){
		free();
	}

	BigUnsigned const& operator=(BigUnsigned const& u){
		allocate(u.size);
		memcpy(data,u.data,u.size*sizeof(base));
		return *this;
	}

	BigUnsigned const& operator=(const int& k){
		return *this=int64_t(k);
	}

	BigUnsigned const& operator=(const unsigned& k){
		return *this=uint64_t(k);
	}

	BigUnsigned const& operator=(const int64_t& k){
		BIGNUMBER_ASSERT(k>=0,"negative value");
		return *this=uint64_t(k);
	}

	BigUnsigned const& operator=(const uint64_t& k);

	BigUnsigned const& operator=(const char* s);//s="+0xabcd" hex string or s="+1234" decimal string

	BigUnsigned const& operator=(const std::string& s){
		return *this=s.c_str();
	}

	//begin string functions
	std::string toHexString(const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const;

	std::string to0xHexString(const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const{
		return "0x"+toHexString(positions,separator);
	}

	std::string toDecString(const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const{
		return hexToDecString(to0xHexString(),positions,separator);
	}

	std::string toString(const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const{
		return toDecString(positions,separator);
	}

	//"0xabcd" -> "43981"
	static std::string hexToDecString(const char* s,
			const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR);

	//"0xabcd" -> "43981"
	static std::string hexToDecString(const std::string& s,
			const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR){
		return hexToDecString(s.c_str(),positions,separator);
	}

	//"43981" -> "abcd"
	static std::string decToHexString(const char* s,
			const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR){
		BigUnsigned u(s);
		return u.toHexString(positions,separator);
	}

	//"43981" -> "abcd"
	static std::string decToHexString(const std::string& s,
			const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR){
		return decToHexString(s.c_str(),positions,separator);
	}
	//
	/**
	 * Note prepareString function
	 * remove leading zeros and separator char DEFAULT_SEPARATOR
	 * prepareString("+000001,222,33") -> "122233" hex=false
	 * prepareString("00,,,0001,33") -> "133" hex=false
	 * prepareString("0xabb,,,cc")-> "abbcc" hex=true
	 * prepareString("+0xabb,,,cc")-> "abbcc"  hex=true
	 * prepareString("-0x0000,,,00")-> "0"  hex=true
	 * prepareString("-000,,,00")-> "0"  hex=false
	 * throws exception on error
	 */
	static std::string prepareString(const char* s, bool& hex);
	//end string functions

	//begin other functions
	void div(BigUnsigned const& divisor,BigUnsigned& quotient,BigUnsigned& remainder)const;//this=quotient*divisor+remainder

	BigUnsigned pow(unsigned exponent)const;

	static BigUnsigned pow(unsigned base,unsigned exponent){
		return BigUnsigned(base).pow(exponent);
	}

	BigUnsigned factorial()const;
	static BigUnsigned factorial(unsigned n);
	//end other functions

	/*
	 * plus operators
	 * Note +(const int&), +(const unsigned&), +(const std::string&), +(const uint64_t& k) will be defined later using macro
	 */
	BigUnsigned const& operator+()const{//unary
		return *this;
	}

	BigUnsigned operator+(BigUnsigned const& u)const;

	/*
	 * BigUnsigned a=3;int64_t k=-2; a=a+k=1
	 * cann't use BigUnsigned operator+=(BigUnsigned const& u)
	 * a+BigUnsigned(-2)
	 */
	BigUnsigned operator+(const int64_t& k)const{
		if(k==0){
			return *this;
		}
		else if(k>0){
			return *this+BigUnsigned(k);
		}
		else{
			return *this-BigUnsigned(-k);
		}
	}

	BigUnsigned operator+(const char* s)const{
		if(*s=='-'){
			return *this-BigUnsigned(s+1);
		}
		else{
			return *this+BigUnsigned(s);
		}
	}

	/*
	 * minus operators
	 * Note -(const int&), -(const unsigned&), -(const std::string&), -(const uint64_t& k) will be defined later using macro
	 */
	BigUnsigned operator-()const{//unary
		BIGNUMBER_ASSERT(*this==0,"nonzero value");
		return *this;
	}

	BigUnsigned operator-(BigUnsigned const& u)const;

	BigUnsigned operator-(const int64_t& k)const{
		if(k==0){
			return *this;
		}
		else if(k>0){
			return *this-BigUnsigned(k);
		}
		else{
			return *this+BigUnsigned(-k);
		}
	}

	BigUnsigned operator-(const char* s)const{
		if(*s=='-'){
			return *this+BigUnsigned(s+1);
		}
		else{
			return *this-BigUnsigned(s);
		}
	}

	/*
	 * multiply operators
	 * Note *(const int&), *(const unsigned&), *(const std::string&), *(const uint64_t& k) will be defined later using macro
	 */
	BigUnsigned operator*(BigUnsigned const& u)const;

	BigUnsigned operator*(const int64_t& k)const{
		BIGNUMBER_ASSERT(k>=0,"negative value");
		return *this*BigUnsigned(k);
	}

	BigUnsigned operator*(const char* s)const{
		return *this*BigUnsigned(s);
	}

	/*
	 * division operators
	 * Note /(const int&), /(const unsigned&), /(const std::string&), /(const uint64_t& k) will be defined later using macro
	 */
	BigUnsigned operator/(BigUnsigned const& u)const{
		BigUnsigned quotient,remainder;
		div(u,quotient,remainder);
		return quotient;
	}

	BigUnsigned operator/(const int64_t& k)const{
		BIGNUMBER_ASSERT(k>0,"non positive value");
		return *this/BigUnsigned(k);
	}

	BigUnsigned operator/(const char* s)const{
		return *this/BigUnsigned(s);
	}

	/*
	 * % operators
	 * Note %(const int&), %(const unsigned&), %(const std::string&), %(const uint64_t& k) will be defined later using macro
	 */
	BigUnsigned operator%(BigUnsigned const& u)const{
		BigUnsigned quotient,remainder;
		div(u,quotient,remainder);
		return remainder;
	}

	BigUnsigned operator%(const int64_t& k)const{
		BIGNUMBER_ASSERT(k>0,"non positive value");
		return *this%BigUnsigned(k);
	}

	BigUnsigned operator%(const char* s)const{
		return *this%BigUnsigned(s);
	}

	//begin shift operators
	BigUnsigned operator>>(unsigned k)const;
	BigUnsigned operator>>=(unsigned k){
		return *this=*this>>k;
	}

	BigUnsigned operator<<(unsigned k)const;
	BigUnsigned operator<<=(unsigned k){
		return *this=*this<<k;
	}
	//end shift operators

	//begin comparison operators
	/*
	 * Note (const int&), (const unsigned&), (const std::string&), (const uint64_t& k) will be defined later using macro
	 */
	bool operator==(BigUnsigned const& u)const{
		return compareTo(u)==0;
	}
	bool operator==(const int64_t& k)const{
		if(k<0){
			return false;
		}
		return *this==BigUnsigned(k);
	}

	bool operator==(const char* s)const{
		if(*s=='-'){
			//check string
			bool hex;
			std::string p=prepareString(s+1,hex);
			if(p=="0" && *this==0){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return *this==BigUnsigned(s);
		}
	}

	/*
	 * Note (const int&), (const unsigned&), (const std::string&), (const uint64_t& k) will be defined later using macro
	 */
	bool operator<(BigUnsigned const& u)const{
		return compareTo(u)==-1;
	}

	bool operator<(const int64_t& k)const{
		if(k<0){
			return false;
		}
		else{
			return *this<BigUnsigned(k);
		}
	}

	bool operator<(const char* s)const{
		if(*s=='-'){
			//check string
			bool hex;
			prepareString(s+1,hex);
			return false;
		}
		else{
			return *this<BigUnsigned(s);
		}
	}

	/*
	 * Note (const int&), (const unsigned&), (const std::string&), (const uint64_t& k) will be defined later using macro
	 */
	bool operator<=(BigUnsigned const& u)const{
		return compareTo(u)!=1;
	}

	bool operator<=(const int64_t& k)const{
		if(k<0){
			return false;
		}
		else{
			return *this<=BigUnsigned(k);
		}
	}

	bool operator<=(const char* s)const{
		if(*s=='-'){
			//check string
			bool hex;
			std::string p=prepareString(s+1,hex);
			if(p=="0" && *this==0){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return *this<=BigUnsigned(s);
		}
	}

//define others ==,<,<=
#define N(o) bool operator o(const int& t)const{return *this o int64_t(t);}\
		bool operator o(const unsigned& t)const{return *this o BigUnsigned(t);}\
		bool operator o(const uint64_t& t)const{return *this o BigUnsigned(t);}\
		bool operator o(const std::string& t)const{return *this o t.c_str();}

N(==)
N(<)
N(<=)
#undef N

//define all !=,>,>=
#define M(o,o1,type) bool operator o(const type t)const{return !(*this o1 t);}

#define N(o,o1)  M(o,o1,BigUnsigned&)\
	M(o,o1,int&)M(o,o1,unsigned&)\
	M(o,o1,int64_t&)M(o,o1,uint64_t&)\
	M(o,o1,std::string&)M(o,o1,char*)

N(!=,==)
N(>,<=)
N(>=,<)
#undef N
#undef M

	int compareTo(BigUnsigned const& u)const;//return 0 if *this==u,-1 if *this<u, 1 if *this>u
	int compareSameSize(BigUnsigned const& u)const;//return 0 if *this==u,-1 if *this<u, 1 if *this>u
	//end comparison operators

	//begin bitwise operators
	BigUnsigned operator&(BigUnsigned const& u)const;
	BigUnsigned operator&(const int64_t& k)const{
		return *this&BigUnsigned(uint64_t(k));
	}
	BigUnsigned operator&(const char* s)const{
		return *this&BigUnsigned(s);
	}

	BigUnsigned operator|(BigUnsigned const& u)const;
	BigUnsigned operator|(const int64_t& k)const{
		return *this|BigUnsigned(uint64_t(k));
	}
	BigUnsigned operator|(const char* s)const{
		return *this|BigUnsigned(s);
	}

	BigUnsigned operator^(BigUnsigned const& u)const;
	BigUnsigned operator^(const int64_t& k)const{
		return *this^BigUnsigned(uint64_t(k));
	}
	BigUnsigned operator^(const char* s)const{
		return *this^BigUnsigned(s);
	}

	BigUnsigned operator~()const;
	//end bitwise operators

	uint64_t toUint64_t()const;

	double toDouble()const;

	std::pair<double,int> getMantissaExponent()const;
	/*
	 * Note
	 * operator+(const int& t) {return *this=*this+int64_t(t);}
	 * operator+(const unsigned& t) {return *this=*this+BigUnsigned(t);}
	 * operator+(const uint64_t& t) {return *this=*this+BigUnsigned(t);}
	 * operator+(const std::string& t) {return *this=*this+t.c_str();}
	 * operators +=, -=, *= etc
	 */

	#define N(o) BigUnsigned operator o(const int& t)const{return *this o int64_t(t);}\
			BigUnsigned operator o(const unsigned& t)const{return *this o BigUnsigned(t);}\
			BigUnsigned operator o(const uint64_t& t)const{return *this o BigUnsigned(t);}\
			BigUnsigned operator o(const std::string& t)const{return *this o t.c_str();}

	N(+)
	N(-)
	N(*)
	N(/)
	N(%)
	N(&)
	N(|)
	N(^)
	#undef N

	/*
	 * Note
	 * operator+=(some_type t) {return *this=*this+t;}
	 * operators +=, -=, *= etc
	 */
	#define M(o,o1,type) BigUnsigned const& operator o(const type t){return *this=*this o1 t;}
	#define N(o,o1) M(o,o1,BigUnsigned &)\
			M(o,o1,int&)M(o,o1,unsigned&)\
			M(o,o1,uint64_t&)M(o,o1,int64_t&)\
			M(o,o1,char*)M(o,o1,std::string&)

	N(+=,+)
	N(-=,-)
	N(*=,*)
	N(/=,/)
	N(%=,%)
	N(&=,&)
	N(|=,|)
	N(^=,^)
	#undef N
	#undef M

	void print(const char *s="",const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const{
		printf("%s%s",toString(positions,separator).c_str(),s);
	}
};

/**
 * One defined BigUnsigned operator+(const uint64_t& k,BigUnsigned const& u);
 * allow do
 * BigUnsigned operator+(const unsigned&,BigUnsigned const& u);
 * BigUnsigned operator+(const int&,BigUnsigned const& u);
 * etc
 */
#define M(o,type) BigUnsigned operator o(const type,BigUnsigned const&);
#define N(o) M(o,int&)M(o,unsigned&)\
		M(o,uint64_t&)M(o,int64_t&)\
		M(o,char*)M(o,std::string&)
N(+)N(-)N(*)N(/)N(%)
N(&)N(|)N(^)
#undef N
#undef M

//boolean operators
#define M(o,type) bool operator o(const type,BigUnsigned const&);
#define N(o) M(o,int&)M(o,unsigned&)\
		M(o,uint64_t&)M(o,int64_t&)\
		M(o,char*)M(o,std::string&)
N(==)N(!=)N(<)N(<=)N(>)N(>=)
#undef N
#undef M

std::ostream& operator<<(std::ostream& , BigUnsigned const&);

#endif /* BIGUNSIGNED_H_ */
