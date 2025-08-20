#include "VehicleFleet.h"
#include "Vehicle.h"
#include "Render.h"
#include "Time.h"
#include "Model.h"
#include "Log.h"
#include "Trace.h"
#include "ParameterLoader.h"
#include "Segment.h"
#include "KPI.h"

#include <algorithm>
#include <chrono>
#include <map>


VehicleFleet::VehicleFleet(Model& m)
	: _mol(new deque<Vehicle*>),
	_vrtxArray(nullptr), _vrtxBuffer(0), _mdl(m), _globalNominalTravelTime(0.0)
{

	for (int i = 0; i < ParameterLoader::getInt("Heavy"); i++)
		_mol->push_back(new Vehicle(m, ParameterLoader::getDouble("HeavyWidth") / 1000, ParameterLoader::getDouble("HeavyLength") / 1000));
	for (int i = 0; i < ParameterLoader::getInt("Medium"); i++)
		_mol->push_back(new Vehicle(m, ParameterLoader::getDouble("MediumWidth") / 1000, ParameterLoader::getDouble("MediumLength") / 1000));
	for (int i = 0; i < ParameterLoader::getInt("Light"); i++)
		_mol->push_back(new Vehicle(m, ParameterLoader::getDouble("LightWidth") / 1000, ParameterLoader::getDouble("LightLength") / 1000));

	Log::it() << "Created " << _mol->size() << " vehicles total" << endl;

	const chrono::time_point<chrono::steady_clock> start = chrono::steady_clock::now();
	const chrono::time_point<chrono::steady_clock> end = chrono::steady_clock::now();
	const chrono::duration<double> elapsed_seconds = end - start;
	
}

VehicleFleet::~VehicleFleet()
{
	for (deque<Vehicle*>::const_iterator mi = _mol->begin(); mi != _mol->end(); ++mi)
		delete* mi;
	delete _mol;

}



void VehicleFleet::tick()
{
	double now = _mdl.getTime().t();

	checkFutureCollision();

	size_t activeVehicles = _mol->size();

	// Update all vehicles
	for (deque<Vehicle*>::iterator mi = _mol->begin(); mi != _mol->end(); mi++)
	{
		(*mi)->tick();

		if ((*mi)->hasExited() || !(*mi)->running())
			activeVehicles--;
	}
	calcCollision(*_mol);

	KPI::getKPI().updateTrafficDensity(int(activeVehicles));
	if (activeVehicles == 0)
	{
		_mdl.setSimulation(false);
	}
}

void VehicleFleet::init()
{
	Trace t("VehicleFleet::init");

	_vrtxArray = new GLfloat[_mol->size() * 6];

	// Calculate total optimal travel time for all vehicles
	_globalNominalTravelTime = getNominalTravelTime();

	glGenBuffers(1, &_vrtxBuffer);
}

void VehicleFleet::draw()
{
	Trace t("VehicleFleet::draw");

	if (!_vrtxArray || _mol->empty()) {
		return;
	}

	GLfloat* fp = _vrtxArray;
	for (int i = 0; i < _mol->size(); i++)
	{
		Vehicle& m = *(*_mol)[i];
		Vect3D p = m.position();

		// Better logic for spawning vehicles visualization
		bool isSpawning = (p.x() == 0.0 && p.y() == 0.0 && !m.hasExited() && !m.hasSegment());

		if (isSpawning) {
			// Spread spawning vehicles in a circle around origin
			double angle = (i * 2.0 * 3.14159) / _mol->size();
			double radius = 2.5; // Distance from roundabout
			*fp++ = (GLfloat)(radius * cos(angle));
			*fp++ = (GLfloat)(radius * sin(angle));
			*fp++ = (GLfloat)0.0;
		}
		else {
			*fp++ = (GLfloat)p.x();
			*fp++ = (GLfloat)p.y();
			*fp++ = (GLfloat)p.z();
		}

		// Color vehicles based on state with better distinction
		if (m.hasExited()) {
			// Blue for vehicles that have exited
			*fp++ = 0.f;  // Rouge
			*fp++ = 0.f;  // Vert  
			*fp++ = 1.f;  // Bleu
		}
		else if (m.getcoll()) {
			// Red for colliding vehicles (includes both entry waiting and segment waiting)
			*fp++ = 1.f;  // Rouge
			*fp++ = 0.f;  // Vert  
			*fp++ = 0.f;  // Bleu
		}
		else {
			// Green for moving vehicles
			*fp++ = 0.f;  // Rouge
			*fp++ = 1.f;  // Vert  
			*fp++ = 0.f;  // Bleu
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _mol->size() * 6, _vrtxArray, GL_DYNAMIC_DRAW);

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		6 * sizeof(GLfloat),  // stride
		(void*)0            // array buffer offset
	);

	//color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vrtxBuffer);
	glVertexAttribPointer(
		1,                              // attribute 1
		3,                              // size
		GL_FLOAT,                       // type
		GL_FALSE,                       // normalized?
		6 * sizeof(GLfloat),              // stride
		(void*)(3 * sizeof(GL_FLOAT))   // array buffer offset
	);

	glDrawArrays(GL_POINTS, 0, _mol->size());
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void VehicleFleet::checkFutureCollision()
{
	//_mdl.startSimulationTime();

	for (auto v : *_mol)
	{
		if (!v->getCurrentSegment()->priorityLevel())
		{
			double distToFinishSeg = (v->getCurrentSegment()->length() - v->getDistOnSeg());
			double safetyDist = v->length() + v->width();
			if (distToFinishSeg <= 1.5*safetyDist) //dist to finish is less than safety distance
			{
				if (!noIncomingVehicle(v))
				{
					v->stopVehi();
				}
				else if (v->getSpeed() == 0 && noVehiAhead(v))
					v->restartVehi();
			}
			else
			{
				if ( noVehiAheadOnSeg(v, ParameterLoader::getInt("TimeAhead")))
				{
					v->restartVehi();
				}
			}
		}
	}
}


double VehicleFleet::getNominalTravelTime() const
{
	double globalNominalTravelTime = 0.0;
	for (size_t i = 0; i < _mol->size(); i++)
	{
		globalNominalTravelTime += (*_mol)[i]->getTravelTimeOptimal();
	}
	return globalNominalTravelTime;
}

bool VehicleFleet::calcCollision(deque<Vehicle*>& vehi) const
{
	bool rv = false;
	if( !_mdl.isSimulationRunning() )
		for (auto v : vehi)
			v->setcol(false);
	
	for (auto v1 : vehi)
	{
		if (!v1->running())
			continue;
		for (auto v2 : vehi)
		{
			if (v1 == v2 || !v2->running())
				continue;
			bool col = v1->collides(v2);
			if (col) {
				v1->setcol(true);
				v2->setcol(true);
				Log::it() << "VEHICLE #" << v1->getId() << " with VEHICLE #" << v2->getId() << " on segments v1 " << v1->getCurrentSegment()->id() 
					<< " ; v2 " << v2->getCurrentSegment()->id() << endl;
				rv = true;
			}
		}
	}
	return rv;
}

deque<Vehicle*> VehicleFleet::getVehiWithCollision(const deque<Vehicle*>& all)
{
	deque<Vehicle*> rv;
	for (auto v : all)
	{
		if (v->getcoll())
			rv.push_back(v);
	}
	return rv;
}


deque<const Vehicle*> VehicleFleet::vehiOnSegment(const Segment* s) const
{
	deque<const Vehicle*> rv;
	for (auto v : *_mol)
	{
		if (v->running() && v->getCurrentSegment() == s)
			rv.push_back(v);
	}
	return rv;
}


Vehicle* VehicleFleet::getVehiFromId(int id) const
{
	for (auto v : *_mol)
	{
		if (v->getId() == id)
			return v;
	}
	return nullptr;
}

bool VehicleFleet::isLeading(const Vehicle* v, const deque<Vehicle*>& all)
{
	double dmax = v->getDistOnSeg();
	for (auto a : all)
	{
		if (a->getCurrentSegment() == v->getCurrentSegment() && a->getDistOnSeg() > dmax)
		{
			return false;
		}
	}
	return true;
}

bool VehicleFleet::emptyCircle(const Vehicle* v)
{
	int nvehicle = 0;
	const Segment* cur = v->getCurrentSegment();
	const Segment* first = nullptr;
	bool loopcomplete = false;
	do
	{
		int nnext = cur->nNext();
		for (int i = 0; i < nnext; i++)
		{
			const Segment* nex = (*cur)[i];
			if (nex->priorityLevel())
			{
				if( first && first == nex) loopcomplete = true;
				if( !first ) first = nex;
				nvehicle += vehicleOnSegment(nex);
				cur = nex;
				break;
			}
		}
	} while (!loopcomplete);

	return nvehicle==0;
}

int VehicleFleet::vehicleOnSegment(const Segment* s)
{
	int rv = 0;
	for (auto v : *_mol)
		if (v->getCurrentSegment() == s)
			rv++;
	return rv;
}

bool VehicleFleet::noIncomingVehicle(const Vehicle* v) const
{
	if (!v->getNextSegmentInPath())
		return true;
	double distToEndSeg = v->getCurrentSegment()->length() - v->getDistOnSeg();
	double timeToEndSeg = distToEndSeg / v->getCurrentSegment()->vmax();
	double distOnNextSeg = v->length();
	double timeOnNextSeg = distOnNextSeg / v->getNextSegmentInPath()->vmax();
	double safetyTime = timeToEndSeg + timeOnNextSeg;

	for (auto i : *_mol)
	{
		if (i != v)
		{
			double timeToColl = i->timeToReachSeg(v->getNextSegmentInPath());
			if (timeToColl > 0 && timeToColl < safetyTime)
				return false;
		}
	}
	return true;
}

bool VehicleFleet::noVehiAhead(const Vehicle*v) const
{
	const Segment* nextseg = v->getNextSegmentInPath();
	if (!nextseg)
		return true;
	
	double distToEndSeg = v->getCurrentSegment()->length() - v->getDistOnSeg();
	double timeToEndSeg = distToEndSeg / v->getCurrentSegment()->vmax();

	for (auto i : *_mol)
	{
		if (i != v)
		{
			double timeToColl = i->timeToReachSeg(nextseg);
			if (timeToColl < 0) continue;

			if (timeToColl < timeToEndSeg)
			{
				// i ahead of v
				double deltat = timeToEndSeg - timeToColl;
				double dist = deltat * nextseg->vmax();
				if (dist < i->length())
				{
					return false;
				}
					
			}
			else
			{
				// v ahead of i
				double deltat = timeToColl - timeToEndSeg;
				double dist = deltat * nextseg->vmax();
				if (dist < v->length())
					return false;
			}
		}
	}
	std::cout << "Vehicle #" << v->getId() << " restarts" << endl;
	return true;

}

bool VehicleFleet::noVehiAheadOnSeg(const Vehicle* v, double deltat) const
{
	const Segment* curSeg = v->getCurrentSegment();
	double vpos = v->getDistOnSeg();
	for (auto i : *_mol)
	{
		if ( v != i && i->getCurrentSegment() == curSeg)
		{
			double ipos = i->getDistOnSeg();
			if (ipos > vpos)
			{
				double deltad = ipos - vpos;
				double time = deltad / curSeg->vmax();
				if (time < deltat)
					return false;
			}
		}
	}
	return true;
}



Vehicle* VehicleFleet::getFollowingVehicle(const Vehicle* ref) const
{
	const Segment* seg = ref->getCurrentSegment();
	Vehicle* rv = nullptr;
	double progress = 0;
	for (auto vehi : *_mol)
	{
		if (vehi == ref || vehi->getCurrentSegment() != seg || vehi->getDistOnSeg() >= ref->getDistOnSeg() || ref->hasExited())
			continue;
		if (progress < vehi->getDistOnSeg())
		{
			progress = vehi->getDistOnSeg();
			rv = vehi;
		}
	}
	if (rv)
		Log::it() << ref->getId() << " is followed by " << rv->getId() << " at distance " << ref->getDistOnSeg() - rv->getDistOnSeg() << endl;
	return rv;
}

ostream& operator<<(ostream& o, const VehicleFleet& g)
{
	for (deque<Vehicle*>::iterator mi = g._mol->begin(); mi != g._mol->end(); ++mi)
	{
		o << **mi << endl;
	}
	return o;
}