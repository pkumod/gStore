
#ifndef _UTIL_IDTRIPLE_H
#define _UTIL_IDTRIPLE_H

#include "Util.h"


class IDTriple
{
private:
	
	TYPE_ENTITY_LITERAL_ID subject;
	TYPE_ENTITY_LITERAL_ID predicate;
	TYPE_ENTITY_LITERAL_ID object;
	
public:
	IDTriple();
	IDTriple(TYPE_ENTITY_LITERAL_ID _subject, TYPE_ENTITY_LITERAL_ID _predicate, TYPE_ENTITY_LITERAL_ID _object)\
	:subject(_subject), predicate(_predicate), object(_object) {}
	~IDTriple() {}
	
	//get and set
	inline const TYPE_ENTITY_LITERAL_ID get_subject() const {return this->subject;}
	inline const TYPE_ENTITY_LITERAL_ID get_predicate() const {return this->predicate;}
	inline const TYPE_ENTITY_LITERAL_ID get_object() const {return this->object;}
	inline void set_subject(TYPE_ENTITY_LITERAL_ID _subject) {this->subject = _subject;}
	inline void set_predicate(TYPE_ENTITY_LITERAL_ID _predicate) {this->predicate = _predicate;}
	inline void set_object(TYPE_ENTITY_LITERAL_ID _object) {this->object = _object;}
	
	
	//compare
	bool operator < (const IDTriple& a) const;
	bool operator > (const IDTriple& a) const;
	bool operator = (const IDTriple& a) const;
	
	
};

#endif
