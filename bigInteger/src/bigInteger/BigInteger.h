/*
 * BigInteger.h
 *
 *  Created on: 17.01.2016
 *      Author: alexey slovesnov
 */

#ifndef BIGINTEGER_H_
#define BIGINTEGER_H_

#include "BigUnsigned.h"

class BigInteger{
	BigUnsigned u;
	bool positive;//>=0

	static const unsigned DEFAULT_POSITIONS=BigUnsigned::DEFAULT_POSITIONS;//no separation by default
	static const char DEFAULT_SEPARATOR=BigUnsigned::DEFAULT_SEPARATOR;

public:
	BigInteger(){
		positive=true;
	}

	BigInteger(BigUnsigned const& _u,bool _positive=true){
		positive=_positive;
		u=_u;
	}

#define M(type) BigInteger(const type t) {*this=t;}
	M(BigInteger&)
	M(int&)
	M(unsigned&)
	M(uint64_t&)
	M(int64_t&)
	M(char*)
	M(std::string&)
#undef M

	virtual ~BigInteger(){
	}

	BigInteger const& operator=(BigInteger const& t){
		positive=t.positive;
		u=t.u;
		return *this;
	}

	BigInteger const& operator=(const int& t){
		return *this=int64_t(t);
	}

	BigInteger const& operator=(const unsigned& t){
		return *this=uint64_t(t);
	}

	BigInteger const& operator=(const int64_t& t){
		positive=t>=0;
		u=llabs(t);
		return *this;
	}

	BigInteger const& operator=(const uint64_t& t){
		positive=true;
		u=t;
		return *this;
	}

	BigInteger const& operator=(const char* s){//s="+0xabcd" hex string or s="+1234" decimal string
		const char* p;
		if(*s=='-'){
			p=s+1;
			positive=false;
		}
		else{
			p=s;
			positive=true;
		}
		u=p;
		return *this;
	}

	BigInteger const& operator=(const std::string& s){
		return *this=s.c_str();
	}

	//begin string functions
	std::string toHexString(const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR)const{
		std::string s;
		if(!positive){
			s+='-';
		}
		return s+u.toHexString(positions,separator);
	}

	std::string to0xHexString(const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR)const{
		std::string s;
		if(!positive){
			s+='-';
		}
		return s+u.to0xHexString(positions,separator);
	}

	std::string toDecString(const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR)const{
		std::string s;
		if(!positive){
			s+='-';
		}
		return s+u.toDecString(positions,separator);
	}
	std::string toString(const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR)const{
		return toDecString(positions,separator);
	}

	void print(const char *s="",const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR)const{
		printf("%s%s",toString(positions,separator).c_str(),s);
	}

	BigInteger const& operator+()const{//unary
		return *this;
	}

	//other operator argument types define later using macro
	BigInteger operator+(BigInteger const& t)const{
		if(positive==t.positive){
			return BigInteger(u+t.u,positive);
		}
		int j=u.compareTo(t.u);
		if(j==0){
			return BigInteger(0,true);
		}
		else if(j>0){
			return BigInteger(u-t.u,positive);
		}
		else{
			return BigInteger(t.u-u,t.positive);
		}
	}

	BigInteger const operator-()const{//unary
		if(u==0){
			return BigInteger(0);
		}
		else{
			return BigInteger(u,!positive);
		}
	}

	//other operator argument types define later using macro
	BigInteger operator-(BigInteger const& t)const{
		return *this+(-t);
	}

	//other operator argument types define later using macro
	BigInteger operator*(BigInteger const& t)const{
		return BigInteger(u*t.u,positive==t.positive);
	}

	//other operator argument types define later using macro
	BigInteger operator/(BigInteger const& t)const{
		BigUnsigned q,r;
		u.div(t.u,q,r);
		return BigInteger(q,positive==t.positive);
	}

	//other operator argument types define later using macro
	BigInteger operator%(BigInteger const& t)const{
		BigUnsigned q,r;
		u.div(t.u,q,r);
		return BigInteger(r,positive==t.positive);
	}

	BigInteger operator>>(unsigned k)const{
		return BigInteger(u>>k,positive);
	}

	BigInteger& operator>>=(unsigned k){
		u>>=k;
		return *this;
	}

	BigInteger operator<<(unsigned k)const{
		return BigInteger(u<<k,positive);
	}

	BigInteger& operator<<=(unsigned k){
		u<<=k;
		return *this;
	}

	bool operator==(BigInteger const& t)const{
		return compareTo(t)==0;
	}

	bool operator<(BigInteger const& t)const{
		return compareTo(t)==-1;
	}

	bool operator<=(BigInteger const& t)const{
		return compareTo(t)!=1;
	}

	//define others ==,<,<=
	#define M(o,type) bool operator o(const type t)const{return *this o BigInteger(t);}
	#define N(o) M(o,int&)M(o,unsigned&)\
			M(o,int64_t&)M(o,uint64_t&)\
			M(o,std::string&)M(o,char*)
	N(==)
	N(<)
	N(<=)
	#undef N
	#undef M

	//define all !=,>,>=
	#define M(o,o1,type) bool operator o(const type t)const{return !(*this o1 t);}

	#define N(o,o1)  M(o,o1,BigInteger&)\
		M(o,o1,int&)M(o,o1,unsigned&)\
		M(o,o1,int64_t&)M(o,o1,uint64_t&)\
		M(o,o1,std::string&)M(o,o1,char*)

	N(!=,==)
	N(>,<=)
	N(>=,<)
	#undef N
	#undef M

	//return 0 if *this==t,-1 if *this<t, 1 if *this>t
	int compareTo(BigInteger const& t)const{
		int j;
		if(positive==t.positive){
			j=u.compareTo(t.u);
			return positive ? j : -j;
		}
		else{
			return positive ? 1:-1;
		}
	}


	//define + - * / %
	#define M(o,type) BigInteger operator o(const type t)const{return *this o BigInteger(t);}

	#define N(o) M(o,BigUnsigned&)\
		M(o,int&)M(o,unsigned&)\
		M(o,int64_t&)M(o,uint64_t&)\
		M(o,std::string&)M(o,char*)

	N(+)
	N(-)
	N(*)
	N(/)
	N(%)
	#undef N
	#undef M

	//define += -= *= /= %=
	#define M(o,o1,type) BigInteger operator o(const type t){return *this=*this o1 t;}
	#define N(o,o1)  M(o,o1,BigInteger&)M(o,o1,BigUnsigned&)\
		M(o,o1,int&)M(o,o1,unsigned&)\
		M(o,o1,int64_t&)M(o,o1,uint64_t&)\
		M(o,o1,std::string&)M(o,o1,char*)

	N(+=,+)
	N(-=,-)
	N(*=,*)
	N(/=,/)
	N(%=,%)
	#undef N
	#undef M

	BigInteger abs()const{
		return BigInteger(u,true);
	}

	BigUnsigned getUnsigned()const{
		return u;
	}

	BigInteger pow(unsigned exponent)const{
		return BigInteger( u.pow(exponent),exponent%2==0 );
	}

	static BigInteger pow(int base,unsigned exponent){//allow -5^3
		return BigInteger( BigUnsigned(::abs(base)).pow(exponent), base>0 || (base<0 && exponent%2==0) );
	}

	BigInteger factorial()const{
		return u.factorial();
	}

	static BigInteger factorial(unsigned n){
		return BigUnsigned::factorial(n);
	}

	static BigInteger binomial(unsigned k,unsigned n){
		return BigUnsigned::binomial(k, n);
	}

	void div(BigInteger const& divisor,BigInteger& quotient,BigInteger& remainder)const{//this=quotient*divisor+remainder
		remainder.positive=quotient.positive=positive == divisor.positive;//-5/2=-2*2-1
		u.div(divisor.u,quotient.u,remainder.u);
	}

	//"0xabcd" -> "43981"
	static std::string hexToDecString(const std::string& s,
			const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR){
		return BigUnsigned::hexToDecString(s,positions,separator);
	}

	//"0xabcd" -> "43981"
	static std::string hexToDecString(const char* s,
			const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR){
		return BigUnsigned::hexToDecString(s,positions,separator);
	}

	//"43981" -> "abcd"
	static std::string decToHexString(const char* s,
			const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR){
		return BigUnsigned::decToHexString(s,positions,separator);
	}

	//"43981" -> "abcd"
	static std::string decToHexString(const std::string& s,
			const unsigned positions=DEFAULT_POSITIONS,
			const char separator=DEFAULT_SEPARATOR){
		return BigUnsigned::decToHexString(s,positions,separator);
	}


	double toDouble()const{
		double v = u.toDouble();
		return positive ? v : -v;
	}

	long double toLongDouble()const{
		long double v = u.toLongDouble();
		return positive ? v : -v;
	}

	std::pair<double,int> getMantissaExponent()const{
		auto p=u.getMantissaExponent();
		if(!positive){
			p.first=-p.first;
		}
		return p;
	}

	std::pair<double,int> getMantissaExponentLongDouble()const{
		auto p=u.getMantissaExponentLongDouble();
		if(!positive){
			p.first=-p.first;
		}
		return p;
	}


};

//define + - * / %
#define M(o,type) BigInteger operator o(const type,BigInteger);
#define N(o) M(o,int&)M(o,unsigned&)\
	M(o,int64_t&)M(o,uint64_t&)\
	M(o,std::string&)M(o,char*)

N(+)
N(-)
N(*)
N(/)
N(%)
#undef N
#undef M

//boolean operators
#define M(o,type) bool operator o(const type,BigInteger const&);
#define N(o) M(o,int&)M(o,unsigned&)\
		M(o,uint64_t&)M(o,int64_t&)\
		M(o,char*)M(o,std::string&)
N(==)N(!=)N(<)N(<=)N(>)N(>=)
#undef N
#undef M

std::ostream& operator<<(std::ostream& , BigInteger const&);

#endif /* BIGINTEGER_H_ */
