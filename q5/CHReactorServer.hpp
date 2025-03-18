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

    // Create a single instance of the convex hull calculator
    ConvexHullCalculator calculator;
    reactor_t* reactor_p;
    void handleRequest(int clientfd);
    void handleCommand(const std::string &command);
    void handleAddPoint(int clientfd);
    void handleAcceptClient(int fd);
    void init();
    int run();
    void stop();
    void start();




#endif //CHREACTORSERVER_HPP
