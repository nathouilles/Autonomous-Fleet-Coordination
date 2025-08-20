#pragma once

//
//
//

class Point
{
public:
	static Point* create(double, double);

	double x() const { return _x; }
	double y() const { return _y; }
	double z() const { return 0.; }

private:
	Point(double, double);

	double _x, _y;
};

//
//
//

class Line
{
public:
	static Line* create(const Point*, const Point*);

	const Point* A() const { return _a; }
	const Point* B() const { return _b; }

private:
	Line(const Point* a, const Point* b);

	const Point* _a, * _b;
};

//
//
//

class Circle
{
public:
	static Circle* create(const Point*, double);

	const Point* C() const { return _c; }
	double r() const { return _r; }

	bool isOnCircle(const Point&) const;

private:
	Circle(const Point*, double);

	const Point* _c;
	double _r;
};
