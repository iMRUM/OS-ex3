#ifndef TEMP_SERVER_HPP
#define TEMP_SERVER_HPP
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

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

struct sockaddr_storage remoteaddr; // client address
socklen_t addrlen;


char remoteIP[INET6_ADDRSTRLEN];


void handleRequest(int clientfd);

void handleCommand(int clientfd, const std::string &input_command);

void handleAcceptClient(int fd_listener);

void init();

int run();

void stop();

void start();


#endif //TEMP_SERVER_HPP
