#ifndef CHREACTORSERVER_HPP
#define CHREACTORSERVER_HPP
#define PORT "9034"   // port we're listening on
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <map>
#include "../utils/ConvexHullCalculator.hpp"
#include "../Reactor/include/Reactor.hpp"

    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;
    // Create a single instance of the convex hull calculator
    ConvexHullCalculator calculator;
    reactor_t* reactorInstance;
    void handleRequest(int fd);
    void handleCommand(int clientfd);
    void handleAddPoint(int clientfd);
    void handleAcceptClient(int fd);
    void init();
    int run();
    void stop();
    void start();




#endif //CHREACTORSERVER_HPP
