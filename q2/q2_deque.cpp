#include "q2_deque.hpp"
#include <iostream>
#include <deque>
#include <algorithm>
#include <cmath>
#include <string>
using namespace std;

struct Point {
    float x, y;
    Point(float x = 0, float y = 0) : x(x), y(y) {}
};

// Calculate cross product of vectors p1p2 and p1p3
float crossProduct(Point p1, Point p2, Point p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

// Find the bottommost point (and leftmost if tied)
Point p0;
bool compare(Point &p1, Point &p2) {
    float cp = crossProduct(p0, p1, p2);
    if (cp == 0)
        return (p1.x + p1.y) < (p2.x + p2.y);
    return cp > 0;
}

// Calculate area of polygon using shoelace formula
float polygonArea(const deque<Point>& hull) {
    float area = 0.0;
    int n = hull.size();
    if (n < 3) return 0.0;

    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += hull[i].x * hull[j].y - hull[j].x * hull[i].y;
    }

    return abs(area) / 2.0;
}

// Graham Scan algorithm to find convex hull
deque<Point> grahamScan(deque<Point>& points) {
    int n = points.size();
    if (n < 3) return points;  // Cannot form a convex hull with less than 3 points

    // Find bottommost point
    int min_idx = 0;
    for (int i = 1; i < n; i++) {
        if ((points[i].y < points[min_idx].y) ||
            (points[i].y == points[min_idx].y && points[i].x < points[min_idx].x)) {
            min_idx = i;
        }
    }

    // Place bottommost point at first position
    swap(points[0], points[min_idx]);
    p0 = points[0];

    // Sort points based on polar angle
    sort(points.begin() + 1, points.end(), compare);

    // Build convex hull
    deque<Point> hull;
    hull.push_back(points[0]);
    hull.push_back(points[1]);
    hull.push_back(points[2]);

    // Process remaining points
    for (int i = 3; i < n; i++) {
        while (hull.size() > 1 && crossProduct(hull[hull.size()-2],
               hull[hull.size()-1], points[i]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }

    return hull;
}

// Parse a string in format "x,y" to get coordinates
Point parsePoint(const string& str) {
    size_t comma_pos = str.find(',');
    if (comma_pos != string::npos) {
        float x = stof(str.substr(0, comma_pos));
        float y = stof(str.substr(comma_pos + 1));
        return Point(x, y);
    }
    return Point(0, 0); // Default return in case of parsing error
}

int main() {
    int n;
    cin >> n;
    cin.ignore(); // Consume the newline after n

    deque<Point> points;
    string line;

    // Read points
    for (int i = 0; i < n; i++) {
        getline(cin, line);
        points.push_back(parsePoint(line));
    }

    // Calculate convex hull
    deque<Point> hull = grahamScan(points);

    // Calculate and output area
    cout << polygonArea(hull) << endl;

    return 0;
}