#pragma once

class Time;
class VehicleFleet;

#include "InstanceCount.h"
#include "RoadTopo.h"
#include "KPI.h"

//
// Model
//

class Model
{
public:
	Model();
	~Model();

	void tick();

	void init();
	void draw();
	
	void setSimulation(bool x) { _running = x; }
	bool getSimulation() { return _running; }
	
	Time& getTime();
	RoadTopo& getRoadTopo();
	VehicleFleet& getVehiculeFleet();

	void startSimulationTime();
	void endSimulationTime();

	bool isSimulationRunning() const { return _simulatedTime; }

private:
	Time* _tps, *_simulatedTime;
	RoadTopo* _rt;
	VehicleFleet* _vf;
	InstanceCount _ic;
	bool _running;
};

