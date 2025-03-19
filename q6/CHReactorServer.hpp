#ifndef CHREACTORSERVER_HPP
#define CHREACTORSERVER_HPP
#define PORT "9034"
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
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <csignal>
#include "../utils/ConvexHullCalculator.hpp"
#include "../Reactor/include/Reactor.hpp"

    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];


    ConvexHullCalculator calculator;
    reactor_t* reactor_p;
    int isWaitingForPoints = 0;
    void handleRequest(int clientfd);
    void handleCommand(int clientfd, const std::string &input_command);
    void handleAcceptClient(int fd);
    void init();
    int run();
    void stop();
    void start();




#endif //CHREACTORSERVER_HPP
