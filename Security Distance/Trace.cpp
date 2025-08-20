#include "Trace.h"

#include <iostream>
using namespace std;

int Trace::_indent = 0;
bool Trace::_active = false;

Trace::Trace(const char* txt)
	: _txt(txt)
{
	if (!_active) return;
	for (int i = 0; i < _indent; i++)
		cout << " ";
	cout << ">>" << _txt << endl;
	++_indent;
}

Trace::~Trace()
{
	if (!_active) return;
	--_indent;
	for (int i = 0; i < _indent; i++)
		cout << " ";
	cout << "<<" << _txt << endl;
}
