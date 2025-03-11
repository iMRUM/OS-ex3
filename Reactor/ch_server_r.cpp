/*
** ch_server.cpp -- Convex Hull network server
** Based on selectserver.c by Beej, integrated with ConvexHullCalculator and Reactor pattern
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
#include "Reactor.h"

#define PORT "9034"   // port we're listening on

// Global variables for client state management
std::map<int, std::vector<std::string>> clientPendingLines;
std::map<int, int> clientCommandState; // 0 = normal, 1 = expecting points for Newgraph
std::map<int, int> clientPointsNeeded; // For Newgraph command
ConvexHullCalculator calculator; // Single instance for all clients
void* reactorInstance; // Global reactor instance

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Forward declarations for our callback functions
void handleNewConnection(int listener);
void handleClientData(int clientfd);

// Process a single line from a client
void processClientLine(int clientfd, const std::string& line) {
    if (clientCommandState[clientfd] == 0) {
        // Normal command processing mode
        if (line.substr(0, 8) == "Newgraph") {
            // Extract the number of points
            std::istringstream iss(line);
            std::string cmd;
            int n;
            iss >> cmd >> n;

            if (n > 0) {
                clientCommandState[clientfd] = 1;  // Switch to point collection mode
                clientPointsNeeded[clientfd] = n;  // Set the number of points to collect
                clientPendingLines[clientfd].clear();  // Clear any existing points

                // Send acknowledgment
                std::string response = "Please enter " + std::to_string(n) + " points, one per line:\n";
                send(clientfd, response.c_str(), response.length(), 0);
            } else {
                std::string response = "Invalid number of points.\n";
                send(clientfd, response.c_str(), response.length(), 0);
            }
        } else {
            // Process regular command
            std::vector<std::string> empty;
            std::string result = calculator.processCommand(line, empty);
            result += "\n";  // Add newline for client display

            send(clientfd, result.c_str(), result.length(), 0);

            // Check if this was an exit command
            if (result == "exit\n") {
                printf("Client %d requested exit\n", clientfd);

                // Remove from reactor
                removeFdFromReactor(reactorInstance, clientfd);

                // Clean up client data
                clientPendingLines.erase(clientfd);
                clientCommandState.erase(clientfd);
                clientPointsNeeded.erase(clientfd);

                close(clientfd);
            }
        }
    } else if (clientCommandState[clientfd] == 1) {
        // Collecting points for Newgraph command
        clientPendingLines[clientfd].push_back(line);

        if (clientPendingLines[clientfd].size() >= clientPointsNeeded[clientfd]) {
            // We have all points, process the Newgraph command
            calculator.commandNewGraph(clientPointsNeeded[clientfd], clientPendingLines[clientfd]);

            // Send confirmation
            std::string response = "Graph created with " +
                                  std::to_string(clientPointsNeeded[clientfd]) +
                                  " points.\n";
            send(clientfd, response.c_str(), response.length(), 0);

            // Reset client state
            clientCommandState[clientfd] = 0;
            clientPointsNeeded[clientfd] = 0;
            clientPendingLines[clientfd].clear();
        } else {
            // Acknowledge point receipt
            int remaining = clientPointsNeeded[clientfd] - clientPendingLines[clientfd].size();
            std::string response = "Point received. " +
                                  std::to_string(remaining) +
                                  " more point(s) needed.\n";
            send(clientfd, response.c_str(), response.length(), 0);
        }
    }
}

// Callback for handling new connections
void handleNewConnection(int listener) {
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;
    int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1) {
        perror("accept");
        return;
    }

    // Add new client to reactor
    addFdToReactor(reactorInstance, newfd, handleClientData);

    // Initialize client state
    clientCommandState[newfd] = 0;
    clientPointsNeeded[newfd] = 0;

    char remoteIP[INET6_ADDRSTRLEN];
    printf("New connection from %s on socket %d\n",
        inet_ntop(remoteaddr.ss_family,
            get_in_addr((struct sockaddr*)&remoteaddr),
            remoteIP, INET6_ADDRSTRLEN),
        newfd);

    // Send welcome message
    std::string welcome = "Welcome to the Convex Hull Server.\nType 'help' for available commands.\n";
    send(newfd, welcome.c_str(), welcome.length(), 0);
}

// Callback for handling client data
void handleClientData(int clientfd) {
    char buf[256];
    int nbytes = recv(clientfd, buf, sizeof buf - 1, 0);

    if (nbytes <= 0) {
        // Connection closed or error
        if (nbytes == 0) {
            printf("Socket %d hung up\n", clientfd);
        } else {
            perror("recv");
        }

        // Clean up
        removeFdFromReactor(reactorInstance, clientfd);
        clientPendingLines.erase(clientfd);
        clientCommandState.erase(clientfd);
        clientPointsNeeded.erase(clientfd);
        close(clientfd);
        return;
    }

    // We got data from the client
    buf[nbytes] = '\0';

    // Process data line by line
    std::string data(buf);
    std::istringstream stream(data);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        processClientLine(clientfd, line);
    }
}

int main(void) {
    int listener;     // listening socket descriptor
    int yes = 1;      // for setsockopt() SO_REUSEADDR
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get a listening socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Avoid "address already in use" error
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    // Listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // Create and initialize the reactor
    reactorInstance = startReactor();
    if (reactorInstance == NULL) {
        fprintf(stderr, "Failed to create reactor\n");
        exit(4);
    }

    // Add listener socket to reactor
    if (addFdToReactor(reactorInstance, listener, handleNewConnection) < 0) {
        fprintf(stderr, "Failed to add listener to reactor\n");
        stopReactor(reactorInstance);
        exit(5);
    }

    std::cout << "Convex Hull server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    // Run the reactor (this will block until stopReactor is called)
    int result = runReactor(reactorInstance);

    // Clean up
    close(listener);
    stopReactor(reactorInstance);

    return result == 0 ? 0 : 1;
}
