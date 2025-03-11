/*
** ch_server.cpp -- Convex Hull network server
** Based on selectserver.c by Beej, integrated with ConvexHullCalculator
*/

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
#include "../utils/ConvexHullCalculator.h"

#define PORT "9034"   // port we're listening on

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1;      // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    // Create a single instance of the convex hull calculator
    ConvexHullCalculator calculator;

    // Store client state for multi-line commands
    std::map<int, std::vector<std::string>> clientPendingLines;
    std::map<int, int> clientCommandState; // 0 = normal, 1 = expecting points for Newgraph
    std::map<int, int> clientPointsNeeded; // For Newgraph command

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    std::cout << "Convex Hull server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;
        //HERE SHOULD BE START_REACTOR(?) INSTEAD OF MAIN LOOP, THE REACTOR WILL HANDLE EVERYTHING
    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }

                        // Initialize client state
                        clientCommandState[newfd] = 0;
                        clientPointsNeeded[newfd] = 0;

                        printf("New connection from %s on socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);

                        // Send welcome message to the new client
                        std::string welcome = "Welcome to the Convex Hull Server.\nType 'help' for available commands.\n";
                        send(newfd, welcome.c_str(), welcome.length(), 0);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf - 1, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("Socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }

                        // Clean up client data
                        clientPendingLines.erase(i);
                        clientCommandState.erase(i);
                        clientPointsNeeded.erase(i);

                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // We got some data from a client
                        buf[nbytes] = '\0'; // Null-terminate the string

                        // Process the received data line by line
                        std::string data(buf);
                        std::istringstream stream(data);
                        std::string line;

                        while (std::getline(stream, line)) {
                            // Remove carriage return if present (handles Windows line endings)
                            if (!line.empty() && line.back() == '\r') {
                                line.pop_back();
                            }

                            if (clientCommandState[i] == 0) {
                                // Normal command processing mode
                                if (line.substr(0, 8) == "Newgraph") {
                                    // Extract the number of points
                                    std::istringstream iss(line);
                                    std::string cmd;
                                    int n;
                                    iss >> cmd >> n;

                                    if (n > 0) {
                                        clientCommandState[i] = 1;  // Switch to point collection mode
                                        clientPointsNeeded[i] = n;  // Set the number of points to collect
                                        clientPendingLines[i].clear();  // Clear any existing points

                                        // Send acknowledgment
                                        std::string response = "Please enter " + std::to_string(n) + " points, one per line:\n";
                                        send(i, response.c_str(), response.length(), 0);
                                    } else {
                                        std::string response = "Invalid number of points.\n";
                                        send(i, response.c_str(), response.length(), 0);
                                    }
                                } else {
                                    // Process regular command
                                    std::vector<std::string> empty;
                                    std::string result = calculator.processCommand(line, empty);
                                    result += "\n";  // Add newline for client display

                                    send(i, result.c_str(), result.length(), 0);

                                    // Check if this was an exit command
                                    if (result == "exit\n") {
                                        printf("Client %d requested exit\n", i);
                                        close(i);
                                        FD_CLR(i, &master);

                                        // Clean up client data
                                        clientPendingLines.erase(i);
                                        clientCommandState.erase(i);
                                        clientPointsNeeded.erase(i);
                                    }
                                }
                            } else if (clientCommandState[i] == 1) {
                                // Collecting points for Newgraph command
                                clientPendingLines[i].push_back(line);

                                if (clientPendingLines[i].size() >= clientPointsNeeded[i]) {
                                    // We have all points, process the Newgraph command
                                    calculator.commandNewGraph(clientPointsNeeded[i], clientPendingLines[i]);

                                    // Send confirmation
                                    std::string response = "Graph created with " +
                                                          std::to_string(clientPointsNeeded[i]) +
                                                          " points.\n";
                                    send(i, response.c_str(), response.length(), 0);

                                    // Reset client state
                                    clientCommandState[i] = 0;
                                    clientPointsNeeded[i] = 0;
                                    clientPendingLines[i].clear();
                                } else {
                                    // Acknowledge point receipt
                                    int remaining = clientPointsNeeded[i] - clientPendingLines[i].size();
                                    std::string response = "Point received. " +
                                                          std::to_string(remaining) +
                                                          " more point(s) needed.\n";
                                    send(i, response.c_str(), response.length(), 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}