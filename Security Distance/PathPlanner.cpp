#include "PathPlanner.h"
#include "Topo.h"
#include "Log.h"
#include <cmath>
#include <algorithm>

PathPlanner::PathPlanner()
{
}

PathPlanner::~PathPlanner()
{
}

vector<const Segment*> PathPlanner::findPath(const Segment* start, const Segment* goal)
{
    if (!start || !goal) {
        return vector<const Segment*>();
    }

    if (start == goal) {
        return vector<const Segment*>{start};
    }

    // Priority queue for open list
    priority_queue<AStarNode*, vector<AStarNode*>, NodeComparator> openList;

    // Sets to track visited segments
    unordered_set<const Segment*> closedSet;
    unordered_map<const Segment*, AStarNode*> openMap;

    // Vector to store all created nodes for cleanup
    vector<AStarNode*> allNodes;

    // Create start node
    double startHeuristic = calculateHeuristic(start, goal);
    AStarNode* startNode = new AStarNode(start, 0.0, startHeuristic, nullptr);
    allNodes.push_back(startNode);

    openList.push(startNode);
    openMap[start] = startNode;

    while (!openList.empty()) {
        // Get node with lowest fCost
        AStarNode* currentNode = openList.top();
        openList.pop();
        openMap.erase(currentNode->segment);

        // Move current node to closed set
        closedSet.insert(currentNode->segment);

        // Check if we reached the goal
        if (currentNode->segment == goal) {
            Log::it() << "PathPlanner: Path found Total cost: "
                << (currentNode->gCost + goal->length())
                << " (includes final segment)" << endl;
            vector<const Segment*> path = reconstructPath(currentNode);

            // Cleanup all nodes
            for (AStarNode* node : allNodes) 
            {
                delete node;
            }
            return path;
        }

        // Explore neighbors (connected segments via _next)
        const Segment* currentSegment = currentNode->segment;
        for (int i = 0; i < currentSegment->nNext(); i++) {
            const Segment* neighbor = (*currentSegment)[i];

            if (!neighbor) continue;

            // Skip if neighbor is in closed set
            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            // Calculate costs
            double tentativeGCost = currentNode->gCost + currentSegment->length();
            double hCost = calculateHeuristic(neighbor, goal);

            // Check if neighbor is already in open list
            AStarNode* existingNode = nullptr;
            unordered_map<const Segment*, AStarNode*>::iterator openIt = openMap.find(neighbor);
            if (openIt != openMap.end()) {
                existingNode = openIt->second;
            }

            // If neighbor not in open list or we found a better path
            if (!existingNode || tentativeGCost < existingNode->gCost) {
                if (existingNode) {
                    // Update existing node
                    existingNode->gCost = tentativeGCost;
                    existingNode->fCost = tentativeGCost + hCost;
                    existingNode->parent = currentNode;
                }
                else {
                    // Create new node and add to open list
                    AStarNode* newNode = new AStarNode(neighbor, tentativeGCost, hCost, currentNode);
                    allNodes.push_back(newNode);
                    openList.push(newNode);
                    openMap[neighbor] = newNode;
                }
            }
        }
    }

    // Cleanup all nodes
    for (AStarNode* node : allNodes) {
        delete node;
    }

    return vector<const Segment*>();
}

//
// calculateHeuristic
// guides the direction of the search, but is not the real distance the vehicle will be travelling
//

double PathPlanner::calculateHeuristic(const Segment* from, const Segment* to)
{
    if (!from || !to) return 0.0;

    // Vehicle is at start of the first segment and wants to reach the end of the last segment
    const Point* currentPos = from->A();  // Start of segment
    const Point* goalEnd = to->B();       // End of goal segment

    double dx = goalEnd->x() - currentPos->x();
    double dy = goalEnd->y() - currentPos->y();
    double dz = goalEnd->z() - currentPos->z();

    return sqrt(dx * dx + dy * dy + dz * dz);
}

//
// reconstructPath
// builds the optimal path found
//

vector<const Segment*> PathPlanner::reconstructPath(const AStarNode* goalNode)
{
    vector<const Segment*> path;
    const AStarNode* current = goalNode;

    // Trace back from goal to start
    while (current != nullptr) {
        path.push_back(current->segment);
        current = current->parent;
    }

    // Reverse to get path from start to goal
    reverse(path.begin(), path.end());

    return path;
}