/*
 * BigNumberException.h
 *
 *  Created on: 17.01.2016
 *      Author: alexey slovesnov
 */

#ifndef BIGNUMBEREXCEPTION_H_
#define BIGNUMBEREXCEPTION_H_

#include <iostream>
#include <exception>
#include <string>

class BigNumberException:std::exception{
	std::string message;
public:
	BigNumberException(const char* file, int line, const char* function,	const char* errorMessage){
		char b[2048];
		sprintf(b,"%s:%d %s [%s]\n",file,line,function,errorMessage);
		message=b;
	}

	virtual const char* what() const throw(){
		return message.c_str();
	}

	virtual ~BigNumberException() throw(){

	}
};

std::ostream& operator<<(std::ostream& , BigNumberException const&);

#define BIGNUMBER_ASSERT(v,message) if(!(v)){throw BigNumberException(__FILE__,__LINE__,__func__,message); }
#define BIGNUMBER_ASSERT_(v) BIGNUMBER_ASSERT(v,#v)

#endif /* BIGNUMBEREXCEPTION_H_ */
