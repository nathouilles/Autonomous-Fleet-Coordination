#pragma once

#include <map>
#include <string>
#include <fstream>
#include <chrono>
#include "InstanceCount.h"

using namespace std;

class KPI
{
private:
    map<string, double> _metrics;
    map<string, int> _counters;
    chrono::steady_clock::time_point _simulationStartTime;
    chrono::steady_clock::time_point _lastUpdateTime;
    ofstream _kpiFile;
    InstanceCount _ic;
    bool _simulationStarted;

    // Private constructor for singleton
    KPI();

public:
    ~KPI();

    // accessor
    static KPI& getKPI();

    // Simulation lifecycle
    void startSimulation();
    void endSimulation();
    void updateMetrics(double currentTime);

    // Counter management
    void incrementCounter(const string& counterName);
    void setCounter(const string& counterName, int value);
    int getCounter(const string& counterName) const;

    // Metric management
    void setMetric(const string& metricName, double value);
    void addToMetric(const string& metricName, double value);
    double getMetric(const string& metricName) const;

    // Vehicle-specific KPIs
    void recordVehicleEntry();
    void recordVehicleExit();
    void recordVehicleCompletion(double travelTime, double optimalTime);
    void recordCollisionAvoided();
    void recordNearMiss(double distance);
    void recordOptimalTravelTime(double optimalTime);

    // Traffic flow KPIs
    void updateTrafficDensity(int vehiclesOnRoad);
    void recordSegmentTransition();

    // Output and reporting
    void dumpKPIs() const;
    void saveToFile();

    // Real-time access
    double getSimulationTime() const;
    double getAverageTravelTime() const;
    double getTrafficFlowRate() const;
    int getTotalVehicles() const;
    int getActiveVehicles() const;

    void setTotalOptimalTime(double totalOptimalTime);
};