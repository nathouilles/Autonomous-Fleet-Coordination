#pragma once

class Trace
{
public:
	Trace(const char*);
	~Trace();

private:
	const char* _txt;
	static int _indent;
	static bool _active;
};

