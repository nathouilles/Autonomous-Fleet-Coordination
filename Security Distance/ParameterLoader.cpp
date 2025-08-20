#include "ParameterLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

ParameterLoader::ParameterLoader(const char* name)
{
	ifstream f(name);
	string first, second;
	while (f >> first >> second)
	{
		_prmtr.insert({ first, second });
	}
}

ParameterLoader::~ParameterLoader()
{

}

int ParameterLoader::getInt(const string& s)
{
	
	string val = it().getString(s);
	istringstream is(val);
	int v = 0;
	is >> v;

	return v;
}

double ParameterLoader::getDouble(const string& s)
{
	string val = it().getString(s);
	istringstream is(val);
	double v = 0.;
	is >> v;
	return v;
}

string ParameterLoader::getString(const string& s)
{
	map<string, string>::const_iterator i = it()._prmtr.find(s);
	if (i != it()._prmtr.end())
	{
		return i->second;
	}
	return string();
}

void ParameterLoader::dump() const
{
	for (map<string, string>::const_iterator i = it()._prmtr.begin(); i != it()._prmtr.end(); ++i)
	{
		cout << i->first << " : " << i->second << endl;
	}
}

ParameterLoader& ParameterLoader::it()
{
	static ParameterLoader pl("SimulFleet.ini");
	return pl;
}
