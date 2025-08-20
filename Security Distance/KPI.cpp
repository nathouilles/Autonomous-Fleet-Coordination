#include "KPI.h"
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

KPI::KPI()
    : _kpiFile("kpi_results.txt"), _ic("KPI"), _simulationStarted(false)
{
    // Initialise default counters
    _counters["vehicles_entered"] = 0;
    _counters["vehicles_exited"] = 0;

    // Initialise default metrics
    _metrics["total_travel_time"] = 0.0;
    _metrics["average_travel_time"] = 0.0;
    _metrics["traffic_flow_rate"] = 0.0;
    _metrics["current_traffic_density"] = 0.0;
    _metrics["max_traffic_density"] = 0.0;
    _metrics["min_near_miss_distance"] = 1000.0; // Initialise to large value

    _metrics["total_optimal_travel_time"] = 0.0;
    _metrics["simulation_duration"] = 0.0;

    // Write header to KPI file
    if (_kpiFile.is_open()) {
        time_t now = time(0);
        char* timeStr = ctime(&now);
        _kpiFile << "Simulation started: " << timeStr << endl;
    }
}

KPI::~KPI()
{
    if (_simulationStarted) {
        endSimulation();
    }
    if (_kpiFile.is_open()) {
        _kpiFile.close();
    }
}

KPI& KPI::getKPI()
{
    static KPI instance;
    return instance;
}

void KPI::startSimulation()
{
    _simulationStartTime = chrono::steady_clock::now();
    _lastUpdateTime = _simulationStartTime;
    _simulationStarted = true;
}

void KPI::endSimulation()
{
    if (!_simulationStarted) return;

    chrono::steady_clock::time_point endTime = chrono::steady_clock::now();
    chrono::duration<double> elapsed = endTime - _simulationStartTime;
    _metrics["simulation_duration"] = elapsed.count();

    // Calculate final metrics
    if (_counters["vehicles_completed"] > 0) {
        _metrics["average_travel_time"] = _metrics["total_travel_time"] / _counters["vehicles_completed"];
    }

    if (_metrics["simulation_duration"] > 0) {
        _metrics["traffic_flow_rate"] = _counters["vehicles_completed"] / _metrics["simulation_duration"];
    }

    saveToFile();
    _simulationStarted = false;
}

void KPI::updateMetrics(double currentTime)
{
    chrono::steady_clock::time_point now = chrono::steady_clock::now();
    chrono::duration<double> elapsed = now - _simulationStartTime;
    _metrics["simulation_duration"] = elapsed.count();

    // Write real-time data to file
    if (_kpiFile.is_open()) {
        _kpiFile << fixed << setprecision(2) << currentTime << ","
            << getActiveVehicles() << ","
            << _metrics["current_traffic_density"] << ","
            << _counters["vehicles_completed"] << endl;
    }

    _lastUpdateTime = now;
}

void KPI::incrementCounter(const string& counterName)
{
    _counters[counterName]++;
}

void KPI::setCounter(const string& counterName, int value)
{
    _counters[counterName] = value;
}

int KPI::getCounter(const string& counterName) const
{
    map<string, int>::const_iterator it = _counters.find(counterName);
    return (it != _counters.end()) ? it->second : 0;
}

void KPI::setMetric(const string& metricName, double value)
{
    _metrics[metricName] = value;
}

void KPI::addToMetric(const string& metricName, double value)
{
    _metrics[metricName] += value;
}

double KPI::getMetric(const string& metricName) const
{
    map<string, double>::const_iterator it = _metrics.find(metricName);
    return (it != _metrics.end()) ? it->second : 0.0;
}

void KPI::recordVehicleEntry()
{
    incrementCounter("vehicles_entered");
}

void KPI::recordVehicleExit()
{
    incrementCounter("vehicles_exited");
}

void KPI::recordVehicleCompletion(double travelTime, double optimalTime)
{
    incrementCounter("vehicles_completed");
    addToMetric("total_travel_time", travelTime);
    addToMetric("total_optimal_travel_time", optimalTime);

    // Update average travel time in real-time
    if (_counters["vehicles_completed"] > 0) {
        _metrics["average_travel_time"] = _metrics["total_travel_time"] / _counters["vehicles_completed"];
    }
}

void KPI::recordCollisionAvoided()
{
    incrementCounter("collisions_avoided");
}

void KPI::recordNearMiss(double distance)
{
    incrementCounter("near_misses");
    if (distance < _metrics["min_near_miss_distance"]) {
        _metrics["min_near_miss_distance"] = distance;
    }
}

void KPI::updateTrafficDensity(int vehiclesOnRoad)
{
    _metrics["current_traffic_density"] = static_cast<double>(vehiclesOnRoad);

    if (vehiclesOnRoad > _metrics["max_traffic_density"]) {
        _metrics["max_traffic_density"] = static_cast<double>(vehiclesOnRoad);
    }
}

void KPI::recordOptimalTravelTime(double optimalTime)
{
    addToMetric("total_optimal_travel_time", optimalTime);
}

void KPI::recordSegmentTransition()
{
    incrementCounter("segment_transitions");
}

void KPI::dumpKPIs() const
{
    cout << "Vehicles entered: " << getCounter("vehicles_entered") << endl;
    cout << "Vehicles completed: " << getCounter("vehicles_completed") << endl;
    cout << "Collisions avoided: " << getCounter("collisions_avoided") << endl;
    cout << "Current traffic density: " << _metrics.at("current_traffic_density") << endl;
    cout << "Average travel time: " << _metrics.at("average_travel_time") << "s" << endl;
    cout << "Simulation time: " << _metrics.at("simulation_duration") << "s" << endl;
}

void KPI::saveToFile()
{
    if (!_kpiFile.is_open()) return;

    _kpiFile << "Simulation Duration: " << _metrics.at("simulation_duration") << " seconds" << endl;
    _kpiFile << endl << "Vehicle Statistics:" << endl;
    _kpiFile << "  Total Vehicles Entered: " << _counters.at("vehicles_entered") << endl;
    _kpiFile << "  Total Vehicles Completed: " << _counters.at("vehicles_completed") << endl;

    if (_counters.at("vehicles_entered") > 0) {
        double completionRate = static_cast<double>(_counters.at("vehicles_completed")) / _counters.at("vehicles_entered") * 100.0;
        _kpiFile << "  Completion Rate: " << fixed << setprecision(1) << completionRate << "%" << endl;
    }

    _kpiFile << endl << "Traffic Flow Statistics:" << endl;
    _kpiFile << "  Average Travel Time: " << fixed << setprecision(2) << _metrics.at("average_travel_time") << " seconds" << endl;
    _kpiFile << "  Traffic Flow Rate: " << fixed << setprecision(2) << _metrics.at("traffic_flow_rate") << " vehicles/second" << endl;
    _kpiFile << "  Maximum Traffic Density: " << _metrics.at("max_traffic_density") << " vehicles" << endl;

    // Calculate and display the ratio using VehicleFleet's total optimal time
    double totalActualTime = _metrics.at("total_travel_time");
    double totalOptimalTime = _metrics.at("total_optimal_travel_time");

    if (totalOptimalTime > 0.0) {
        double ratio = totalActualTime / totalOptimalTime;
        _kpiFile << "  Ratio To Objective time: " << fixed << setprecision(3) << ratio << endl;
        _kpiFile << "  (Total actual: " << fixed << setprecision(2) << totalActualTime
            << "s, Total optimal: " << totalOptimalTime << "s)" << endl;
    }
    else {
        _kpiFile << "  problem in data" << endl;
    }

    _kpiFile.flush();
}


double KPI::getSimulationTime() const
{
    if (!_simulationStarted) return 0.0;

    chrono::steady_clock::time_point now = chrono::steady_clock::now();
    chrono::duration<double> elapsed = now - _simulationStartTime;
    return elapsed.count();
}

double KPI::getAverageTravelTime() const
{
    return _metrics.at("average_travel_time");
}

double KPI::getTrafficFlowRate() const
{
    return _metrics.at("traffic_flow_rate");
}

int KPI::getTotalVehicles() const
{
    return _counters.at("vehicles_entered");
}

int KPI::getActiveVehicles() const
{
    return _counters.at("vehicles_entered") - _counters.at("vehicles_completed") - _counters.at("vehicles_exited");
}

void KPI::setTotalOptimalTime(double totalOptimalTime)
{
    setMetric("total_optimal_travel_time", totalOptimalTime);
}