#include "Segment.h"
#include "Topo.h"
#include "Log.h"

#include <iostream>
using namespace std;
#include <cmath>
#include <utility>

static double pi = 4 * atan(1);

static int nextId = 0;
//
//
//

Segment::Segment(const Point* a, const Point* b)
	: _a(a), _b(b), _maxSpeed(80./3600), _id(nextId++)
{
}

Segment::~Segment()
{
	// _a _b lifecycle managed elsewhere
}

double Segment::vmax() const
{
	return _maxSpeed;
}

bool Segment::priorityLevel() const
{
	return false;
}

void Segment::addNext(const Segment* s)
{
	_next.push_back(s);
	cout << "linked " << this << " to " << s << endl;
}

int Segment::nNext() const
{
	return _next.size();
}

const Segment* Segment::operator[](int i) const
{
	return _next[i];
}


void Segment::getPosVit(double lengthSpentOnSeg, Vect3D& pos, Vect3D& vit) const
{
	double alpha = lengthSpentOnSeg / length();

	pos.x() = _a->x() + alpha * (_b->x() - _a->x()); //straight line to follow here because virtual
	pos.y() = _a->y() + alpha * (_b->y() - _a->y());
	pos.z() = 0;

	vit.x() = _b->x() - _a->x();
	vit.y() = _b->y() - _a->y();
	vit.z() = 0;

	vit = vit / vit.norme();

}
//
// getPosVit
// follow the curve of the segment if its a curved segment
//


void SegmentCourbe::getPosVit(double lengthSpentOnSeg, Vect3D& pos, Vect3D& vit) const
{
	double teta = lengthSpentOnSeg / r();
	double alpha = atan2(_a->y() - _c->y(), _a->x() - _c->x());

	pos.x() = _c->x() + r() * cos(teta + alpha);
	pos.y() = _c->y() + r() * sin(teta + alpha);
	pos.z() = 0;
	
	Vect3D r = pos - Vect3D(_c->x(), _c->y(),0);
	r = r / r.norme();

	vit.x() = -r.y();
	vit.y() = r.x();
	vit.z() = 0;
	
	//Segment::getPosVit(lengthSpentOnSeg, pos, vit);
}


//
//
//

SegmentDroit::SegmentDroit(const Point* a, const Point* b)
	: Segment(a, b)
{
	cout << "segment straight #" << id() << " " << _a->x() << " " << _a->y() << " " << _b->x() << " " << _b->y() << endl;
}

SegmentDroit::~SegmentDroit()
{
	// _a _b lifecycle managed elsewhere
}

double SegmentDroit::length() const
{
	double dx = _b->x() - _a->x();
	double dy = _b->y() - _a->y();

	return sqrt(dx * dx + dy * dy);
}


//
//
//

SegmentCourbe::SegmentCourbe(const Point* c, const Point* a, const Point* b, bool s)
	: Segment(a, b), _c(c)
{
	if (s && angle() < 0.)
		swap(_a, _b);
	cout << "segment curved #" << id() << " " << _a->x() << " " << _a->y() << " " << _b->x() << " " << _b->y() << " " << angle() << endl;
	_maxSpeed = 30. / 3600;
}

SegmentCourbe::~SegmentCourbe()
{
	// _a _b lifecycle managed elsewhere
}

double SegmentCourbe::angle() const
{
	double alpha = atan2(_a->y() - _c->y(), _a->x() - _c->x());
	double beta = atan2(_b->y() - _c->y(), _b->x() - _c->x());
	double delta = beta - alpha;
	if (delta < 0.) delta += 2 * pi;
	return delta;
}

double SegmentCourbe::r() const
{
	double dx = _b->x() - _c->x();
	double dy = _b->y() - _c->y();
	return sqrt(dx * dx + dy * dy);
}

double SegmentCourbe::length() const
{
	double l = angle() * r();
	return l;
}

bool SegmentCourbe::priorityLevel() const
{
	return true;
}