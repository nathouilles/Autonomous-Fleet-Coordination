#include "InstanceCount.h"
#include <iostream>


InstanceCount::InstanceCount(const char* name)
	: _classname(name)
{
	static bool done = false;
	if (!done)
	{
		atexit(dumpDB);
		done = true;
	}

	map<string, int>::iterator it = _db.find(_classname);

	if (it == _db.end())
	{
		_db.insert({ name, 1 });
	}
	else
	{
		it->second++;
	}
}


InstanceCount::InstanceCount(const InstanceCount& ic):
	_classname(ic._classname)
{
	map<string, int>::iterator it = _db.find(_classname);

	if (it != _db.end())
		it->second++;
}

InstanceCount::~InstanceCount()
{
	_db[_classname]--;
}

void InstanceCount::dumpDB()
{
	for (map<string, int>::const_iterator it = _db.begin(); it != _db.end(); ++it)
	{
		cout << it->first << " : " << it->second << endl;
	}
}

map<string, int> InstanceCount::_db;