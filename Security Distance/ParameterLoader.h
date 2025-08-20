#pragma once

#include <map>
#include <string>

class ParameterLoader
{
public:

	static int getInt(const std::string&);
	static double getDouble(const std::string&);
	static std::string getString(const std::string&);

private:
	ParameterLoader(const char*);
	virtual ~ParameterLoader();
	void dump() const;

	static ParameterLoader& it();

	ParameterLoader(const ParameterLoader&) = delete;
	ParameterLoader& operator=(const ParameterLoader&) const = delete;

	std::map<std::string, std::string> _prmtr;
};

