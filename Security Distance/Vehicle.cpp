#include "Vehicle.h"
#include "Time.h"
#include "Random.h"
#include "Model.h"
#include "Render.h"
#include "Log.h"
#include "Trace.h"
#include "RoadTopo.h"
#include "Segment.h"
#include "Topo.h"
#include "PathPlanner.h"
#include "KPI.h"
#include "VehicleFleet.h"
#include"ParameterLoader.h"

#include <cmath>
#include <iostream>

int Vehicle::nextId()
{
    static int nId = 0;
    return nId++;
}

//
// ctor
// model, width in meters, length in meters
//
Vehicle::Vehicle(Model& m, double w, double l)
    : _t(m.getTime().t()), _id(nextId()), _ic("Vehicle"), _mdl(m),
    _ended(false), _started(false), _entryTime(0.0), _exitTime(0.0), _speed(0),
    _plannedPath(), _segmentStartTime(0.0), _pathPlanner(new PathPlanner()),
    _currentPathIndex(0), _targetExit(nullptr),
    _coll(false), _width(w), _length(l), _distCurSeg(0.), _distTotal(0.)
{

    _pos = Vect3D(0, 0, 0);
    _vit = Vect3D(1, 0, 0);
    _entryTime = _t;

    // Select target exit first
    RoadTopo& rt = _mdl.getRoadTopo();
    _targetExit = selectRandomExit(rt);

    // Now plan path using existing method
    if (_targetExit && !planPathToExit(rt)) {
        //Log::it() << "Vehicle #" << _id << " failed to plan initial path" << endl;
    }
}

Vehicle::~Vehicle()
{
    // Vehicle during simulation is a copy of the real vehicle, so _pathPlanner is a pointer copy, not a value cpy
    // So we don't delete it when we fastforward the simulation to detect collisions
    if (!_mdl.isSimulationRunning())
        delete _pathPlanner;
}

double Vehicle::getTravelTimeOptimal() const
{
    // Calculate optimal travel time
    double rv = 0.0;
    for (const Segment* segment : _plannedPath) {
        if (segment) {
            double segmentLength = segment->length();
            rv += segmentLength / segment->vmax();
        }
    }
    return rv;
}

static bool checkConvex(Vect3D& p, Vect3D body[4])
{
    bool refsign = false;

    for (int j = 0; j < 4; j++)
    {
        Vect3D b = body[(j+1)%4] - body[j];
        Vect3D a = p - body[j];
        double z = b.y() * a.x() - b.x() * a.y();
        bool sign = z > 0;
        if (j == 0) {
            refsign = sign;
        }
        else if (z == 0.)
            break;
        else if (sign!=refsign)
        {
            return false;
        }
    }
    return true;
}

bool Vehicle::collides(const Vehicle* v2) const
{
    double r1, r2;
    r1 = sqrt( pow((length() / 2), 2) + pow(width() / 2, 2) );
    r2 = sqrt( pow(v2->length() / 2, 2) + pow(v2->width() / 2, 2) );

    Vect3D d(_pos - v2->_pos);

    if (d.norme() < (r1 + r2))
    {
        Vect3D car1Body[4], car2Body[4];
        footprint(car1Body);
        v2->footprint(car2Body);

        for (size_t i = 0; i < 4; i++)
        {
            bool inside = checkConvex(car1Body[i], car2Body);
            if (inside) 
                return true;
        }
    }
    return false;
}

void Vehicle::footprint(Vect3D coordinates[4]) const
{
    Vect3D v = _vit / _vit.norme();
    Vect3D toFront(v * length() / 2);
    Vect3D toLeft(-v.y() * width() / 2, v.x() * width() / 2, 0);

    coordinates[0] = _pos + toLeft + toFront; //front left
    coordinates[1] = _pos - toLeft + toFront; //front right
    coordinates[2] = _pos - toLeft - toFront; //rear right
    coordinates[3] = _pos + toLeft - toFront; //rear left

}

double Vehicle::timeToReachSeg(const Segment* final) const
{
    double timeToGo = 0.;
    for (int iter = getPathIndex(); iter < _plannedPath.size(); iter++)
    {
        if (_plannedPath[iter] == final)
            return timeToGo;

        double distToEnd = _plannedPath[iter]->length();
        double timeToEnd = distToEnd / _plannedPath[iter]->vmax();
        if (iter == getPathIndex())
            timeToEnd -= getDistOnSeg() / getCurrentSegment()->vmax();

        timeToGo += timeToEnd;
    }
    return -1.;
}

void Vehicle::stopVehi()
{
    _speed = 0;

    Log::it() << "I am #" << _id << " and I stop on segment #" << getCurrentSegment()->id() << " at " << getCurrentSegment()->length()-getDistOnSeg() << endl;

    Vehicle* v = _mdl.getVehiculeFleet().getFollowingVehicle(this);

    if (v != nullptr && v->running())
    {
        double dist = _distCurSeg;// getCurrentSegment()->length()* _segmentProgress;
        double distV = v->_distCurSeg;// v->getCurrentSegment()->length()* v->_segmentProgress;
        double diffDist = dist - distV;
        double vitOnSeg = getCurrentSegment()->vmax();
        double elapse = diffDist / vitOnSeg;

        if (elapse < ParameterLoader::getInt("TimeAhead"))
        {
            v->stopVehi();
        }
    }
}

void Vehicle::restartVehi()
{
    _speed = getCurrentSegment()->vmax();
}

void Vehicle::reinitForSimulation()
{
    _coll = false;
    restartVehi();
}


const Segment* Vehicle::getNextSegmentInPath() const
{
    if (_currentPathIndex + 1 < _plannedPath.size()) {
        return _plannedPath[_currentPathIndex + 1];
    }
    return nullptr;
}


bool Vehicle::tick()
{
    bool hit = false;
    hit = changePosVit();
    
    return hit;
}

ostream& operator<<(ostream& o, const Vehicle& v)
{   
    /*
    o << "#" << v._id << " Pos:" << v._pos.x() << "," << v._pos.y()
        << " State:" << (v._ended ? "EXITED" : (v._isWaiting ? "WAITING" : (v._started ? "MOVING" : "SPAWNING")))
        << " Segment:" << (v._plannedPath[&v.getPathIndex()] ? "YES" : "NO");
    */
    return o;
    
}


bool Vehicle::changePosVit()
{
    Time& time = _mdl.getTime();
    RoadTopo& rt = _mdl.getRoadTopo();

    // Ended my whole path
    if (_ended) {
        return false;
    }
    
    if (_started)  // drives on a segment
    {
        updatePositionOnSegment(time);
    }
    else // didn't start the road
    {
        initialiseOnFirstSegment(rt);
    }

    _t = time.t();
    return false;
}


void Vehicle::initialiseOnFirstSegment(RoadTopo& rt)
{
    if (rt.getEntries().empty() || rt.getExits().empty()) {
        //Log::it() << "Vehicle #" << _id << " ERROR: No entries or exits available" << endl;
        return;
    }

    if (_plannedPath.empty()) {
        //Log::it() << "Vehicle #" << _id << " ERROR: No planned path available" << endl;
        return;
    }

    const Segment* startSegment = _plannedPath[0];

    deque <const Vehicle*> vos = _mdl.getVehiculeFleet().vehiOnSegment(startSegment);

    double distancemin = startSegment->length();
    // iterate through each vehicle on the segment
    for (auto v : vos)
    {
        double d = (v->_pos - Vect3D(startSegment->A()->x(), startSegment->A()->y(), 0)).norme();
        if (d < distancemin)
        {
            distancemin = d;
        }
    }
    // then verify whether there is a vehicle between 2 and 3 sec ahead at least, and if thats the case then enter segment
    if (distancemin / startSegment->vmax() > Random::alea() + ParameterLoader::getInt("TimeAhead"))
    {
        _currentPathIndex = 0;
        _started = true;
        _distCurSeg = 0.0;
        _segmentStartTime = _mdl.getTime().t();
        //_entryTime = _mdl.getTime().t();
        _speed = startSegment->vmax();

        KPI::getKPI().recordVehicleEntry();
    }
    // follow the path of the segment whether its curved or straight
    _plannedPath[_currentPathIndex]->getPosVit(_distCurSeg, _pos, _vit);
}


void Vehicle::updatePositionOnSegment(Time& time)
{
    double lastTickDistance = _speed * time.dt();
    _distCurSeg += lastTickDistance;
    _distTotal += lastTickDistance;

    //_segmentProgress = _distCurSeg / _plannedPath[_currentPathIndex]->length();

    if (_distCurSeg < getCurrentSegment()->length())//(_segmentProgress < 1.0) 
    {
        _plannedPath[_currentPathIndex]->getPosVit(_distCurSeg, _pos, _vit);
    }
    else {
        finishCurrentSegment();
    }
}

void Vehicle::finishCurrentSegment()
{
    // Check if we've reached the target exit
    if (_plannedPath[_currentPathIndex] == _targetExit) {        
        // Release current segment only when truly exiting
        _vit = Vect3D(0.0, 0.0, 0.0);
        _ended = true;

        if (!_mdl.isSimulationRunning())
        {
            KPI::getKPI().recordVehicleCompletion(_mdl.getTime().t() - _entryTime, getTravelTimeOptimal());
            std::cout << _mdl.getTime().t() - _entryTime << endl;
        }
        Log::it() << "Vehicle #" << _id << " completed its at target exit" << endl;
    }
    else {
        // Try to move to next segment
        moveToNextSegmentInPath();
    }
}

const Segment* Vehicle::selectRandomExit(RoadTopo& rt)
{
    if (!rt.getExits().empty()) {
        int randomIndex = (int)(Random::alea() * rt.getExits().size());
        const Segment* exit = rt.getExits()[randomIndex];
        return exit;
    }
    return nullptr;
}


void Vehicle::moveToNextSegmentInPath()
{
    double prevdist = _plannedPath[_currentPathIndex]->length();
    int nextPathIndex = _currentPathIndex + 1;

    const Segment* nextSegment = _plannedPath[nextPathIndex];

    _speed = _plannedPath[nextPathIndex]->vmax(); // update the speed of the segment we're going to be on

    _currentPathIndex = nextPathIndex;
    _distCurSeg -= prevdist;
    //_segmentProgress = _distCurSeg/nextSegment->length();
    _segmentStartTime = _mdl.getTime().t();


    double segmentLength = _plannedPath[_currentPathIndex]->length();
}


bool Vehicle::planPathToExit(RoadTopo& rt)
{
    if (!_pathPlanner || !_targetExit) {
        return false;
    }

    int randomIndex = (int)(Random::alea() * rt.getEntries().size());

    const Segment* startSegment = rt.getEntries()[randomIndex];

    _plannedPath = _pathPlanner->findPath(startSegment, _targetExit);

    if (_plannedPath.empty()) {
        return false;
    }

    return true;
}


