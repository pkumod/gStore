/*
 * Triple.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef TRIPLE_H_
#define TRIPLE_H_
#include<iostream>
#include<string.h>
using namespace std;

class Triple{
public:
	string subject;
	string predicate;
	string object;

	Triple();

	/* _line responds to a triple
	 * It's OK if _line ends up with '.' or not
	 * tuples separated by '\t'
	 *  */
	Triple(string _line);
	Triple(const string _s, const string _p, const string _o);
	Triple(const Triple& _triple);
	Triple& operator=(const Triple& _triple);

	void setSubject(const string& _s);
	void setPredicate(const string& _p);
	void setObject(const string& _o);

	const string& getSubject()const;
	const string& getPredicate()const;
	const string& getObject()const;

	const string toString()const;
};


#endif /* TRIPLE_H_ */
