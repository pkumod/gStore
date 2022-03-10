
#include "IDTriple.h"

bool IDTriple::operator < (const IDTriple& a) const
{
	if(this->subject < a.get_subject()) return true;
	else if(this->subject > a.get_subject()) return false;
	else
	{
		if(this->predicate < a.get_predicate()) return true;
		else if(this->predicate > a.get_predicate()) return false;
		else
		{
			if(this->object <= a.get_object()) return true;
			else if(this->object > a.get_object()) return false;
		}
	}
}


bool IDTriple::operator > (const IDTriple& a) const
{
	return !(*this < a );
}

bool IDTriple::operator = (const IDTriple& a) const
{
	return (this->subject == a.get_subject() && this->predicate == a.get_predicate() && this->object == a.get_object());
}
