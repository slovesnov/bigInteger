/*
 * main.cpp
 *
 *  Created on: ??.12.2016
 *      Author: alexey slovesnov
 */


#include <stdio.h>
#include <time.h>

void sumPower_bernoulli();

int main (int argc, char *argv[]){
	clock_t begin = clock();

	sumPower_bernoulli();

	printf("time %.3lf, the end\n",double(clock() - begin)/CLOCKS_PER_SEC);

  return 0;
}

