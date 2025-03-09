#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

struct Point {
    double x, y;

    // Constructor
    Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}
};

// Function to calculate the cross product of vectors p1p2 and p1p3
double crossProduct(const Point& p1, const Point& p2, const Point& p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

// Function to calculate the square of the distance between two points
double distanceSquared(const Point& p1, const Point& p2) {
    return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
}

// Graham Scan algorithm to find the convex hull
std::vector<Point> grahamScan(std::vector<Point>& points) {
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
    std::sort(points.begin() + 1, points.end(), [&pivot](const Point& p1, const Point& p2) {
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

int main() {
    int n;
    std::cin >> n;

    // Consume the newline after the number
    std::cin.ignore();

    std::vector<Point> points(n);

    for (int i = 0; i < n; ++i) {
        std::string line;
        std::getline(std::cin, line);

        std::size_t commaPos = line.find(',');
        if (commaPos != std::string::npos) {
            std::string xStr = line.substr(0, commaPos);
            std::string yStr = line.substr(commaPos + 1);
            points[i].x = std::stod(xStr);
            points[i].y = std::stod(yStr);
        }
    }

    std::vector<Point> hull = grahamScan(points);
    double area = calculateArea(hull);

    // Output the area
    std::cout << area << std::endl;

    return 0;
}