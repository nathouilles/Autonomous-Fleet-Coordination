#pragma once

#include <fstream>
using namespace std;

class Log
{
public:
	// accesseur au flux de sortie
	static ostream& it();


private:
	Log();
	~Log();

	// accesseur a l'unique instance
	static Log& getLog();	
	
	ofstream _file;
};

