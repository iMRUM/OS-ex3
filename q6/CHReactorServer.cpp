#include "CHReactorServer.hpp"
/*
 *When client is accepted with unique fd, its corresponding reactorFuncd[fd] is set to handleCommand.
 *then, the relevant methods will correspond.
 *
 */

void handleRequest(int clientfd) {
    char buf[256]; // buffer for client data
    int nbytes;
    nbytes = recv(clientfd, buf, sizeof(buf), 0);
    if (nbytes <= 0) {
        if (nbytes == 0) {
            std::cout << "selectserver: socket " << clientfd << " hung up" << std::endl;
        } else {
            perror("recv");
        }
        close(clientfd);
        removeFdFromReactor(reactor_p, clientfd);
        return;
    }
    buf[nbytes] = '\0';
    handleCommand(clientfd, buf);
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
    int clientfd = accept(fd_listener, nullptr, nullptr);
    addFdToReactor(reactor_p, clientfd, handleRequest);
}

void init() {
    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int i, j, rv, listener;
    struct addrinfo hints, *ai, *p;
    reactor_p = static_cast<reactor_t *>(startReactor());

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(nullptr, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != nullptr; p = p->ai_next) {
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
    if (p == nullptr) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }
    addFdToReactor(reactor_p, listener, handleAcceptClient);
}

void start() {
    init();
    if (!run()) {
        exit(2);
    }
}

int run() {
    if (!runReactor(reactor_p)) {
        return 1;
    }
    fprintf(stderr, "reactor failure: failed to runReactor\n");
    return 0;
}

void stop() {
    std::cout << "CHReactorServer::stop - shutting down server" << std::endl;
    if (reactor_p != nullptr) {
        stopReactor(reactor_p);
        reactor_p = nullptr;
    }

    // Reset the waiting points counter
    isWaitingForPoints = 0;

    std::cout << "Server shutdown complete" << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "Starting Convex Hull Reactor Server on port " << PORT << std::endl;

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
