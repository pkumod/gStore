/*=======================================================================
 * File name: ISEntry.h
 * Author: Zongyue Qin
 * Mail: qinzongyue@pku.edu.cn
 * Last Modified: 2018-01-30
 * Description: head file of IVEntry, the most basic unit of IVArray
 * =====================================================================*/

#include "../../Util/Util.h"
#include "../../Util/VList.h"

using namespace std;

class ISEntry
{
	bool usedFlag;   // mark if the entry is used
	bool dirtyFlag;
	bool cacheFlag;
	bool LongListFlag;
	unsigned store;  //index of block where value is stored
	Bstr* value;
public:
	ISEntry();
	
	void setBstr(const Bstr* _value);
	bool getBstr(char *& _str, unsigned& _len) const;
	void setBstr(const char *_str, unsigned _len);
	
	void setStore(unsigned _store);
	unsigned getStore() const;

	void setUsedFlag(bool _flag);
	bool isUsed() const;

	void setDirtyFlag(bool _flag);
	bool isDirty() const;
	
	void setCacheFlag(bool _flag);
	bool inCache() const;

	void setLongListFlag(bool _flag);
	bool isLongList() const;

	void release();

	void Copy(const ISEntry& _entry);

	~ISEntry();
};
