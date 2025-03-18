//
// Created by imry on 3/11/25.
//

#ifndef CONVEXHULLCALCULATOR_H
#define CONVEXHULLCALCULATOR_H
#ifndef CONVEX_HULL_CALCULATOR_H
#define CONVEX_HULL_CALCULATOR_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

// Point structure needed by the ConvexHullCalculator
struct Point {
    double x, y;

    Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}

    bool operator==(const Point& other) const {
        return (fabs(x - other.x) < 1e-9 && fabs(y - other.y) < 1e-9);
    }
};

class ConvexHullCalculator {
private:
    std::vector<Point> points;

    // Function to calculate the cross product of vectors p1p2 and p1p3
    double crossProduct(const Point& p1, const Point& p2, const Point& p3);

    // Function to calculate the square of the distance between two points
    double distanceSquared(const Point& p1, const Point& p2);

    // Function to parse a point from a string (format: "x,y")
    Point parsePoint(const std::string& str);

public:
    // Constructor
    ConvexHullCalculator(){}

    // Graham Scan algorithm to find the convex hull
    std::vector<Point> grahamScan(std::vector<Point> points);

    // Calculate area of the convex hull using the Shoelace formula
    double calculateArea(const std::vector<Point>& hull);

    // Command: Create a new graph with n points
    void commandNewGraph(int n);

    // Command: Create a new graph with n points
    void commandNewGraph(int n, const std::vector<std::string>& pointStrings);

    // Command: Calculate and display the convex hull area
    double commandCalculateHull();

    // Command: Add a new point to the current graph
    void commandAddPoint(const std::string& pointStr);

    void commandAddPoint(Point new_point);
    // Command: Remove a point from the current graph
    bool commandRemovePoint(const std::string& pointStr);

    // Process a command from a string
    std::string processCommand(const std::string& command, std::vector<std::string>& followupLines);
};

#endif // CONVEX_HULL_CALCULATOR_H

#endif //CONVEXHULLCALCULATOR_H
