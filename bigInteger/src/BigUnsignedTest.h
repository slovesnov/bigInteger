/*
 * BigUnsignedTest.h
 *
 *  Created on: 07.01.2016
 *      Author: alexey slovesnov
 */

#ifndef BIGUNSIGNEDTEST_H_
#define BIGUNSIGNEDTEST_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "BigUnsigned.h"

const unsigned NUMBER_OF_TRIALS=100000;

template <class type>
void generateRandom(type& v){
	v=0;
	unsigned i;
	type t;
	for(i=0;i<sizeof(type)/sizeof(char);i++){
		t=rand()&0xff;
		v|=(t<<(i*8));
	}
}

std::string generateRandomHexString(unsigned len,bool OxPrefix){
	unsigned i;
	std::string s;
	if(OxPrefix){
		s="0x";
	}
	char c[]="0123456789abcdef";
	const unsigned clen=strlen(c);
	for(i=0;i<len;i++){
		s+= c[rand()%clen];
	}
	return s;
}


void testHexString(){
	printf("%s()\n",__func__);
	unsigned i;
	uint64_t v;
	BigUnsigned a;
	std::string b;
	for(i=0;i<NUMBER_OF_TRIALS;i++){
		generateRandom(v);
		b=NumberFormatter::uint64_tToHexString(v);
		a="0x"+b;
		if(a.toHexString()!=b){
			printf("ERROR 0x%s != %s trial %d\n",a.toHexString().c_str(),b.c_str(),i);
			break;
		}
	}

}

void testDecString(){
	printf("%s()\n",__func__);
	unsigned i;
	uint64_t v;
	BigUnsigned a;
	std::string b;
	for(i=0;i<NUMBER_OF_TRIALS;i++){
		generateRandom(v);
		b=NumberFormatter::uint64_tToString(v);
		a=b;
		b=NumberFormatter::uint64_tToHexString(v);
		if(a.toHexString()!=b){
			printf("ERROR 0x%s != %s trial %d\n",a.toHexString().c_str(),b.c_str(),i);
			break;
		}
	}

}

void testArithmeticOperators(){
	const char O[]="+-*/%";
	//const char O[]="/%";
	printf("%s() %s\n",__func__,O);
	unsigned i;
	const char*po;
	uint32_t va,vb,t;
	uint64_t v;
	std::string s;
	BigUnsigned a,b,c;
	for(po=O;*po!=0;po++){
		for(i=0;i<NUMBER_OF_TRIALS;i++){
			generateRandom(va);
			generateRandom(vb);

			if(*po=='-' && va<vb){
				t=va;
				va=vb;
				vb=t;
			}

			v=va;
			a=va;
			b=vb;

			switch(*po){
			case '+':
				v+=vb;
				c=a+b;
				break;

			case '-':
				v-=vb;
				c=a-b;
				break;

			case '*':
				v*=vb;
				c=a*b;
				break;

			case '/':
				v/=vb;
				c=a/b;
				break;

			case '%':
				v%=vb;
				c=a%b;
				break;

			default:
				assert(0);
			}

			s=NumberFormatter::uint64_tToHexString(v);
			if(c.toHexString()!=s){
				printf("ERROR 0x%x %c 0x%x = 0x%s != 0x%s trial %u\n",va,*po,vb,s.c_str(),c.toHexString().c_str(),i);
				break;
			}
		}
	}
}

void testBitwiseOperators(){
	//Note ~ operator isn't good for test ~0=ffffffff ~BigUnsigned(0)=ff if base=unsigned char
	const char O[]="&|^";
	printf("%s() %s\n",__func__,O);
	unsigned i;
	const char*po;
	uint64_t va,vb,v;
	std::string s;
	BigUnsigned a,b,c;
	for(po=O;*po!=0;po++){
		for(i=0;i<NUMBER_OF_TRIALS;i++){
			generateRandom(va);
			generateRandom(vb);

			v=va;
			a=va;
			b=vb;
			switch(*po){
			case '&':
				v&=vb;
				c=a&b;
				break;

			case '|':
				v|=vb;
				c=a|b;
				break;

			case '^':
				v^=vb;
				c=a^b;
				break;

			default:
				assert(0);
			}
			s=NumberFormatter::uint64_tToHexString(v);
			if(c.toHexString()!=s){
				printf("ERROR 0x%s %c 0x%s = 0x%s != 0x%s trial %d\n",
						NumberFormatter::uint64_tToHexString(va).c_str(),
						*po,
						NumberFormatter::uint64_tToHexString(vb).c_str(),
						s.c_str(),
						c.toHexString().c_str(),i);
				break;
			}
		}
	}
}

void testShiftOperators(){
	const char O[]="><";
	printf("%s() << >>\n",__func__);
	unsigned i,j;
	const char*po;
	uint32_t va;
	uint64_t v;
	std::string s;
	BigUnsigned a,c;
	for(po=O;*po!=0;po++){
		for(i=0;i<NUMBER_OF_TRIALS;i++){
			generateRandom(va);

			j=rand()%32;

			v=va;
			a=va;

			switch(*po){
			case '<':
				v<<=j;
				c=a<<j;
				break;

			case '>':
				v>>=j;
				c=a>>j;
				break;

			default:
				assert(0);
			}
			s=NumberFormatter::uint64_tToHexString(v);
			if(c.toHexString()!=s){
				printf("ERROR 0x%x %c%c %d = 0x%s != 0x%s trial %d\n",va,*po,*po,j, s.c_str(),c.toHexString().c_str(),i);
				break;
			}
		}
	}
}

void testHexToDecString(){
	printf("%s()\n",__func__);
	unsigned i;
	uint64_t v;
	std::string s,b;
	for(i=0;i<NUMBER_OF_TRIALS;i++){
		generateRandom(v);
		b=NumberFormatter::uint64_tToHexString(v);
		s=BigUnsigned::hexToDecString("0x"+b);
		b=NumberFormatter::uint64_tToString(v);
		if(s!=b){
			printf("ERROR %s != %s trial %d\n",s.c_str(),b.c_str(),i);
			break;
		}
	}
}

void testHexToDecStringLong(const int len){
	printf("%s()\n",__func__);
	unsigned i;
	std::string s,q;
	BigUnsigned a;
	for(i=0;i<NUMBER_OF_TRIALS;i++){
		s="0x"+generateRandomHexString(len,false);
		a=s;
		q=BigUnsigned::hexToDecString(s.c_str());
		s=a.toDecString();
		if(s!=q){
			printf("ERROR %s != %s trial %d\n",s.c_str(),q.c_str(),i);
			break;
		}
	}
}

void test(const int len){
	printf("%s()\n",__func__);
//	unsigned i;
//	std::string s,q;
//	for(i=0;i<NUMBER_OF_TRIALS;i++){
//		s=generateRandomHexString(len,false);
//		q=BigUnsigned::hexToDecString(s.c_str());
//	}

//	http://www.tsm-resources.com/alists/fact.html
		std::string s="933262154439441526816992388562667004907159682643816214685929638952175999932299156089414639761565182862536979208272237582511852109168640000000000000000000000";
		printf("%d\n",BigUnsigned::factorial(99).toDecString().c_str()==s);

		printf("%d\n",BigUnsigned::pow(2,222).toDecString()=="6739986666787659948666753771754907668409286105635143120275902562304" );
		printf("%s\n",BigUnsigned::pow(2,10).toDecString(6).c_str());
		printf("%s\n",BigUnsigned::pow(2,15).toDecString().c_str());
		printf("%s\n",BigUnsigned::pow(2,16).toDecString().c_str());

}

#endif /* BIGUNSIGNEDTEST_H_ */

