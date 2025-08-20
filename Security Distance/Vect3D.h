#pragma once

#include <array>
#include <iostream>
using namespace std;

class Vect3D : private array<double, 3>
{
public:
	Vect3D();
	Vect3D(double, double, double);
	Vect3D(double[3]);
	Vect3D(const Vect3D&);
	const Vect3D& operator=(const Vect3D&);

	~Vect3D();

	inline double x() const { return operator[](0); }
	inline double y() const { return operator[](1); }
	inline double z() const { return operator[](2); }

	inline double &x() { return operator[](0); }
	inline double &y() { return operator[](1); }
	inline double &z() { return operator[](2); }

	Vect3D operator+(const Vect3D&) const;
	Vect3D operator-(const Vect3D&) const;
	Vect3D operator-() const;
	Vect3D operator*(double) const;
	Vect3D operator/(double) const;

	double norme() const;
	double norme2() const;

	friend ostream& operator<<(ostream&, const Vect3D&);

private:
};

