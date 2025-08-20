#include "Vect3D.h"


//
// ctor
// par défaut, mise à 0.
//

Vect3D::Vect3D()
{
	(*this)[0] = (*this)[1] = (*this)[2] = 0.;
}

//
// ctor
// transformation de 3 double en un Vect3D
//

Vect3D::Vect3D(double x, double y, double z)
{
	operator[](0) = x;
	operator[](1) = y;
	operator[](2) = z;
}

//
// ctor
// trasnformation d'un tableau en un Vect3D
//

Vect3D::Vect3D(double x[3])
{
	operator[](0) = x[0];
	operator[](1) = x[1];
	operator[](2) = x[2];
}

//
// copy ctor
//

Vect3D::Vect3D(const Vect3D& v)
	: array<double, 3>(v)
{
	// nothing else
}

//
// dtor
// rien à faire
//

Vect3D::~Vect3D()
{

}

//
// longueur
// calcule la norme
//

double Vect3D::norme2() const
{
	return x() * x() + y() * y() + z() * z();
}

double Vect3D::norme() const
{
	return sqrt(norme2());
}

//
// operator=
// copie le membre de droite dans le membre de gauche
//

const Vect3D& Vect3D::operator=(const Vect3D& b)
{
	array<double, 3>::operator=(b);
	return *this;
}

//
// operator+
// additionne 2 Vect3D
//

Vect3D Vect3D::operator+(const Vect3D& b) const
{
	const Vect3D& a = (*this);
	Vect3D rv(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
	return rv;
}

//
// operator-
// soustrait 2 Vect3D
//

Vect3D Vect3D::operator-(const Vect3D& b) const
{
	const Vect3D& a = (*this);
	Vect3D rv(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
	return rv;
}

//
// operator- (unaire)
// renvoit l'oppose
//

Vect3D Vect3D::operator-() const
{
	return Vect3D(-x(), -y(), -z());
}

//
// operator*
// multiplie chaque coordonnée par la valeur donnée
//

Vect3D Vect3D::operator*(double d) const
{
	const Vect3D& a = (*this);
	Vect3D rv(x() * d, y() * d, z() * d);

	return rv;
}

//
// operator/
// divise chaque coordonnée par la valeur donnée
//

Vect3D Vect3D::operator/(double d) const
{
	const Vect3D& a = (*this);
	Vect3D rv(a[0] / d, a[1] / d, a[2] / d);

	return rv;
}

//
// operator<<
// ostream compatible
//

ostream& operator<<(ostream& o, const Vect3D& v)
{
	o << "{";
	for (int i = 0; i < 3; i++)
		o << v[i] << (i==2?"}":",");
	return o;
}