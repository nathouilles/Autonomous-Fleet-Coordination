#include "Time.h"
#include "ParameterLoader.h"
#include <iostream>

//
// ctor
// initialisation avec l'instant de départ et le pas de temps
//

Time::Time()
	: _t(0.), _dt(1./30.)
{
	int fps = ParameterLoader::getInt("FPS");
	if (fps > 0)
		_dt = 1. / fps;

}

//
// dtor
// rien à faire
//

Time::~Time()
{

}

//
// get
// accesseur pour l'instant actuel
//

double Time::t() const
{
	return _t;
}

//
// step
// accesseur pour le pas de temps
//

double Time::dt() const
{
	return _dt;
}

//
// tick
// propage le temps futur au modele
// change le temps actuel du pas de temps
//

void Time::tick()
{
	_t += _dt;
	//	Log::
	// () << *this << endl;
}

//
// operator <<
// gestion standard des flux
//

ostream& operator<<(ostream& o, const Time& t)
{
	o << t._t << " " << t._dt;
	return o;
}
