#include "RoadTopo.h"
#include "Topo.h"
#include "Segment.h"

#include <iostream>
using namespace std;

#include <cmath>

const double RoadTopo::_laneWidth = .01;


RoadTopo::RoadTopo() : _vrtxArrayPt(nullptr), _vrtxArrayLn(nullptr), _vrtxArrayCi(nullptr), _vrtxBufferPt(0), _vrtxBufferLn(0), _vrtxBufferCi(0)
{
	_points.push_back(Point::create(.0, .0)); // Origin					
	_points.push_back(Point::create(-.5, -.5)); // South West			//500m length
	_points.push_back(Point::create(.5, .5)); // North East
	_points.push_back(Point::create(.5, -.5)); // South East
	_points.push_back(Point::create(-.5, .5)); // North West

	_circles.push_back(Circle::create(_points[0], .025)); // radius 25m

	_lines.push_back(Line::create(_points[0], _points[1])); // route W
	_lines.push_back(Line::create(_points[0], _points[2])); // route E
	_lines.push_back(Line::create(_points[0], _points[3])); // route S
	_lines.push_back(Line::create(_points[0], _points[4])); // route N

	makeSegments();
}

RoadTopo::~RoadTopo()
{
	delete[] _vrtxArrayPt;
	_vrtxArrayPt = nullptr;
	delete[] _vrtxArrayLn;
	_vrtxArrayLn = nullptr;
	delete[] _vrtxArrayCi;
	_vrtxArrayCi = nullptr;

	for (auto ptr : _circles)
		delete ptr;
	for (auto ptr : _lines)
		delete ptr;
	for (auto ptr : _points)
		delete ptr;
	for (auto ptr : _segDroits)
		delete ptr;
	for (auto ptr : _segCourbes)
		delete ptr;
}

//
//
//

void RoadTopo::makeSegDroits()
{
	// pour toutes les lignes, on va chercher les intersections avec un cercle
	for (auto l : _lines)
	{
		const Point* p1 = l->A(), * p2 = l->B();
		const Circle* ci = circleUsingPoint(p1);
		if (!ci)
		{
			p1 = l->B(); p2 = l->A();
			ci = circleUsingPoint(p1);
			if (!ci) continue;
		}

		// on cherche l'intersection entre le cercle et la droite parametrique, equation du 2nd degre
		double x1 = p1->x();
		double x2 = p2->x();
		double y1 = p1->y();
		double y2 = p2->y();
		double dx = x2 - x1;
		double dy = y2 - y1;
		double xc = ci->C()->x();
		double yc = ci->C()->y();
		double a = dx * dx + dy * dy;

		// normale a la direction, normalisee a la largeur de voie
		double nx = -dy / a * _laneWidth / 2;
		double ny = dx / a * _laneWidth / 2;

		// on offset l'axe median dans les 2 sens
		x1 += nx; y1 += ny; x2 += nx; y2 += ny;
		int n = 0;
		for (int k = 0; k < 2; k++)
		{
			if (k)
			{
				x1 -= 2 * nx; y1 -= 2 * ny; x2 -= 2 * nx; y2 -= 2 * ny;
			}

			double b = 2 * (x1 - xc) * dx + 2 * (y1 - yc) * dy;
			double c = (x1 - xc) * (x1 - xc) + (y1 - yc) * (y1 - yc) - ci->r() * ci->r();
			double delta = b * b - 4. * a * c;
			if (delta > 0)
			{
				double t1 = (-b + sqrt(delta) / 2 / a);
				double t2 = (-b - sqrt(delta) / 2 / a);
				double t = t1 < t2 ? t2 : t1;
				Point* pi = Point::create(x1 + t * dx, y1 + t * dy), * pe = Point::create(x2, y2);
				SegmentDroit* sd = new SegmentDroit(k ? pi : pe, k ? pe : pi);
				_segDroits.push_back(sd);
				n++;
			}
		}
		if (n == 2)
		{
			const Point* c = ci->C();
			const Point* a = _segDroits[_segDroits.size() - 1]->A(); // tant que c'est dans cet ordre dans le push back
			const Point* b = _segDroits[_segDroits.size() - 2]->B(); // tant que c'est dans cet ordre dans le push back
			SegmentCourbe* sc = new SegmentCourbe(c, a, b, true);
			_segCourbes.push_back(sc);

		}
		else
			cout << "big problem: not 2 lanes with circle" << endl;
	}
}

//
//
//

void RoadTopo::makeSegCourbes()
{
	// pour tous les cercles, on cherche les points intersectes par les segments droits
	const size_t m = _segCourbes.size();
	for (int i = 0; i < m; i++)
	{
		const Point* c = _segCourbes[i]->C();
		const Point* b = _segCourbes[i]->B();
		cout << "i=" << i << " " << b->x() << " " << b->y() << endl;

		// pour tous les segcourbes j'ai le segment a vers b, il faut trouver les segments b - a (d'un autre segment droit)
		int jmin = i + 1;
		double amin = 2 * 4 * atan(1); // 2 pi
		for (int j = 0; j < m; j++)
		{
			if (i == j) continue;
			const Point* a = _segCourbes[j]->A();
			SegmentCourbe sg(c, b, a, false);
			double angle = sg.angle();
			if (angle < amin)
				jmin = j, amin = angle;
		}
		cout << "courbe de " << i << " vers " << jmin << endl;
		SegmentCourbe* sc = new SegmentCourbe(c, b, _segCourbes[jmin]->A(), false);
		_segCourbes.push_back(sc);
	}
}

//
//
//

void RoadTopo::linkSegments()
{
	for (auto out : _segDroits)
	{
		for (auto in : _segCourbes)
		{
			if (out->B() == in->A())
				out->addNext(in);
		}
		for (auto in : _segDroits)
		{
			if (out->B() == in->A())
				out->addNext(in);
		}
	}
	for (auto out : _segCourbes)
	{
		for (auto in : _segCourbes)
		{
			if (out->B() == in->A())
				out->addNext(in);
		}
		for (auto in : _segDroits)
		{
			if (out->B() == in->A())
				out->addNext(in);
		}
	}
}

//
//
//

void RoadTopo::findEntries()
{
	for (auto s : _segDroits)
	{
		// une entree est un segment qui n'est le next d'aucun autre - point A
		bool found = false;
		for (auto s2 : _segCourbes)
		{
			for (int i = 0; i < s2->nNext(); i++)
			{
				if ((*s2)[i] == s)
				{
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		if (!found)
			_entries.push_back(s);
	}
	for (auto s : _entries)
		cout << " entries " << s->A()->x() << " " << s->A()->y() << endl;
}

//
//
//

void RoadTopo::findExits()
{
	for (auto s : _segDroits)
	{
		// une sortie est un segment sans next - point B
		if (!s->nNext())
			_exits.push_back(s);
	}
	for (auto s : _exits)
		cout << " exits " << s->B()->x() << " " << s->B()->y() << endl;

}

//
//
//

void RoadTopo::makeSegments()
{
	makeSegDroits();
	makeSegCourbes();
	linkSegments();
	findEntries();
	findExits();

	cout << _entries.size() << " entries" << endl;
	cout << _exits.size() << " exits" << endl;
}

//
//
//

const Circle* RoadTopo::circleUsingPoint(const Point* p)
{
	for (auto const c : _circles)
	{
		if (c->C() == p)
			return c;
	}
	return nullptr;
}


void RoadTopo::init()
{
	// points

	_vrtxArrayPt = new GLfloat[6 * _points.size()];
	GLfloat* fp = _vrtxArrayPt;
	for (int i = 0; i < _points.size(); i++)
	{
		const Point& p = *_points[i];
		*fp++ = (GLfloat)p.x();
		*fp++ = (GLfloat)p.y();
		*fp++ = (GLfloat)p.z();

		*fp++ = 1.f;
		*fp++ = 1.f;
		*fp++ = 1.f;
	}
	glGenBuffers(1, &_vrtxBufferPt);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferPt);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _points.size() * 6, _vrtxArrayPt, GL_STATIC_DRAW);
	glPointSize(8.);

	// lines

	_vrtxArrayLn = new GLfloat[6 * 2 * _segDroits.size()];
	fp = _vrtxArrayLn;
	for (int i = 0; i < _segDroits.size(); i++)
	{
		const Point& a = *_segDroits[i]->A();
		const Point& b = *_segDroits[i]->B();
		*fp++ = (GLfloat)a.x();
		*fp++ = (GLfloat)a.y();
		*fp++ = (GLfloat)a.z();

		*fp++ = 1.f;
		*fp++ = 1.f;
		*fp++ = 1.f;

		*fp++ = (GLfloat)b.x();
		*fp++ = (GLfloat)b.y();
		*fp++ = (GLfloat)b.z();

		*fp++ = 1.f;
		*fp++ = 1.f;
		*fp++ = 1.f;
	}
	glGenBuffers(1, &_vrtxBufferLn);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferLn);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _segDroits.size() * 6 * 2, _vrtxArrayLn, GL_STATIC_DRAW);

	// circles
	const int segment = 18; // 18 segments pour une circonference
	const double pi = 3.1415926535;
	_vrtxArrayCi = new GLfloat[segment * 6 * _circles.size()];
	fp = _vrtxArrayCi;
	for (int i = 0; i < _circles.size(); i++)
	{
		const Point& c = *_circles[i]->C();
		const double r = _circles[i]->r();
		for (int j = 0; j < segment; j++)
		{
			*fp++ = c.x() + r * cos(pi * 2 * j / segment);
			*fp++ = c.y() + r * sin(pi * 2 * j / segment);
			*fp++ = c.z();

			*fp++ = 1.f;
			*fp++ = 1.f;
			*fp++ = 1.f;
		}
	}
	glGenBuffers(1, &_vrtxBufferCi);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferCi);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _circles.size() * 6 * segment, _vrtxArrayCi, GL_STATIC_DRAW);
}

void RoadTopo::draw()
{
	// points

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferPt);
	glVertexAttribPointer(
		0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		6 * sizeof(GLfloat),	// stride
		(void*)0			// array buffer offset
	);

	//color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferPt);
	glVertexAttribPointer(
		1,								// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,								// size
		GL_FLOAT,						// type
		GL_FALSE,						// normalized?
		6 * sizeof(GLfloat),				// stride
		(void*)(3 * sizeof(GL_FLOAT))	// array buffer offset
	);

	glDrawArrays(GL_POINTS, 0, _points.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// lines

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferLn);
	glVertexAttribPointer(
		0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		6 * sizeof(GLfloat),	// stride
		(void*)0			// array buffer offset
	);

	//color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferLn);
	glVertexAttribPointer(
		1,								// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,								// size
		GL_FLOAT,						// type
		GL_FALSE,						// normalized?
		6 * sizeof(GLfloat),				// stride
		(void*)(3 * sizeof(GL_FLOAT))	// array buffer offset
	);

	glDrawArrays(GL_LINES, 0, 2 * _segDroits.size()); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// circles

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferCi);
	glVertexAttribPointer(
		0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		6 * sizeof(GLfloat),	// stride
		(void*)0			// array buffer offset
	);

	//color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBufferCi);
	glVertexAttribPointer(
		1,								// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,								// size
		GL_FLOAT,						// type
		GL_FALSE,						// normalized?
		6 * sizeof(GLfloat),				// stride
		(void*)(3 * sizeof(GL_FLOAT))	// array buffer offset
	);

	for (int k = 0; k < _circles.size(); k++)
	{
		glDrawArrays(GL_LINE_LOOP, k * 18, 18); // Starting from vertex 0; 3 vertices total -> 1 triangle
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}

