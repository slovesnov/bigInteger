/*
 * Fraction.h
 *
 *  Created on: ??.12.2015
 *      Author: alexey slovesnov
 */

#ifndef FRACTION_H_
#define FRACTION_H_

#include "BigUnsigned.h"
#include <stdlib.h>//_abs64 strtoull
#include <sstream>

template<typename type=BigUnsigned>class Fraction{
public:
	bool positive;
	type dividend,divider;//dividend/divider

	Fraction(Fraction const& f){
		init(f.positive,f.dividend,f.divider);
	}

	Fraction(){
		init(true,0,1);
	}

	Fraction(const bool& _positive,const type& _dividend,const type& _divider=1){
		init( _positive, _dividend, _divider);
	}

	/*
	 * allow calls Fraction("0xff",2); all possible types
	 */
	template<typename type1,typename type2>
	Fraction(const type1 a,const type2 b){
		positive=true;//prevents eclipse warning
		init(a,b);
	}

	template<typename type1>
	Fraction(const type1 a){
		positive=true;//prevents eclipse warning
		init(a);
	}

	template<typename type1,typename type2>
	void init(const type1 a,const type2 b){
		SignedType va=createSignedType(a);
		SignedType vb=createSignedType(b);
		BIGNUMBER_ASSERT(vb.value!=0,"zero division");
		positive=va.positive==vb.positive;
		dividend=va.value;
		divider=vb.value;
		normilize();
	}

	template<typename type1>
	void init(const type1 a){
		init(a,1);
	}

	void init(const bool& _positive,const type& _dividend,const type& _divider){
		BIGNUMBER_ASSERT(_divider!=0,"zero division");
		positive=_positive;
		dividend=_dividend;
		divider=_divider;
		normilize();
	}

	Fraction const& operator=(Fraction const& f){
		init(f.positive,f.dividend,f.divider);
		return *this;
	}

	//unary +
	Fraction const& operator+()const {
		return *this;
	}

	//unary -
	Fraction operator-()const {
		if(dividend==0){
			return *this;
		}
		else{
			return Fraction(!positive,dividend,divider);
		}
	}

	Fraction operator+(Fraction const& o)const {
		type i1=dividend*o.divider;
		type i2=divider*o.dividend;
		type i3=divider*o.divider;
		Fraction f;
		if(positive == o.positive){
			f.init(positive,i1+i2,i3);
		}
		else{
			if(i1==i2){
				f.init(true,0,1);
				return f;
			}
			if(i1>=i2){
				f.init(positive,i1-i2,i3);
			}
			else{
				f.init(o.positive,i2-i1,i3);
			}
		}
		f.normilize();
		return f;
	}

	Fraction operator-(Fraction const& o)const {
		return *this+(-o);
	}

	Fraction operator*(Fraction const& o)const {
		type g1=gcd(o.dividend,divider);
		type g2=gcd(dividend,o.divider);
		return Fraction(positive==o.positive , (o.dividend/g1)*(dividend/g2) , (o.divider/g2)*(divider/g1) );
	}

	Fraction operator/(Fraction const& o)const {
		BIGNUMBER_ASSERT(o.divider!=0,"zero division");
		return *this*Fraction(o.positive,o.divider,o.dividend);
	}

#define M(o,type) Fraction operator o(const type t){return *this o Fraction(t);}
#define N(o) M(o,int&)M(o,unsigned&)\
		M(o,uint64_t&)M(o,int64_t&)

N(+)
N(-)
N(*)
N(/)
#undef N
#undef M

#define M(o,o1,type) Fraction const& operator o(const type t){return *this=*this o1 t;}
#define N(o,o1) M(o,o1,Fraction &)\
		M(o,o1,int&)M(o,o1,unsigned&)\
		M(o,o1,uint64_t&)M(o,o1,int64_t&)

N(+=,+)
N(-=,-)
N(*=,*)
N(/=,/)
#undef N
#undef M

	//begin comparison operators
	bool operator==(Fraction const& o)const {
		return (dividend==0 && o.dividend==0) || (positive==o.positive && dividend==o.dividend && divider==o.divider);
	}

	bool operator>(Fraction const& o)const {
		if(*this==o){
			return false;
		}
		if(positive==o.positive){
			bool  b=dividend*o.divider > o.dividend*divider;;
			if(positive){
				return b;
			}
			else{
				return !b;
			}
		}
		else{
			return positive;
		}
	}

	bool operator>=(Fraction const& o)const {
		if(*this==o){
			return true;
		}
		else{
			return *this>o;
		}
	}

	bool operator!=(Fraction const& o)const {
		return !(*this==o);
	}

	bool operator<(Fraction const& o)const {
		return !(*this>=o);
	}

	bool operator<=(Fraction const& o)const {
		return !(*this>o);
	}
	//end comparison operators

	void normilize(){
		if(divider==1){
			return;
		}

		if(dividend==0){
			divider=1;
			return;
		}

		type g=gcd(dividend,divider);
		if(g!=1){
			dividend/=g;
			divider/=g;
		}
	}

	static std::string typeToString(type const& u){
		std::stringstream s;
		s<<u;
		return s.str();
	}

	static std::string format(type t,
			const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR){
		return NumberFormatter::formatString(typeToString(t),positions,separator);
	}

	std::string toString(const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const {
		std::string s;
		if(!positive && dividend!=0){
			s+='-';
		}
		s+=format(dividend,positions,separator);
		if(dividend!=0 && divider!=1){
			s+='/';
			s+=format(divider,positions,separator);
		}
		return s;
	}

	void print(const char *s="")const{
		printf("%s%s",toString().c_str(),s);
	}

	std::string specialString(bool latex,const unsigned positions=NumberFormatter::DEFAULT_POSITIONS,
			const char separator=NumberFormatter::DEFAULT_SEPARATOR)const{
		std::string s=positive?"+":"-";
		if(divider==1){
			if(dividend!=1){
				s+=format(dividend,positions,separator);
			}
		}
		else{
			if(latex){
				s+="\\frac{";
			}
			s+=format(dividend,positions,separator);
			s+=latex? "}{" : "/";
			s+=format(divider,positions,separator);
			if(latex){
				s+="}";
			}
		}
		return s;
	}

	static std::string index(int i){
		char b[32];
		if(i<10){
			sprintf(b,"%d",i);
		}
		else{
			sprintf(b,"{%d}",i);
		}
		return b;
	}

	static type gcd(type num1, type num2){
		unsigned pof2;
		type tmp;
		if (num1==0 || num2==0) {
			return (num1 | num2);
		}

		/* pof2 is the greatest power of 2 dividing both numbers . We will use pof2 to multiply the returning number . */
		pof2 = 0;
		while((num1 & 1)==0 && (num2 & 1)==0) {
			/* gcd(even1, even1) = pof2 * gcd(even1/pof2, even2/pof2) */
			num1 >>=1;
			num2 >>=1;
			pof2++;
		}

		do {
			while ((num1 & 1)==0) {
				num1 >>=1;
			}
			while ((num2 & 1)==0) {
				num2 >>=1;
			}
			/* At this point we know for sure that num1 and num2 are odd */
			if (num1 >= num2) {
				num1 = (num1 - num2) >> 1;
			}
			else {
				tmp = num1;
				num1 = (num2 - num1) >> 1;
				num2 = tmp;
			}
		}while (!(num1 == num2 || num1 == 0));

		return (num2 << pof2);
	}

private:
	struct SignedType{
		bool positive;
		type value;
		SignedType(bool p,type v){
			positive=p;
			value=v;
		}
	};

	/*
	 * Note for type=uint64_t type we need createSignedType(const uint64_t&)
	 * For type=BigUnsigned we need createSignedType(const uint64_t&) & createSignedType(const BigUnsigned&)
	 *
	 * So the solution is create two functions for both types
	 * createSignedType(const uint64_t&) and createSignedType(const BigUnsigned&)
	 */

	static SignedType createSignedType(const int& i){
		return SignedType(i>=0, abs(i) );
	}

	static SignedType createSignedType(const unsigned& i){
		return SignedType(true, i );
	}

	static SignedType createSignedType(const int64_t& i){
		return SignedType(i>=0, _abs64(i) );
	}

	static SignedType createSignedType(const uint64_t& i){
		return SignedType(true, i );
	}

	static SignedType createSignedType(const BigUnsigned& i);

	static SignedType createSignedType(const std::string& s){
		return createSignedType(s.c_str());
	}

	static SignedType createSignedType(const char*s){
		const char*p=s;
		bool positive;
		if( (*p=='-' || *p=='+') && (p[1]=='-' || p[1]=='+') ){
			BIGNUMBER_ASSERT(0,"invalid string");
		}

		if(*p=='-'){
			p++;
			positive=false;
		}
		else{
			positive=true;
		}

		return SignedType(positive,createType(p));

	}

	static type createType(const char*s);

};

//should be defined for all template types
template<>uint64_t Fraction<uint64_t>::createType(const char*s);
template<>BigUnsigned Fraction<BigUnsigned>::createType(const char*s);

template<>Fraction<uint64_t>::SignedType Fraction<uint64_t>::createSignedType(const BigUnsigned& i);
template<>Fraction<BigUnsigned>::SignedType Fraction<BigUnsigned>::createSignedType(const BigUnsigned& i);

#define M(ret,op,type) template<typename t>ret operator op(const type v,const Fraction<t>& f){return Fraction<t>(v) op f;}

#define N(op) M(Fraction<t>,op,int&)M(Fraction<t>,op,unsigned&)\
		M(Fraction<t>,op,int64_t&)M(Fraction<t>,op,uint64_t&)
N(+)N(-)N(*)N(/)
#undef N

//boolean operators
#define N(op) M(bool,op,int&)M(bool,op,unsigned&)\
		M(bool,op,int64_t&)M(bool,op,uint64_t&)

N(==)N(!=)N(<)N(<=)N(>)N(>=)
#undef N

#undef M

template<typename t>
std::ostream& operator<<(std::ostream& o, Fraction<t> const& f){
	return o<<f.toString();
}

#endif
