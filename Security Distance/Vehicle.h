#pragma once

#include "Vect3D.h"
#include "InstanceCount.h"
#include <vector>
#include <iostream>
using namespace std;

// Forward declarations
class Model;
class RoadTopo;
class Segment;
class PathPlanner;
class Time;

class Vehicle
{
private:
    Vect3D _pos, _vit;
    int _id;
    InstanceCount _ic;
    Model& _mdl;
    double _t, _speed, _width, _length, _distCurSeg, _distTotal;

    bool _ended, _started;
    double _entryTime, _exitTime;

    // Road navigation
    double _segmentStartTime;

    //for collisions
    bool _coll;

    // Path planning
    PathPlanner* _pathPlanner;
    vector<const Segment*> _plannedPath;
    int _currentPathIndex;
    const Segment* _targetExit;

    // Waiting state for collision prevention

    static int nextId();

    void initialiseOnFirstSegment(RoadTopo& rt);
    void updatePositionOnSegment(Time& time);
    void finishCurrentSegment();
    const Segment* selectRandomExit(RoadTopo& rt);
    bool planPathToExit(RoadTopo& rt);
    void moveToNextSegmentInPath();
    bool changePosVit();

public:
    Vehicle(Model& m, double width, double length);
    ~Vehicle();

    bool tick();

    // Accessors
    double width() const { return _width; }
    double length() const { return _length; }

    const Vect3D& position() const { return _pos; }
    const Vect3D& vitesse() const { return _vit; }
    int getId() const { return _id; }

    bool hasStarted() const { return _started; }
    bool hasExited() const { return _ended; }
    bool running() const { return _started && !_ended; }    //is on the road or not ?

    bool hasSegment() const { return _plannedPath[_currentPathIndex] != nullptr; }
    const Segment* getCurrentSegment() const { return _plannedPath[_currentPathIndex]; }
    double getDistOnSeg() const { return _distCurSeg; }
    bool onLastSegment() const { return _plannedPath[_currentPathIndex] == _plannedPath[_plannedPath.size() - 1]; }

    //For time management
    double getTravelTimeOptimal() const;
    int getPathIndex() const { return _currentPathIndex; }


    //collision management
    bool collides(const Vehicle*) const;        // collides with another vehicle
    void footprint(Vect3D[4]) const;            //get the footprint of the vehicle
    void setcol(bool x) { _coll = x; }          //set collision to true or false
    bool getcoll() const { return _coll; }      //collision ?
    double timeToReachSeg(const Segment*) const;

    double getSpeed() const { return _speed; }

    void stopVehi();
    void restartVehi();
    void reinitForSimulation();

    // Get next segment in path for collision management
    const Segment* getNextSegmentInPath() const;

    friend ostream& operator<<(ostream& o, const Vehicle& v);
};