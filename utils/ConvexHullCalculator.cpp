
#include "ConvexHullCalculator.h"
double ConvexHullCalculator::crossProduct(const Point &p1, const Point &p2, const Point &p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}
double ConvexHullCalculator::distanceSquared(const Point &p1, const Point &p2) {
    return (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
}

Point ConvexHullCalculator::parsePoint(const std::string &str) {
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
std::vector<Point> ConvexHullCalculator::grahamScan(std::vector<Point> points) {

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
double ConvexHullCalculator::calculateArea(const std::vector<Point> &hull) {

    if (hull.size() < 3) return 0.0;  // A polygon needs at least 3 vertices

    double area = 0.0;
    for (int i = 0; i < hull.size(); ++i) {
        int j = (i + 1) % hull.size();
        area += hull[i].x * hull[j].y - hull[j].x * hull[i].y;
    }

    return std::abs(area) / 2.0;
}
void ConvexHullCalculator::commandNewGraph(int n, const std::vector<std::string> &pointStrings) {
    points.clear();
    for (int i = 0; i < n && i < pointStrings.size(); ++i) {
        points.push_back(parsePoint(pointStrings[i]));
    }
}
double ConvexHullCalculator::commandCalculateHull() {
    if (points.empty()) {
        return 0.0;
    }
    std::vector<Point> hull = grahamScan(points);
    return calculateArea(hull);
}
void ConvexHullCalculator::commandAddPoint(const std::string &pointStr) {
    // Remove leading whitespace if present
    std::string trimmedStr = pointStr;
    trimmedStr.erase(0, trimmedStr.find_first_not_of(" \t"));

    Point newPoint = parsePoint(trimmedStr);
    points.push_back(newPoint);
}
bool ConvexHullCalculator::commandRemovePoint(const std::string &pointStr) {
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
std::string ConvexHullCalculator::processCommand(const std::string &command, std::vector<std::string> &followupLines) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "Newgraph") {
        int n;
        iss >> n;
        commandNewGraph(n, followupLines);
        return "Graph created with " + std::to_string(n) + " points.";
    }
    if (cmd == "CH") {
        double area = commandCalculateHull();
        return std::to_string(area);
    }
    if (cmd == "Newpoint") {
        std::string pointStr;
        std::getline(iss, pointStr); // Get the rest of the line
        commandAddPoint(pointStr);
        return "Point added.";
    }
    if (cmd == "Removepoint") {
        std::string pointStr;
        std::getline(iss, pointStr); // Get the rest of the line
        bool removed = commandRemovePoint(pointStr);
        if (removed) {
            return "Point removed.";
        }
        return "Point not found.";
    }
    if (cmd == "help") {
        return "Commands: Newgraph n, CH, Newpoint x,y, Removepoint x,y, help, exit";
    }
    if (cmd == "exit") {
        return "exit";
    }
    return "Unknown command. Type 'help' for available commands.";
}





