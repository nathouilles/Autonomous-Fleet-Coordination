#include "Topo.h"

#include <cmath>

//
//
//

Point::Point(double x, double y)
    : _x(x), _y(y)
{
}

Point* Point::create(double x, double y)
{
    return new Point(x, y);
}

//
//
//

Line::Line(const Point* a, const Point* b)
    : _a(a), _b(b)
{

}

Line* Line::create(const Point* a, const Point* b)
{
    return new Line(a, b);
}

//
//
//

Circle::Circle(const Point* c, double r)
    : _c(c), _r(r)
{

}

Circle* Circle::create(const Point* c, double r)
{
    return new Circle(c, r);
}

bool Circle::isOnCircle(const Point& p) const
{
    double x = p.x() - _c->x();
    double y = p.y() - _c->y();
    double r = _r;

    return abs(x * x + y * y - r * r) < .001;
}
