/*
 * BigInteger.cpp
 *
 *  Created on: 18.01.2016
 *      Author: alexey slovesnov
 */

#include "BigInteger.h"

//define + - * / %
#define M(o,type) BigInteger operator o(const type t,BigInteger i){return BigInteger(t) o i;}
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
#define M(o,type) bool operator o(const type t,BigInteger const& i){return BigInteger(t) o i;}
#define N(o) M(o,int&)M(o,unsigned&)\
		M(o,uint64_t&)M(o,int64_t&)\
		M(o,char*)M(o,std::string&)
N(==)N(!=)N(<)N(<=)N(>)N(>=)
#undef N
#undef M

std::ostream& operator<<(std::ostream& o, BigInteger const& u){
	return o<<u.toString();
}
