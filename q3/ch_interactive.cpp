#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>

struct Point {
    double x, y;

    // Constructor
    Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}
    
    // Equality comparison for finding points
    bool operator==(const Point& other) const {
        return (fabs(x - other.x) < 1e-9 && fabs(y - other.y) < 1e-9);
    }
};

class ConvexHullCalculator {
private:
    std::vector<Point> points;

    // Function to calculate the cross product of vectors p1p2 and p1p3
    double crossProduct(const Point& p1, const Point& p2, const Point& p3) {
        return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    }

    // Function to calculate the square of the distance between two points
    double distanceSquared(const Point& p1, const Point& p2) {
        return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
    }

    // Function to parse a point from a string (format: "x,y")
    Point parsePoint(const std::string& str) {
        std::size_t commaPos = str.find(',');
        if (commaPos != std::string::npos) {
            std::string xStr = str.substr(0, commaPos);
            std::string yStr = str.substr(commaPos + 1);
            try {
                double x = std::stod(xStr);
                double y = std::stod(yStr);
                return Point(x, y);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing point: " << str << std::endl;
            }
        }
        return Point(0, 0); // Default return if parsing fails
    }

public:
    // Constructor
    ConvexHullCalculator() {}

    // Graham Scan algorithm to find the convex hull
    std::vector<Point> grahamScan(std::vector<Point> points) {
        int n = points.size();
        if (n <= 2) return points;  // Handle edge cases

        // Find the lowest point (and if tied, the leftmost)
        int lowestIdx = 0;
        for (int i = 1; i < n; ++i) {
            if (points[i].y < points[lowestIdx].y ||
                (points[i].y == points[lowestIdx].y && points[i].x < points[lowestIdx].x)) {
                lowestIdx = i;
            }
        }

        // Make the lowest point the first point
        std::swap(points[0], points[lowestIdx]);

        // Sort points by polar angle with respect to the lowest point
        Point pivot = points[0];
        std::sort(points.begin() + 1, points.end(), [&pivot, this](const Point& p1, const Point& p2) {
            double cross = crossProduct(pivot, p1, p2);
            if (fabs(cross) < 1e-9) {  // If collinear, sort by distance from pivot
                return distanceSquared(pivot, p1) < distanceSquared(pivot, p2);
            }
            return cross > 0;  // Counter-clockwise orientation
        });

        // Construct the convex hull using a stack
        std::vector<Point> hull;
        hull.push_back(points[0]);

        // Handle collinear points and ensure we have at least 2 points if available
        if (n > 1) {
            hull.push_back(points[1]);
        }

        for (int i = 2; i < n; ++i) {
            while (hull.size() > 1 && crossProduct(hull[hull.size() - 2], hull.back(), points[i]) <= 0) {
                hull.pop_back();
            }
            hull.push_back(points[i]);
        }

        return hull;
    }

    // Calculate area of the convex hull using the Shoelace formula
    double calculateArea(const std::vector<Point>& hull) {
        if (hull.size() < 3) return 0.0;  // A polygon needs at least 3 vertices

        double area = 0.0;
        for (int i = 0; i < hull.size(); ++i) {
            int j = (i + 1) % hull.size();
            area += hull[i].x * hull[j].y - hull[j].x * hull[i].y;
        }

        return std::abs(area) / 2.0;
    }

    // Command: Create a new graph with n points
    void commandNewGraph(int n) {
        points.clear();
        for (int i = 0; i < n; ++i) {
            std::string pointStr;
            if (std::getline(std::cin, pointStr)) {
                points.push_back(parsePoint(pointStr));
            } else {
                break;
            }
        }
    }

    // Command: Calculate and display the convex hull area
    double commandCalculateHull() {
        if (points.empty()) {
            return 0.0;
        } else {
            std::vector<Point> hull = grahamScan(points);
            return calculateArea(hull);
        }
    }

    // Command: Add a new point to the current graph
    void commandAddPoint(const std::string& pointStr) {
        // Remove leading whitespace if present
        std::string trimmedStr = pointStr;
        trimmedStr.erase(0, trimmedStr.find_first_not_of(" \t"));
        
        Point newPoint = parsePoint(trimmedStr);
        points.push_back(newPoint);
    }

    // Command: Remove a point from the current graph
    bool commandRemovePoint(const std::string& pointStr) {
        // Remove leading whitespace if present
        std::string trimmedStr = pointStr;
        trimmedStr.erase(0, trimmedStr.find_first_not_of(" \t"));
        
        Point targetPoint = parsePoint(trimmedStr);
        
        // Find and remove the point if it exists
        auto it = std::find(points.begin(), points.end(), targetPoint);
        if (it != points.end()) {
            points.erase(it);
            return true;
        }
        return false;
    }

    // Process a command from a string
    std::string processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "Newgraph") {
            int n;
            iss >> n;
            commandNewGraph(n);
            return "Graph created with " + std::to_string(n) + " points.";
        } 
        else if (cmd == "CH") {
            double area = commandCalculateHull();
            return std::to_string(area);
        } 
        else if (cmd == "Newpoint") {
            std::string pointStr;
            std::getline(iss, pointStr); // Get the rest of the line
            commandAddPoint(pointStr);
            return "Point added.";
        } 
        else if (cmd == "Removepoint") {
            std::string pointStr;
            std::getline(iss, pointStr); // Get the rest of the line
            bool removed = commandRemovePoint(pointStr);
            if (removed) {
                return "Point removed.";
            } else {
                return "Point not found.";
            }
        } 
        else if (cmd == "exit" || cmd.empty()) {
            return "exit";
        }
        else {
            return "Unknown command.";
        }
    }
};

int main() {
    ConvexHullCalculator calculator;
    std::string command;

    while (std::getline(std::cin, command)) {
        std::string result = calculator.processCommand(command);
        
        if (result == "exit") {
            break;
        }
        
        std::cout << result << std::endl;
    }

    return 0;
}