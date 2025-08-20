#include "Model.h"
#include "Time.h"
#include "VehicleFleet.h"
#include "Log.h"
#include"ParameterLoader.h"


static int timeAhead = ParameterLoader::getInt("TimeAhead");

//
// ctor
// creates the road network
//

Model::Model()
	: _tps(new Time), _vf(nullptr), _ic("Model"), _rt(new RoadTopo), _running(true), _simulatedTime(nullptr)
{
	_vf = new VehicleFleet(*this);
}

//
// dtor
// frees everything
//

Model::~Model()
{
	delete _vf;
	delete _tps;
	delete _rt;
}

//
// tick for time
//

void Model::tick()
{
	_tps->tick();
	_vf->tick();
}

//
// init
//

void Model::init()
{
	_vf->init();
	_rt->init();
}

//
// draw
// dessine la boite et le gaz
//

void Model::draw()
{
	_vf->draw();
	_rt->draw();
}

//
// getters
//

Time& Model::getTime()
{
	if (!isSimulationRunning())
		return *_tps;
	return *_simulatedTime;
}


RoadTopo& Model::getRoadTopo()
{
	return *_rt;
}

VehicleFleet& Model::getVehiculeFleet()
{
	return *_vf;
}

void Model::startSimulationTime()
{
	if (!_simulatedTime)
	{
		_simulatedTime = new Time(*_tps);
		Log::it() << "start simu at " << _simulatedTime->t() << endl;
	}
}

void Model::endSimulationTime()
{
	if (_simulatedTime)
	{
		Log::it() << "end simu at " << _simulatedTime->t() << endl;
		delete _simulatedTime;
		_simulatedTime = nullptr;

	}
		
}
