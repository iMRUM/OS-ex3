#include "../utils/CHServer.hpp"
fd_set master; // master file descriptor list
fd_set read_fds; // temp file descriptor list for select()
int fdmax; // maximum file descriptor number
int listener; // listening socket descriptor



struct addrinfo hints, *ai, *p;

void handleRequest(int clientfd) {
    char buf[256]; // buffer for client data
    int nbytes;
    if ((nbytes = recv(clientfd, buf, sizeof buf - 1, 0)) <= 0) {
        // got error or connection closed by client
        if (nbytes == 0) {
            // connection closed
            printf("Socket %d hung up\n", clientfd);
        } else {
            perror("recv");
        }
        close(clientfd); // bye!
        FD_CLR(clientfd, &master); // remove from master set
    }else {
        buf[nbytes] = '\0';
        handleCommand(clientfd, buf);
    }

}

void handleCommand(int clientfd, const std::string &input_command) {
    std::string command;
    std::istringstream iss(input_command);
    std::string response;
    iss >> command;
    if (isWaitingForPoints) {
        if (input_command.find(',') != std::string::npos) {
            calculator.commandAddPoint(command);
            isWaitingForPoints--;
            response = "Point (" + command + ") was added.";
        } else {
            response = "Error. Insert point as x, y.";
        }
    } else {
        if (command == "Newgraph") {
            int n;
            if (iss >> n) {
                calculator.commandNewGraph(n);
                isWaitingForPoints = n;
                response = "Insert points as x, y. line by line.";
            } else {
                response = "Invalid Newgraph command. Usage: Newgraph n";
                send(clientfd, response.c_str(), response.length(), 0);
            }
        } else {
            response = calculator.processCommand(input_command);
        }
    }
    response += "\n";
    send(clientfd, response.c_str(), response.length(), 0);
}


void handleAcceptClient(int fd_listener) {
    int newfd; // newly accept()ed socket descriptor
    addrlen = sizeof remoteaddr;
    newfd = accept(fd_listener, (struct sockaddr *) &remoteaddr, &addrlen);
    if (newfd == -1) {
        perror("accept");
    } else {
        FD_SET(newfd, &master); // add to master set
        if (newfd > fdmax) {
            // keep track of the max
            fdmax = newfd;
        }
        printf("New connection from %s on socket %d\n",
               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *) &remoteaddr), remoteIP,
                         INET6_ADDRSTRLEN), newfd);
    }
}

void init() {
    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);
    int rv;
    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
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
}

int run() {
    int i;
    std::cout << "Convex Hull server started on port " << PORT << std::endl;
    std::cout << "Waiting for connections..." << std::endl;
    for (;;) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // we got one!!
                if (i == listener) {
                    handleAcceptClient(i);
                }else {
                    handleRequest(i);
                }
            }
        }
    }
    return 0;
}

void start() {
    init();
    if (!run()) {
        exit(2);
    }
}

void stop() {
    // Reset the waiting points counter
    isWaitingForPoints = 0;

    std::cout << "Server shutdown complete" << std::endl;
}


int main(int argc, char *argv[]) {
    std::cout << "Starting Convex Hull Server on port " << PORT << std::endl;

    // Register signal handlers for graceful shutdown
    signal(SIGINT, signalHandler); // Ctrl+C
    signal(SIGTERM, signalHandler); // Termination request

    try {
        // Start the server (this will call init() and run())
        start();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}