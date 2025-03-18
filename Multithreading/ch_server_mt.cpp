/*
** ch_server.cpp -- Threaded Convex Hull network server
** Modified from the original select-based server to use a thread per client
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
#include <thread>
#include <mutex>
#include "../utils/ConvexHullCalculator.hpp"

#define PORT "9034"   // port we're listening on

// Global shared calculator with mutex for protection
ConvexHullCalculator calculator;
std::mutex calculator_mutex;

// Structure to pass to client handler threads
struct ClientData {
    int socket;
    struct sockaddr_storage address;
};

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Client handler function executed by each client thread
void handle_client(ClientData* client_data) {
    int socket_fd = client_data->socket;
    char remoteIP[INET6_ADDRSTRLEN];

    // Print client connection information
    printf("Thread started for client on socket %d from %s\n",
        socket_fd,
        inet_ntop(client_data->address.ss_family,
            get_in_addr((struct sockaddr*)&client_data->address),
            remoteIP, INET6_ADDRSTRLEN));

    // Free the client_data structure as we've extracted what we need
    delete client_data;

    // Client state variables
    std::vector<std::string> pendingLines;
    int commandState = 0; // 0 = normal, 1 = expecting points for Newgraph
    int pointsNeeded = 0; // For Newgraph command

    // Send welcome message to the client
    std::string welcome = "Welcome to the Convex Hull Server.\nType 'help' for available commands.\n";
    send(socket_fd, welcome.c_str(), welcome.length(), 0);

    // Buffer for receiving data
    char buf[256];
    int nbytes;

    // Main client handling loop
    while(1) {
        // Receive data from client
        nbytes = recv(socket_fd, buf, sizeof(buf) - 1, 0);

        if (nbytes <= 0) {
            // Connection closed or error
            if (nbytes == 0) {
                printf("Socket %d hung up\n", socket_fd);
            } else {
                perror("recv");
            }
            break;
        }

        // Null-terminate the received data
        buf[nbytes] = '\0';

        // Process the received data line by line
        std::string data(buf);
        std::istringstream stream(data);
        std::string line;

        while (std::getline(stream, line)) {
            // Remove carriage return if present (handles Windows line endings)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (commandState == 0) {
                // Normal command processing mode
                if (line.substr(0, 8) == "Newgraph") {
                    // Extract the number of points
                    std::istringstream iss(line);
                    std::string cmd;
                    int n;
                    iss >> cmd >> n;

                    if (n > 0) {
                        commandState = 1;  // Switch to point collection mode
                        pointsNeeded = n;  // Set the number of points to collect
                        pendingLines.clear();  // Clear any existing points

                        // Send acknowledgment
                        std::string response = "Please enter " + std::to_string(n) + " points, one per line:\n";
                        send(socket_fd, response.c_str(), response.length(), 0);
                    } else {
                        std::string response = "Invalid number of points.\n";
                        send(socket_fd, response.c_str(), response.length(), 0);
                    }
                } else {
                    // Process regular command
                    std::vector<std::string> empty;
                    std::string result;

                    // Lock the calculator while processing the command
                    {
                        std::lock_guard<std::mutex> lock(calculator_mutex);
                        result = calculator.processCommand(line, empty);
                    }

                    result += "\n";  // Add newline for client display
                    send(socket_fd, result.c_str(), result.length(), 0);

                    // Check if this was an exit command
                    if (result == "exit\n") {
                        printf("Client %d requested exit\n", socket_fd);
                        close(socket_fd);
                        return;
                    }
                }
            } else if (commandState == 1) {
                // Collecting points for Newgraph command
                pendingLines.push_back(line);

                if (pendingLines.size() >= pointsNeeded) {
                    // We have all points, process the Newgraph command
                    {
                        std::lock_guard<std::mutex> lock(calculator_mutex);
                        calculator.commandNewGraph(pointsNeeded, pendingLines);
                    }

                    // Send confirmation
                    std::string response = "Graph created with " +
                                         std::to_string(pointsNeeded) +
                                         " points.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);

                    // Reset client state
                    commandState = 0;
                    pointsNeeded = 0;
                    pendingLines.clear();
                } else {
                    // Acknowledge point receipt
                    int remaining = pointsNeeded - pendingLines.size();
                    std::string response = "Point received. " +
                                         std::to_string(remaining) +
                                         " more point(s) needed.\n";
                    send(socket_fd, response.c_str(), response.length(), 0);
                }
            }
        }
    }

    // Clean up and exit thread
    close(socket_fd);
    printf("Thread ended for client on socket %d\n", socket_fd);
}

int main(void) {
    int listener;     // listening socket descriptor
    struct sockaddr_storage client_addr; // client address
    socklen_t addrlen;
    int yes = 1;      // for setsockopt() SO_REUSEADDR
    int rv;
    struct addrinfo hints, *ai, *p;
    std::vector<std::thread> client_threads;

    // Set up the address info structure
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Loop through all the results and bind to the first one we can
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Avoid "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;  // Successfully bound
    }

    // If we got here and p is NULL, it means we couldn't bind
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // All done with this structure

    // Start listening
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    std::cout << "Threaded Convex Hull server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    // Main accept() loop
    while(1) {
        addrlen = sizeof client_addr;
        int new_fd = accept(listener, (struct sockaddr *)&client_addr, &addrlen);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Create client data structure to pass to the thread
        ClientData* client_data = new ClientData;
        client_data->socket = new_fd;
        client_data->address = client_addr;

        // Create a new thread to handle this client
        std::thread client_thread(handle_client, client_data);
        client_thread.detach();  // Detach the thread so it can run independently
    }

    return 0;
}