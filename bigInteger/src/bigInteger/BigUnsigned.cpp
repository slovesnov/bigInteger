/*
 * BigUnsigned.cpp
 *
 *  Created on: 07.01.2016
 *      Author: alexey slovesnov
 */

#include "BigUnsigned.h"
#include <cstring>//memset
#include <cmath>//log10
#include <cstdlib>//strtoul
#include <sstream>

std::string BigUnsigned::formatString(std::string const& b, const unsigned positions,
		const char separator) {
	if(positions==0){
		return b;
	}
	std::string s;
	unsigned i;
	const char*p=b.c_str();
	for(i=b.length()-1;*p!=0;p++,i--){
		s+=*p;
		if(i%positions==0 && i!=0){
			s+=separator;
		}
	}
	return s;
}

void BigUnsigned::allocate(unsigned newSize) {
	if(size!=newSize){
		free();
		size=newSize;
		data=new base[size];
	}
	BIGNUMBER_ASSERT_(data!=NULL);
}

void BigUnsigned::allocateCopy(unsigned newSize,unsigned copySize){
	base*o=data;
	size=newSize;
	data=new base[size];
	memcpy(data,o,copySize*sizeof(base));
	delete[]o;
}

void BigUnsigned::free(){
	if(data!=NULL){
		delete[]data;
	}
}

BigUnsigned const& BigUnsigned::operator=(const uint64_t& k){
	uint64_t v=k;
	unsigned i;
	if(k==0){
		i=1;
	}
	else{
		for(i=0;v!=0;i++){
			v>>=BASE_BITS;
		}
	}
	allocate(i);
	base*p=data;
	v=k;
	for(i=0 ; i<size ; i++,p++){
		*p=base(v);
		v>>=BASE_BITS;
	}
	return *this;
}

BigUnsigned const& BigUnsigned::operator=(const char* _s) {
	const unsigned sz=2*sizeof(base);//every byte two hex digits
	unsigned i,j,k,l;
	const char* ps;

	//decimal recognizer
	//max uint64_t 18,446,744,073,709,551,615=18 446744 073709 551615
	//allow takes max 19 digits
	const unsigned digits=19;
	std::string ss;
	char b[std::max(sz,digits)+1];
	bool hex;
	ss=prepareString(_s,hex);//throws exception on error
	i=ss.length();
	if(hex){
		j=i%sz;

		l=i/sz;
		if(j==0){
			j=sz;
		}
		else{
			l++;
		}

		ps=ss.c_str();
		//search first nonzero data
		for(  ; *ps!=0 ; l--){
			i = ps==ss.c_str() ? j : sz;
			strncpy(b,ps,i);
			b[i]=0;
			ps+=i;
			k=strtoul(b,NULL,16);
			if(k!=0){
				break;
			}
		}
		if(l==0){
			return *this=0;
		}
		allocate(l);
		base*p=data+size-1;
		*p--=k;
		for( ; *ps!=0 ; p--){
			strncpy(b,ps,sz);
			b[sz]=0;
			*p=strtoul(b,NULL,16);
			ps+=sz;
		}
	}
	else{//decimal string
		uint64_t pow10i,pow10digits=pow10(digits);
		j=i/digits;
		k=i%digits;
		if(k==0){
			pow10i=pow10digits;
		}
		else{
			j++;
			pow10i=pow10(k);
		}

		*this=0;
		b[digits]=0;

		for(k=0,ps=ss.c_str() ; k<j ; k++,ps+=digits ){
			*this *= k==j-1 ? pow10i : pow10digits;
			strncpy(b,ps,digits);
			*this += strtoull(b,NULL,10);
		}
	}
	return *this;
}

std::string BigUnsigned::toHexString(const unsigned positions, const char separator)const{
	if(size==0){
		return "";
	}

	std::string s;
	char b[32];
	base*p=data+size-1;
	do{
		if(p==data+size-1){
			sprintf(b,"%x",*p);
		}
		else{
			sprintf(b,"%0*x",int(sizeof(base))*2,*p);//every byte = two hex digits
		}
		s+=b;
	}while(p--!=data);
	return formatString(s,positions,separator);
}

BigUnsigned BigUnsigned::operator+(BigUnsigned const& u)const{
	BigUnsigned r;
	unsigned i;
	unsigned sz;
	base*p,*pr,*p1;
	if(size>=u.size){
		r.allocate(size);
		p=u.data;
		p1=data;
		sz=u.size;
	}
	else{
		r.allocate(u.size);
		p=data;
		p1=u.data;
		sz=size;
	}
	pr=r.data;
	bool over=false;
	doubleBase d;
	for(i=0;i<sz;i++,p++,p1++,pr++){
		d=*p;
		d+=*p1;
		if(over){
			d++;
		}
		over = d>BASE_MAX;
		*pr=d;
	}
	for(i=0;i<r.size-sz;i++,p1++,pr++){
		d=*p1;
		if(over){
			d++;
		}
		over = d>BASE_MAX;
		*pr=d;
	}
	if(over){//extends
		r.allocateCopy(r.size+1,r.size);
		r.data[r.size-1]=1;
	}
	return r;

}

BigUnsigned BigUnsigned::operator-(BigUnsigned const& u)const{
	BIGNUMBER_ASSERT(*this>=u , "try to subtract bigger number");
	if(*this==u){
		return 0;
	}

	BigUnsigned r(*this);
	r.sub(u);
	r.removeLeadingZeros();
	return r;
}

BigUnsigned BigUnsigned::operator*(BigUnsigned const& u)const{
	unsigned i,j,k;
	base*p,*p1,*p2,*pu;
	doubleBase d,d1;
	BigUnsigned r;
	r.allocate(size+u.size);
	memset(r.data,0,r.size*sizeof(base));
	for( p=data,i=0 ; i<size ; i++,p++ ){
		for( pu=u.data,j=0 ; j<u.size ; j++,pu++ ){
			d=*p;
			d*=*pu;
			//2^(i+j)*BASE_BITS
			p1=r.data+i+j;
			for(k=0;;k++,p1++){
				d1=*p1;
				d1+=base(d);
				*p1=d1;
				if(d1>BASE_MAX){
					for(p2=p1+1 ; ++(*p2) == 0 ; p2++);
				}

				if(k==1){
					break;
				}
				d>>=BASE_BITS;
			}
		}
	}

	r.removeLeadingZeros();
	return r;
}

BigUnsigned BigUnsigned::operator>>(unsigned k)const{
	unsigned i=k/BASE_BITS;
	if(i>=size){
		return 0;
	}
	unsigned j=k%BASE_BITS;
	base*p=data+i;

	bool b=false;
	if(data[size-1]>>j==0){//highest item gives 0 after shift
		i++;
		if(size==i){
			return 0;
		}
		b=true;
	}
	BigUnsigned r;
	r.allocate(size-i);
	base*pr=r.data;

	for(i=0; i<size-b ; i++,p++,pr++ ){
		*pr = *p>>j ;
		if(p!=data+size-1 && j!=0){//shift on BASE_BITS doesn't good work
			*pr |= (*(p+1))<<(BASE_BITS-j);
		}
	}

	return r;
}

BigUnsigned BigUnsigned::operator<<(unsigned k)const{
	BigUnsigned r;
	if(k==0){
		return r=*this;
	}

	//Note is *this==0 we should not extend data with 'allocate' as we do below, so result is always 0 without extension
	if(*this==0){
		r=*this;
		return r;
	}

	unsigned i=k/BASE_BITS;
	unsigned j=k%BASE_BITS;
	unsigned l;
	base*p=data;

	int add;
	if(j==0){
		add=0;
	}
	else{
		//shift on BASE_BITS doesn't good work
		add=data[size-1]>>(BASE_BITS-j)!=0;
	}
	r.allocate(size+i+add);
	base*pr=r.data;

	for(l=0; l<i ; l++,pr++ ){
		//assert(pr<r.data+r.size);
		*pr=0;
	}

	for(; l<r.size-add ; l++,p++,pr++ ){
		//assert(pr<r.data+r.size);
		//assert(p<data+size);
		*pr = *p<<j ;
		if(p!=data && j!=0){//shift on BASE_BITS doesn't good work
			//assert(pr<r.data+r.size);
			//assert(p-1<data+size);
			*pr |= (*(p-1))>>(BASE_BITS-j);
		}
	}
	if(add && j!=0){//shift on BASE_BITS doesn't good work
		//assert(p-1<data+size);
		*pr = (*(p-1))>>(BASE_BITS-j);
	}

	return r;

}

int BigUnsigned::compareTo(const BigUnsigned& u) const {
	if(size==u.size){
		return compareSameSize(u);
	}
	else{
		return size>u.size ? 1:-1;
	}
}

int BigUnsigned::compareSameSize(const BigUnsigned& u) const {
	base*p=data+size-1;
	base*pu=u.data+u.size-1;
	for(;p>=data;p--,pu--){
		if(*p!=*pu){
			return *p>*pu ? 1:-1;
		}
	}
	return 0;
}

void BigUnsigned::removeLeadingZeros() {
	base*p=data+size-1;
	unsigned i;
	for(i=0; i<size && *p==0 ; i++,p--);
	if(i==size){
		*this=0;
	}
	else if(i!=0){
		allocateCopy(size-i,size-i);
	}
}

BigUnsigned BigUnsigned::operator &(const BigUnsigned& u) const {
	unsigned i= size>=u.size ? u.size : size;
	base*p=data+i-1;
	base*pu=u.data+i-1;
	for(; p>=data && (*p&*pu)==0 ; p--,pu--,i--);

	if(p<data){
		return 0;
	}

	BigUnsigned r;
	r.allocate(i);
	base*pr=r.data+r.size-1;
	for(;p>=data;p--,pu--,pr--){
		*pr=*p&*pu;
	}
	return r;
}

BigUnsigned BigUnsigned::operator |(const BigUnsigned& u) const {
	BigUnsigned r;
	base*p;
	unsigned i;
	if(size>=u.size){
		r=*this;
		p=u.data;
		i=u.size;
	}
	else{
		r=u;
		p=data;
		i=size;
	}
	base*pr=r.data;
	for(;i>0;p++,pr++,i--){
		*pr|=*p;
	}
	return r;
}

BigUnsigned BigUnsigned::operator ^(const BigUnsigned& u) const {
	unsigned i;
	base*p,*pu,*pr;
	BigUnsigned r;
	if(size==u.size){
		i= size>=u.size ? u.size : size;
		p=data+i-1;
		pu=u.data+i-1;
		for(; p>=data && (*p^*pu)==0 ; p--,pu--,i--);

		if(p<data){
			return 0;
		}
		r.allocate(i);
		pr=r.data+r.size-1;
		for(;p>=data;p--,pu--,pr--){
			*pr=*p^*pu;
		}
	}
	else{
		if(size>u.size){
			r=*this;
			pu=u.data;
			p=pu+u.size;
		}
		else{
			r=u;
			pu=data;
			p=pu+size;
		}
		pr=r.data;
		for(;pu<p;pu++,pr++){
			*pr^=*pu;
		}
	}

	return r;
}

uint64_t BigUnsigned::pow10(const unsigned k) {
	uint64_t r=1;
	for(unsigned i=0;i<k;i++){
		r*=10;
	}
	return r;
}

BigUnsigned BigUnsigned::factorial(unsigned n) {
	if(n<2){
		return 1;
	}
	BigUnsigned r(2);
	for(unsigned i=3;i<=n;i++){
		r*=i;
	}
	return r;
}

BigUnsigned BigUnsigned::factorial()const {
	if(*this<2){
		return 1;
	}
	BigUnsigned r(2);
	for(uint64_t i=3;i<=*this;i++){
		r*=i;
	}
	return r;
}

void BigUnsigned::sub(const BigUnsigned& u) const {
	base*pr=data;
	base*pu=u.data;
	base*p;
	for(;pu!=u.data+u.size;pr++,pu++){
		if(*pr>=*pu){
			*pr-=*pu;
		}
		else{
			*pr+=(doubleBase(1)<<BASE_BITS)-*pu;
			for(p=pr+1;;p++){
				if( --(*p) != BASE_MAX ){
					break;
				}
			}
		}
	}
}

std::string BigUnsigned::prepareString(const char* s, bool& hex) {
	const char* p=s;
	bool zeroOnly=false;
	if(*p=='+'){
		p++;
	}
	else if(*p=='-'){//allows "-0,000" or "-0x000"
		zeroOnly=true;
		p++;
	}

	if(*p=='0' && tolower(*(p+1))=='x'){
		hex=true;
		p+=2;
	}
	else{
		hex=false;
	}
	for(;*p=='0' || *p==DEFAULT_SEPARATOR;p++);
	if(*p==0){
		return "0";//only zeros and DEFAULT_SEPARATORs
	}
	if(zeroOnly){
		BIGNUMBER_ASSERT(0,"nonzero string starts with minus char");
	}
	std::string r;
	for(;*p!=0;p++){
		if(isdigit(*p) || (hex && ((*p>='a' && *p<='f') || (*p>='A' && *p<='F')) ) ){
			r+=*p;
		}
		else if(*p!=DEFAULT_SEPARATOR){
			BIGNUMBER_ASSERT(0,hex ? "invalid symbol in hex string": "invalid symbol in decimal string");
		}
	}
	return r;
}


BigUnsigned BigUnsigned::operator ~() const {
	BigUnsigned r(*this);
	base*pr=r.data+r.size-1;
	for(;pr>=r.data;pr--){
		*pr=~*pr;
	}
	return r;
}

std::string BigUnsigned::hexToDecString(const char* _s, const unsigned positions, const char separator) {
/*
 * readdigits read max sizeof(uint64_t)*2 hex digits
 *
 * COUNT n = number of items to store string
 * 1 item  0<= value <=pow10(storedigits)-1
 * n items 0<= value <=pow10(storedigits*n)-1
 *
 * hex string
 * 1 char  0<= value <=16-1
 * length chars 0<= value <=16^length-1
 *
 * pow10(storedigits*n)-1 >= 16^length-1
 * storedigits*n>=log10(16)*length
 * n>=log10(16)*length/storedigits
 *
 * COUNT storedigits
 * 10^storedigits-1 <= max_uint (uint our store type)
 * max_uint = 2^(sizeof(uint)*8)-1
 * storedigits <= sizeof(uint)*8*log10(2)
 *
 * max_uint64_t 18,446,744,073,709,551,615=18 446744 073709 551615
 */
	bool hex;
	std::string ps=prepareString(_s,hex);//throws exception on error
	if(ps=="0"){
		return ps;
	}
	const char*s=ps.c_str();

	typedef uint32_t uint;
	const int storedigits=sizeof(uint)*8*log10(2);//9.63 - for uint32_t
	const unsigned readdigits=sizeof(uint64_t)*2;
	const uint64_t P10=pow10(storedigits);
	const uint64_t N[]={18,446744073,709551616};//2^64=18,446744073,709551616 digits of each item <= storedigits nine digits

	uint*p,*p1,*high;//high non zero data
	std::string sdec;
	char b[32];
	bool over;
	uint64_t v,k=strlen(s);
	unsigned l,i=k*log10(16)/storedigits+1;
	uint q;
	uint64_t vv;
	uint*data=new uint[i];

	memset(data,0,i*sizeof(uint));
	i=k%readdigits;
	if(i==0){
		i=readdigits;
	}
	b[i]=0;
	k=strtoull(strncpy(b,s,i),NULL,16);
	s+=i;

//value=k
	for(p=data;k!=0;k/=P10,p++){
		*p=k%P10;
	}
	high=p-1;

	b[readdigits]=0;
	for(;*s!=0;s+=readdigits){
		//value*=2^(4*readdigits)
		for(p=high ; p>=data ; p--){
			vv=*p;
			*p=0;
			for(l=0;l<3;l++){
				k=vv*N[2-l];

				for(i=0;i<2;i++){
					if(i==0){
						q=k%P10;
					}
					else{
						q=k/P10;
					}
					if(q!=0){
						p1=p+l+i;
						v=*p1;
						v+=q;
						if(v>=P10){
							*p1=v-P10;
							for(p1++;;p1++){
								if(*p1==P10-1){
									*p1=0;
								}
								else{
									break;
								}
							}
							(*p1)++;
						}
						else{
							*p1=v;
						}
						if(high<p1){
							high=p1;
						}
					}
				}
			}
		}

		//value+=k
		k=strtoull(strncpy(b,s,readdigits),NULL,16);
		for(over=false,p=data ; k!=0 ; k/=P10,p++){
			v=*p;
			v+=k%P10;
			if(over){
				v++;
			}
			*p=v%P10;
			over=v>=P10;
		}
		if(over){
			for( ; *p==P10-1 ; p++ ){
				*p=0;
			}
			(*p)++;
		}
		else{
			p--;
		}
		if(high<p){
			high=p;
		}

	}

	for(;high>=data;high--){
		if( sdec.empty() ){
			sprintf(b,"%u",*high);
		}
		else{
			sprintf(b,"%0*u",storedigits,*high);
		}
		sdec+=b;
	}

	delete[]data;
	return formatString(sdec,positions,separator);
}

void BigUnsigned::div(const BigUnsigned& divisor, BigUnsigned& quotient,
		BigUnsigned& remainder) const {

	//works with W defined or not, time approximate the same
	//#define W

	BIGNUMBER_ASSERT( divisor!=0 , "zero division");

	int j=compareTo(divisor);

	if(j==-1){
		quotient=0;
		remainder=*this;
		return;
	}
	else if(j==0){
		quotient=1;
		remainder=0;
		return;
	}
	if(divisor==1){
		quotient=*this;
		remainder=0;
		return;
	}

	BigUnsigned v[256];
	unsigned i,l,h;
	bool b;
	typedef unsigned char* pchar;
	pchar p,p1,p2;

#ifdef W
	v[255]=(divisor<<8)-divisor;

	l=v[255].size;
	for(i=0;i<255;i++){//v[255] already done
		v[i].allocate(l);
	}

	if(l>divisor.size){
		for(i=0;i<255;i++){
			v[i].data[l-1]=0;
		}
	}

	memset(v[0].data,0,l*sizeof(base));//v[0]=0
	memcpy(v[1].data,divisor.data,divisor.size*sizeof(base));//v[1]=divisor

	for(i=2;i<255;i++){
		v[i]=v[i-1]+divisor;
	}
	remainder.allocate(l);
#else
	v[0]=0;
	v[1]=divisor;
	for(i=2;i<256;i++){
		v[i]=v[i-1]+divisor;
	}
	//remainder.allocate();later
#endif

	if(sizeof(base)==1){
		h=divisor.size;
		p1=pchar(divisor.data+divisor.size)-1;
		p=pchar(data+size)-1;
	}
	else{
		h=divisor.size*sizeof(base);
		for(p1=pchar(divisor.data+divisor.size)-1 ; p1>=pchar(divisor.data) && *p1==0 ; p1--,h--);
		for(p=pchar(data+size)-1 ; p>=pchar(data) && *p==0 ; p--);
	}
	p2=p;

	for(;p1>=pchar(divisor.data);p--,p1--){
		if(*p!=*p1){
			break;
		}
	}

	if(p1!=pchar(divisor.data)-1 && *p<*p1){//p1!=pchar(divisor.data)-1 the same
		h++;
	}

#ifndef W
	remainder.allocate( (h+sizeof(base)-1)/sizeof(base) );
#endif

	memcpy( remainder.data , p2-h+1 , h );
	memset( pchar(remainder.data)+h , 0 , remainder.size*sizeof(base)-h );

	const int steps=size*sizeof(base) - (pchar(data+size)-p2-1+h)+1 ;
	quotient.allocate( (steps+sizeof(base)-1)/sizeof(base) );

	p1=pchar(quotient.data)+steps-1;
	p=p2-h;

	memset(p1+1,0,quotient.size*sizeof(base)-steps);//recent bytes=0

	for(;;p--){
		//binary search
		l=0;
		h=256;
		b=false;
		for( ; l+1!=h ; ){
			i=(l+h)>>1;
#ifdef W
			j=remainder.compareSameSize(v[i]);
#else
			j=remainder.compareTo(v[i]);
#endif

			if(j==-1){
				h=i;
			}
			else if(j==1){
				l=i;
			}
			else{
				b=true;
				l=i;
				break;
			}
		}

		if(b){
#ifdef W
			//remainder=0;cann't use =operator, because we don't need removeLeadingZeros
			memset(remainder.data,0,remainder.size*sizeof(base));
#else
			remainder=0;
#endif
		}
		else{
#ifdef W
			//remainder -=v[l];cann't use - operator, because we don't need removeLeadingZeros
			remainder.sub(v[l]);
#else
			remainder -=v[l];
#endif
		}

		*p1--=l;
		if(p<pchar(data) ){
			break;
		}
		if(b){
			p2=pchar(remainder.data);
		}
		else{

#ifdef W
			//remainder <<= 8;cann't use shift, because we want that size will be the same
			for(p2=pchar(remainder.data+remainder.size)-1;p2!=pchar(remainder.data);p2--){
				*p2=*(p2-1);
			}
#else
			remainder <<= 8;
			p2=pchar(remainder.data);
#endif


		}

		*p2=*p;

	}

#ifdef W
	remainder.removeLeadingZeros();
#endif

#undef W
}

BigUnsigned BigUnsigned::pow(unsigned exponent) const {
	if(exponent==1){
		return *this;
	}
	if(exponent==0){
		BIGNUMBER_ASSERT(*this!=0 , "zero to power zero");
		return 1;
	}

	unsigned i,j;
	for(i=0;i<32;i++){
		if( (exponent & 1<<(31-i))!=0 ){
			break;
		}
	}

	i=32-i;
	BigUnsigned* a=new BigUnsigned[i];
	BigUnsigned*pa=a;
	*pa=*this;
	for(j=1;j<i;j++,pa++){
		*(pa+1)=*pa*(*pa);
	}
	BigUnsigned r(*pa);
	for(pa=a,j=0;j<i-1;j++,pa++){
		if( (exponent & (1<<j) )!=0 ){
			r*=*pa;
		}
	}

	delete[]a;
	return r;
}

uint64_t BigUnsigned::toUint64_t() const {
	int i;
	BIGNUMBER_ASSERT(size*sizeof(base)<=sizeof(uint64_t), "too big value to convert");

	uint64_t r=0;
	for (i = 0; i <int(size); i++) {
		r += data[i] * (1ull<<(sizeof(base) * 8*i));
	}
	return r;
}

double BigUnsigned::toDouble() const {
	int i;
	double v = 0;
	for (i = 0; i <int(size); i++) {
		v += data[i] * ::pow(2, sizeof(base) * 8*i);
	}
	return v;
}

long double BigUnsigned::toLongDouble()const{
	int i;
	long double v = 0;
	for (i = 0; i <int(size); i++) {
		v += data[i] * ::powl(2, sizeof(base) * 8*i);
	}
	return v;
}


std::pair<double, int> BigUnsigned::getMantissaExponent() const {
	double v = toDouble();
	if (v == 0) {
		return {0,0};
	}
	double exponent = floor(log10(v));
	return {v / ::pow(10, exponent),exponent};
}

std::pair<double, int> BigUnsigned::getMantissaExponentLongDouble() const {
	long double v = toLongDouble();
	if (v == 0) {
		return {0,0};
	}
	long double exponent = floorl(log10l(v));
	return {v / ::powl(10, exponent),exponent};
}

BigUnsigned BigUnsigned::binomial(unsigned k,unsigned n){
	BigUnsigned r = 1;
	unsigned i;
	/* for big n,k
	 * C(n,k)=n*C(n-1,k-1)/k
	 * C(n,k)=n*C(n-1,k-1)/k=(n/k)*(n-1/k-1)...(n-k+1/1)C(n-k,0); C(n-k,0)=1
	 */
	for (i = 1; i <= k; i++) {
		r *= n - k + i;
		r /= i;
	}
	return r;
}

std::ostream& operator<<(std::ostream& o, BigUnsigned const& u){
	return o<<u.toString();
}

#define M(o,type) BigUnsigned operator o(const type t,BigUnsigned const& u){return BigUnsigned(t) o u;}
#define N(o) M(o,char*)M(o,std::string&)M(o,uint64_t&)
N(+)N(-)N(*)N(/)N(%)
N(&)N(|)N(^)
#undef N
#undef M


/*
 * boolean operators
 * cann't use bool operator<(const type t,BigUnsigned const& u){return BigUnsigned(t) < u;}
 * because "-6"<any BigUnsigned but BigUnsigned("-6") it's error
 */
#define M(o,o1,type) bool operator o(const type t,BigUnsigned const& u){return u o1 t;}

#define N(o,o1) M(o,o1,int&)M(o,o1,unsigned&)\
	M(o,o1,int64_t&)M(o,o1,uint64_t&)\
	M(o,o1,char*)M(o,o1,std::string&)

N(==,==)
N(!=,!=)
N(<,>)
N(<=,>=)
N(>,<)
N(>=,<=)
#undef N

#undef M

