#ifndef _UTIL_VERSION_H
#define _UTIL_VERSION_H


#include "Util.h"

using namespace std;

typedef set<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataSet;
typedef pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID> VData;
typedef vector<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataArray;
class Version
{
private:
	VDataArray add_set;
	VDataArray del_set;
	TYPE_TXN_ID begin_ts;
	TYPE_TXN_ID end_ts;
public:
	Version();
	Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts);
	Version& operator=(const Version& V);
	
	~Version() { add_set.clear(); del_set.clear();};
	void add(VData value);
	void remove(VData value);
	
	//get and set
	TYPE_TXN_ID get_begin_ts() const {return this->begin_ts;};
	TYPE_TXN_ID get_end_ts() const{ return this->end_ts;};

	const VDataArray& get_add_set() const {return this->add_set;};
	const VDataArray& get_del_set() const {return this->del_set;};
		   
	void set_begin_ts(TYPE_TXN_ID _begin_ts){ this->begin_ts = _begin_ts;};
	void set_end_ts(TYPE_TXN_ID _end_ts){ this->end_ts = _end_ts;};
}__attribute__ ((aligned (8)));


#endif
