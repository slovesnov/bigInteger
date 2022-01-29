/*
 * Fraction.cpp
 *
 *  Created on: 25.1.2016
 *      Author: alexey slovesnov
 */

#include "Fraction.h"

//should be defined for all template types
template<>
uint64_t Fraction<uint64_t>::createType(const char*s){
	bool hex;
	std::string p=BigUnsigned::prepareString(s,hex);
	char*pe;
	uint64_t v=strtoull(p.c_str(),&pe,hex?16:10);
	BIGNUMBER_ASSERT(*pe==0,"not all string recognized");
	return v;
}

template<>
BigUnsigned Fraction<BigUnsigned>::createType(const char*s){
	return BigUnsigned(s);
}

template<>Fraction<uint64_t>::SignedType Fraction<uint64_t>::createSignedType(const BigUnsigned& i){
	return SignedType(true, i.toUint64_t() );
}

template<>Fraction<BigUnsigned>::SignedType Fraction<BigUnsigned>::createSignedType(const BigUnsigned& i){
	return SignedType(true, i );
}
