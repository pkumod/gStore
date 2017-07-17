class KEY_SIZE_VALUE
{
public:
	unsigned key, size, value;
	KEY_SIZE_VALUE(unsigned _key = 0, unsigned _size = 0, unsigned _value = 0) : key(_key), size(_size), value(_value) {}
	bool operator < (const KEY_SIZE_VALUE& b) const
	{
		return value > b.value;
	}
	bool operator > (const KEY_SIZE_VALUE& b) const
	{
		return value < b.value;
	}

};

class Longlist_inMem
{
public:
	int key;
	unsigned _len;
	char* _str;
	Longlist_inMem()
	{
		key = -1;
		_len = 0;
		_str = NULL;
	}
	~Longlist_inMem()
	{
		if(_str != NULL)
		{
			delete [] _str;
			_str = NULL;
		}
	}
};
