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
	return *this;
}

void Version::add(VData value)
{
	this->add_set.push_back(value);
}

void Version::remove(VData value)
{
	this->del_set.push_back(value);
}
