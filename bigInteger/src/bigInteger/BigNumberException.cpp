/*
 * BigNumberException.cpp
 *
 *  Created on: 20.01.2016
 *      Author: alexey slovesnov
 */

#include "BigNumberException.h"

std::ostream& operator<<(std::ostream& o, BigNumberException const& e){
	return o<<e.what();
}




