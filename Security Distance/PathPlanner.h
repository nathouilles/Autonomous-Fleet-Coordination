#pragma once

#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "Segment.h"

using namespace std;

class PathPlanner
{
private:
    // A* Node structure
    struct AStarNode {
        const Segment* segment;
        double gCost;           // Actual cost from start
        double hCost;           // Heuristic cost to goal  
        double fCost;           // g + h
        const AStarNode* parent;

        AStarNode(const Segment* seg, double g, double h, const AStarNode* p)
            : segment(seg), gCost(g), hCost(h), fCost(g + h), parent(p) {
        }
    };

    // Comparator for priority queue (min-heap based on fCost)
    struct NodeComparator {
        bool operator()(const AStarNode* a, const AStarNode* b) const {
            return a->fCost > b->fCost; // Lower fCost has higher priority
        }
    };

public:
    PathPlanner();
    ~PathPlanner();

    // Main A* pathfinding function
    vector<const Segment*> findPath(const Segment* start, const Segment* goal);

private:
    // Calculate heuristic distance between two segments
    double calculateHeuristic(const Segment* from, const Segment* to);

    // Reconstruct path from goal node back to start
    vector<const Segment*> reconstructPath(const AStarNode* goalNode);
};