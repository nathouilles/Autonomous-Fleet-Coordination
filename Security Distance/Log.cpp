#include "Log.h"
#include <fstream>
using namespace std;

// accesseur a l'unique instance
Log& Log::getLog()
{
	static Log it;
	return it;
}



Log::Log()
	: _file("sgplog.txt")
{
}

Log::~Log()
{
	_file.close();
}

// accesseur au flux de sortie
ostream& Log::it()
{
	return Log::getLog()._file;
}
