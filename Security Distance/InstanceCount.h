#pragma once

#include <map>
#include <string>
using namespace std;

class InstanceCount
{
public:
	InstanceCount(const char*);
	InstanceCount(const InstanceCount&);
	~InstanceCount();

	static void dumpDB();

private:
	const char* _classname;
	static map<string, int> _db;
};

