#pragma once

#include <iosfwd>
using namespace std;


class Time
{
public:
	Time();
	~Time();

	double t() const;
	double dt() const;

	void tick();

	friend ostream& operator<<(ostream&, const Time&);

private:
	double _t, _dt;
};