#include "../utils/CHServer.hpp"
// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int CHServer::run() {
    std::cout << "Convex Hull server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;
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

void CHServer::stop() {
    Server::stop();
}