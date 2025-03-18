/*
** ch_server.cpp -- Convex Hull network server
** Based on selectserver.c by Beej, integrated with ConvexHullCalculator
*/
#ifndef CHSERVER_HPP
#define CHSERVER_HPP
#include "Server.hpp"
#include "ConvexHullCalculator.h"
class CHServer : public Server {
protected:
    // Create a single instance of the convex hull calculator
    ConvexHullCalculator calculator;

    // Store client state for multi-line commands
    std::map<int, std::vector<std::string>> client_pending_lines;
    std::map<int, int> client_command_state; // 0 = normal, 1 = expecting points for Newgraph
    std::map<int, int> client_points_needed; // For Newgraph command
    int run() override;
    void stop() override;
};
#endif //CHSERVER_HPP
