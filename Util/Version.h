#ifndef _UTIL_VERSION_H
#define _UTIL_VERSION_H


#include "Util.h"

using namespace std;

typedef set<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataSet;
typedef pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID> VData;
typedef vector<pair<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> VDataArray;
const int init_size = 16;
class Version
{
private:
	TYPE_ENTITY_LITERAL_ID * data;
	TYPE_TXN_ID begin_ts;
	TYPE_TXN_ID end_ts;
public:
	Version();
	Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts);
	Version& operator=(const Version& V) = delete;
	
	~Version() { free(data); data = nullptr; };
	void add(VData value);
	void remove(VData value);
	void batch_add(VDataArray& values);
	void batch_remove(VDataArray& values);
	void expand(int num);
	void print_data() const;
	//get and set
	TYPE_TXN_ID get_begin_ts() const {return this->begin_ts;};
	TYPE_TXN_ID get_end_ts() const{ return this->end_ts;};

	void get_add_set(VDataSet& add_set) const ;
	void get_del_set(VDataSet& del_set) const ;

	void get_version(VDataSet& add_set, VDataSet& del_set) const ;
	void set_begin_ts(TYPE_TXN_ID _begin_ts){ this->begin_ts = _begin_ts;};
	void set_end_ts(TYPE_TXN_ID _end_ts){ this->end_ts = _end_ts;};
}__attribute__ ((aligned (8)));


#endif
