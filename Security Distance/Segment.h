#pragma once

#include "Topo.h"
#include "Vect3D.h"

#include <vector>
#include <iostream>
using namespace std;

// Forward declaration
class Vehicle;

class Segment
{
private : 
	int _id;

protected:
	const Point* _a;
	const Point* _b;
	vector<const Segment*> _next;

	double _maxSpeed;

public:
	Segment(const Point* a, const Point* b);
	virtual ~Segment();

	const Point* A() const { return _a; }
	const Point* B() const { return _b; }
	int id() const { return _id; }

	virtual double length() const = 0;
	virtual double vmax() const;
	virtual bool priorityLevel() const;

	void addNext(const Segment* s);
	int nNext() const;
	const Segment* operator[](int i) const;

	virtual void getPosVit(double lengthSpentOnSeg, Vect3D& pos, Vect3D& vit) const;
};

class SegmentDroit : public Segment
{
public:
	SegmentDroit(const Point* a, const Point* b);
	virtual ~SegmentDroit();

	virtual double length() const;
};

class SegmentCourbe : public Segment
{
	const Point* _c;

public:
	SegmentCourbe(const Point* c, const Point* a, const Point* b, bool s);
	virtual ~SegmentCourbe();

	void getPosVit(double lengthSpentOnSeg, Vect3D& pos, Vect3D& vit) const;

	const Point* C() const { return _c; }

	double angle() const;
	double r() const;
	virtual double length() const;
	bool priorityLevel() const;

};