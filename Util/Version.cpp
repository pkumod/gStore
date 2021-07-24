#include "Version.h"


Version::Version()
{
	data = (TYPE_ENTITY_LITERAL_ID *)malloc(sizeof(TYPE_ENTITY_LITERAL_ID) * init_size);
	data[0] = init_size;
	for(int i = 1; i < 3; i++) data[i] = 0;
	
	this->begin_ts = INVALID_TS;
	this->end_ts = INVALID_TS;
}

Version::Version(TYPE_TXN_ID _begin_ts, TYPE_TXN_ID _end_ts)
{
	data = (TYPE_ENTITY_LITERAL_ID *)malloc(sizeof(TYPE_ENTITY_LITERAL_ID) * init_size);
	data[0] = init_size;
	for(int i = 1; i < 3; i++) data[i] = 0;

	this->begin_ts = _begin_ts;
	this->end_ts = _end_ts;
}

// Version& Version::operator=(const Version& V)
// {
// 	this->begin_ts = V.get_begin_ts();
// 	this->end_ts = V.get_end_ts();
// 	return *this;
// }

void Version::expand(int num)
{
	//cout << "expand " << endl;
	if(4 + 2*(data[1] + data[2] + num)  > data[0]){
        //cout << "expanding...." << endl;
		int size = data[0] * 2;
        while(size < 4 + 2*(data[1] + data[2] + num))
        {
            size = size*2;
        }
        //cout << size << endl;
		TYPE_ENTITY_LITERAL_ID * tmp = (TYPE_ENTITY_LITERAL_ID *)malloc(sizeof(TYPE_ENTITY_LITERAL_ID) * size);
        tmp[0] = size;
		for(int i = 1; i < 4+2*data[1]; i++)
		{
			tmp[i] = data[i];
		}
		for(int i = data[0]-1, j = size - 1; i >= data[0]-1-2*data[2]; i--, j--)
		{
			tmp[j] = data[i];
		}
		free(data);
		data = tmp;
	}
}

void Version::add(VData value)
{
	expand(1);
	data[2*data[1]+4] = value.first;
	data[2*data[1]+5] = value.second;
	data[1]++;
}

void Version::remove(VData value)
{
	expand(1);
	data[data[0]-1-2*data[2]] = value.first;
	data[data[0]-2-2*data[2]] = value.second;
	data[2]++;
}

void Version::batch_add(VDataArray& values)
{
    expand(values.size());
    for(auto value: values)
    {
        data[2*data[1]+4] = value.first;
        data[2*data[1]+5] = value.second;
        data[1]++;
    }
}

void Version::batch_remove(VDataArray& values)
{
    expand(values.size());
    for(auto value: values)
    {
        data[data[0]-1-2*data[2]] = value.first;
        data[data[0]-2-2*data[2]] = value.second;
        data[2]++;
    }
}

void Version::get_add_set(VDataSet& add_set) const
{
	for(int i = 4; i < 2*data[1] + 4; i+=2)
	{
		add_set.insert({data[i], data[i+1]});
	}
}

void Version::get_del_set(VDataSet& del_set) const
{
	for(int i = data[0]-1; i > data[0] - 1- 2*data[2]; i-=2)
	{
		del_set.insert({data[i], data[i+1]});
	}
}

void Version::get_version(VDataSet& add_set, VDataSet& del_set) const
{
	//print_data();
	for(int i = 4; i < 2*data[1] + 4; i+=2)
	{
		VData p = {data[i], data[i+1]};
		auto del_it = del_set.find(p);
		if(del_it == del_set.end()){
			add_set.insert(p);
		}
		else del_set.erase(del_it);
	}


	for(int i = data[0]-1; i > data[0] - 1- 2*data[2]; i-=2)
	{
		VData p = {data[i], data[i-1]};
		auto add_it = add_set.find(p);
		if(add_it == add_set.end())
		{
			del_set.insert(p);
		} 
		else add_set.erase(add_it);
	}
}

void Version::print_data() const
{
	int size = data[0];
	for(int i = 0; i < size; i++)
		cerr << data[i] << " ";
	cerr << endl;
}