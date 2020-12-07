#include "Version.h"


Version::Version()
{
	this->begin_ts = INVALID_TS;
	this->end_ts = INVALID_TS;
}

Version::Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts)
{
	this->begin_ts = _begin_ts;
	this->end_ts = _end_ts;
}

Version& Version::operator=(const Version& V)
{
	this->begin_ts = V.get_begin_ts();
	this->end_ts = V.get_end_ts();
	V.get_add_set(this->add_set);
	V.get_del_set(this->del_set);
	return *this;
}

void Version::add(VData value)
{
	auto it = this->del_set.find(value);
	if(it == this->del_set.end())
		this->add_set.insert(value);
	else
		this->del_set.erase(it);
}

void Version::remove(VData value)
{
	auto it = this->add_set.find(value);
	if(it == this->add_set.end())
		this->del_set.insert(value);
	else
		this->add_set.erase(it);
}

void Version::get_add_set(VDataSet& _addset) const
{
	_addset = this->add_set;
}
void Version::get_del_set(VDataSet& _delset) const
{
	_delset = this->del_set;
}
	