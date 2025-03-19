/*
** ch_server.cpp -- Convex Hull network server
** Based on selectserver.c by Beej, integrated with ConvexHullCalculator
*/
#ifndef CHSERVER_HPP
#define CHSERVER_HPP
#include "Server.hpp"
#include "ConvexHullCalculator.hpp"
ConvexHullCalculator calculator;
int isWaitingForPoints = 0;
#endif //CHSERVER_HPP
