#pragma once

#include <deque>
#include <list>
#include <iostream>
using namespace std;

// Include GLEW
#include <GL/glew.h>

// Forward declarations
class Vehicle;
class Model;
class Segment;

class VehicleFleet
{
private:
	deque<Vehicle*>* _mol;

	GLfloat* _vrtxArray;
	GLuint _vrtxBuffer;

	Model& _mdl;
	double _globalNominalTravelTime;

	Vehicle* getVehiFromId(int id) const;

	bool isLeading(const Vehicle* v, const deque< Vehicle*>&);
	bool emptyCircle(const Vehicle* v);
	int vehicleOnSegment(const Segment*);
	bool noIncomingVehicle(const Vehicle*) const;
	bool noVehiAhead(const Vehicle*) const;
	bool noVehiAheadOnSeg(const Vehicle*, double) const;

public:
	VehicleFleet(Model& m);
	~VehicleFleet();

	void tick();
	void init();
	void draw();

	void checkFutureCollision();
	bool calcCollision(deque<Vehicle*>&) const;
	deque<Vehicle*> getVehiWithCollision(const deque<Vehicle*>&);
	Vehicle* getFollowingVehicle(const Vehicle* v) const;

	deque<const Vehicle*> vehiOnSegment(const Segment*) const;

	double getNominalTravelTime() const;

	friend ostream& operator<<(ostream& o, const VehicleFleet& g);
};