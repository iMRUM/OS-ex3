#include "q2_list.hpp"
#include <iostream>
#include <list>
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
float polygonArea(list<Point>& hull) {
    float area = 0.0;
    if (hull.size() < 3) return 0.0;

    auto current = hull.begin();
    auto prev = std::prev(hull.end());

    while (current != hull.end()) {
        area += (prev->x + current->x) * (prev->y - current->y);
        prev = current;
        ++current;
    }

    return abs(area) / 2.0;
}

// Graham Scan algorithm to find convex hull
list<Point> grahamScan(list<Point>& points) {
    int n = points.size();
    if (n < 3) return points;  // Cannot form a convex hull with less than 3 points

    // Find bottommost point
    auto min_it = points.begin();
    float ymin = min_it->y;

    for (auto it = next(points.begin()); it != points.end(); ++it) {
        if ((it->y < ymin) || (ymin == it->y && it->x < min_it->x)) {
            ymin = it->y;
            min_it = it;
        }
    }

    // Store the bottommost point and remove it from the list
    p0 = *min_it;
    points.erase(min_it);

    // Sort remaining points based on polar angle
    points.sort(compare);

    // Reconstruct the list with p0 at the beginning
    points.push_front(p0);

    // Build convex hull
    list<Point> hull;
    auto it = points.begin();

    // Add first three points
    hull.push_back(*it++);
    if (it == points.end()) return hull;
    hull.push_back(*it++);
    if (it == points.end()) return hull;
    hull.push_back(*it++);

    // Process remaining points
    while (it != points.end()) {
        while (hull.size() > 1) {
            Point p2 = hull.back();
            hull.pop_back();
            Point p1 = hull.back();

            if (crossProduct(p1, p2, *it) > 0) {
                hull.push_back(p2);
                break;
            }
        }
        hull.push_back(*it++);
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

    list<Point> points;
    string line;

    // Read points
    for (int i = 0; i < n; i++) {
        getline(cin, line);
        points.push_back(parsePoint(line));
    }

    // Calculate convex hull
    list<Point> hull = grahamScan(points);

    // Calculate and output area
    cout << polygonArea(hull) << endl;

    return 0;
}
