#pragma once

#include <deque>
#include <GL/glew.h>

class Point;
class Line;
class Circle;
class Segment;
class SegmentDroit;
class SegmentCourbe;


class RoadTopo
{
public:
	RoadTopo();
	~RoadTopo();

	void init();
	void draw();

	std::deque<SegmentCourbe*> getSegCourbes() const { return _segCourbes; }
	std::deque<SegmentDroit*> getSegDroits() const { return _segDroits; }
	const std::deque<const Segment*> & getEntries() const { return _entries; }
	const std::deque<const Segment*> & getExits() const { return _exits; }

private:
	std::deque<const Point*> _points;
	std::deque<const Line*> _lines;
	std::deque<const Circle*> _circles;
	std::deque<SegmentDroit*> _segDroits;
	std::deque<SegmentCourbe*> _segCourbes;
	std::deque<const Segment*> _entries, _exits;

	void makeSegments();
	void makeSegDroits();
	void makeSegCourbes();
	void linkSegments(); 
	void findEntries();
	void findExits();
	const Circle* circleUsingPoint(const Point*);


	GLfloat* _vrtxArrayPt, * _vrtxArrayLn, * _vrtxArrayCi;
	GLuint _vrtxBufferPt, _vrtxBufferLn, _vrtxBufferCi;

	static const double _laneWidth;
};