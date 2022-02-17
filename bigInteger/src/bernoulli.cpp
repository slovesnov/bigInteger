/*
 * bernoulli.cpp
 *
 *  Created on: 23.01.2016
 *      Author: alexey slovesnov
 */

#include <cassert>

#include "bigInteger/Fraction.h"
#include "bigInteger/BigInteger.h"

//#define USE_UINT64

#ifdef USE_UINT64
	typedef uint64_t basetype;
	typedef int64_t signedBasetype;
	const int MAXC=67;//C^31_68 > 2^64-1
#else
	typedef BigUnsigned basetype;
	typedef BigInteger signedBasetype;
	const int MAXC=67;//can use big values
#endif

typedef Fraction<basetype> Frac;

basetype C[MAXC+1][MAXC+1];//C[i][j]=C^i_j=(i+j)!/i!/j!
basetype POWER[MAXC+1][MAXC+1];//POWER[i][j]=i^j

basetype binomialCoefficient(int i,int j){
	assert(i>=0);
	assert(i<=j);
	//assert(j>=0); automatically true

	assert(j<MAXC+1);
	//assert(i<MAXC+1); automatically true
	return C[i][j];
}

basetype power(int i,int j){
	assert(i>0);
	assert(j>=0);
	assert(i<MAXC+1);
	assert(j<MAXC+1);
	return POWER[i][j];
}

void prepare(){
	int i,j;

	for (i=0; i<MAXC+1; ++i) {
		C[0][i] = C[i][i] = 1;
		for (j=1; j<i; ++j){
			C[j][i] = C[j-1][i-1] + C[j][i-1];
#ifdef USE_UINT64
			if(C[j][i] < C[j-1][i-1] || C[j][i] < C[j][i-1]){
				assert(j-1==MAXC);
				goto out;
			}
#endif
		}
	}
#ifdef USE_UINT64
	out:;
#endif

	for(i=1; i<=MAXC; ++i) {
		POWER[i][0]=1;
		for (j=1; j<MAXC; ++j){
			POWER[i][j]=POWER[i][j-1]*i;
		}
	}

}

// S_k(n) = \sum\limits_{i=1}^{k+1} C^{i}_{n} \sum\limits_{j=0}^{i-1}C^j_{i-1}(i-j)^k (-1)^j
void powerBinomial(int _power){
	int i,j,k;
	signedBasetype l,la;
	basetype bt;

	for(k=0;k<=_power;k++){
		printf("$$ S_%s(n) =",Frac::index(k).c_str());
		for(i=1;i<=k+1;i++){
			for(l=j=0;j<=i-1;j++){
				bt=binomialCoefficient(j,i-1)*power(i-j,k);
				if(j%2==0){
					l+=bt;
				}
				else{
					l-=bt;
				}
			}
			if(l>0){
				if(i!=1){
					printf("+");
				}
			}
			else if(l<0){
				printf("-");
			}

#ifdef USE_UINT64
			la=_abs64(l);
#else
			la=l.abs();
#endif
			if(la!=1 ){
#ifdef USE_UINT64
				printf(" %s",NumberFormatter::uint64_tToString(la).c_str());
#else
				printf(" %s",la.toDecString().c_str());
#endif
			}

			printf(" C^%s_n ",Frac::index(i).c_str());
		}
		printf("$$\n");
	}

}

/**
 *
 * @param power
 * @param type=0 text,type=1 latex,type=2 only coefficients,type=3 only coefficients html
 */
void powerPolynomial(int power,int type){
	assert(power>=0);
	Frac* f;//f^0*n+f[1]*n^2+...+f[power]*n^{power+1} // f[i-1]*n^i , f[i]*n^{i+1}
	int i,j;
	std::string s;

	f=new Frac[power+1];

	for(i=0;i<=power;i++){
		for(j=i-1;j>=0;j--){//f[j]*n^[j+1] -> (n^{j+2}-n)/(j+1)
			f[j+1]=f[j]*Frac(i,j+2);
		}

		f[0]=1;
		for(j=1;j<=i;j++){
			f[0]-=f[j];
		}

		if(type==0 || type==1){
			if(type==1){
				printf("$$ ");
			}
			printf("S_%s(n) = ",Frac::index(i).c_str());

			for(j=i;j>-1;j--){
				if(f[j]==0){
					continue;
				}

				s=f[j].specialString(type==1);
				printf(s.c_str()+(j==i));//skip '+' sign for very first item
				if( type!=1 && s!="+" && s!="-" ){
					printf("*");
				}
				printf("n");
				if(j!=0){
					printf("^");
					printf("%s",Frac::index(j+1).c_str());
				}

			}

			if(type==1){
				printf(" $$");
			}
		}
		else if(type==2 || type==3){
			if(type==2){
				printf("<tr><td>S<sub>%d</sub>(n)",i);
			}
			for(j=i;j>=0;j--){
				printf(type==2 ? "<td>%s" : "%10s" , f[j].toString().c_str());
			}
		}

		printf("\n");
	}
	delete[]f;
}

void bernoulliNumber(int max,int formula){
	/*
	 * formula0 (only even bernoulli numbers) B_{2n}=\frac{1}{(2n+1)(n+1)} \left( n+1-\sum\limits_{j=0}^{n-1} C^{2j}_{2n+2}B_{2j} \right)\quad n>0,
	 * Note formula count good for n<=20 after not enough digits in fraction
	 *
	 * formula1 (only even bernoulli numbers) B_{2n}=\frac{1}{2}-\frac{1}{2n+1}\sum\limits_{j=0}^{n-1} C^{2j}_{2n+1} B_{2j}
	 * Note formula count good for n<=22
	 *
	 * formula2 (all bernoulli numbers) B_n=\sum\limits_{i=1}^{n+1} \frac{1}{i}\sum\limits_{j=1}^{i}C^{j-1}_{i-1}j^n (-1)^{j+1}
	 *
	 * formula3 (all bernoulli numbers) B_k=\frac{1}{k+1} \sum\limits_{i=0}^{k-1} C^i_{k+1} B_i (-1)^{k+i+1} \quad k>0
	 * the same (k->n,i->j) B_n=\frac{1}{n+1} \sum\limits_{j=0}^{n-1} C^j_{n+1} B_j (-1)^{n+j+1}
	 * Note formula count good for n<=22
	 *
	 * formula4 (all bernoulli numbers) Akiyama-Tanigawa triangle
	 * source https://cs.uwaterloo.ca/journals/JIS/VOL3/KANEKO/AT-kaneko.pdf
	 *
	 */

	int n,i,j;
	signedBasetype l;
	basetype l1;

	Frac*f,p;
	Frac*B=new Frac[max+1];

	if(formula==4){
		for(j=0;j<=max;j++){
			B[j]=Frac(1,j+1);
		}
	}
	else{
		B[0]=1;
	}

	for(n=1;n<=(formula>1 ? max:max/2);n++){
		f=B+n;
		switch(formula){
		case 0:
			*f=n+1;
			for(j=0;j<=n-1;j++){
				*f-=B[j]*binomialCoefficient(2*j,2*n+2);
			}
			*f/=((2*n+1)*(n+1));
			break;

		case 1:
			*f=0;
			for(j=0;j<=n-1;j++){
				*f+=B[j]*binomialCoefficient(2*j,2*n+1);
			}
			*f/=2*n+1;
			*f-=Frac(1,2);
			*f=-*f;
			break;

		case 2:
			*f=0;
			for(i=1;i<=n+1;i++){
				for(l=0,j=1;j<=i;j++){
					l1=binomialCoefficient(j-1,i-1)*power(j,n);
					if( j%2==1 ){
						l+=l1;
					}
					else{
						l-=l1;
					}
				}

#ifdef USE_UINT64
				*f+=Frac(l>=0,_abs64(l),i);
#else
				*f+=Frac(l>=0,l.getUnsigned(),i);
#endif
			}

		case 3:
			*f=0;
			for(j=0;j<=n-1;j++){
				p=B[j]*binomialCoefficient(j,n+1);
				if(n+j+1 % 2==0){
					*f+=p;
				}
				else{
					*f-=p;
				}
			}
			*f/=n+1;
			break;

		case 4:
			for(j=max;j>=n;j--){
				B[j]=(B[j-1]-B[j])*(j-n+1);
			}
			break;

		default:
			assert(0);
		}
		if(n%2==0 || formula<2 ){
			printf( " %s", f->toString().c_str());
			fflush(stdout);
		}
	}

	delete[]B;
}

void sumPower_bernoulli(){
	prepare();

	unsigned i;

#ifdef USE_UINT64
	const int MAX=15;
#else
	const int MAX=20;
#endif

	powerBinomial(MAX);//Note if defined USE_UINT64 since S_{16} using binomial coefficients not valid
	powerPolynomial(MAX,1);

#ifdef USE_UINT64
	const int maxb[]={34,34,34,34,30};//max valid values
#else
	const int maxb[]={44,44,44,44,44};//can use big values
#endif

	for(i=0;i<sizeof(maxb)/sizeof(maxb[0]);i++){
		bernoulliNumber(maxb[i],i);
		printf("\n");
	}

}


