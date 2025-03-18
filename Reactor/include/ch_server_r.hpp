#ifndef CH_SERVER_R_H
#define CH_SERVER_R_H

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
#include "../../utils/ConvexHullCalculator.h"
#include "Reactor.h"
#include <string>


// Function to get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa);

// Process a single line from a client
void processClientLine(int clientfd, const std::string &line);

// Callback for handling new connections
void handleNewConnection(int listener);

// Callback for handling client data
void handleClientData(int clientfd);

// Main function
int main(void);

#endif // CH_SERVER_R_H */
