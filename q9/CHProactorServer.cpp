#include "CHProactorServer.hpp"
#include "../Proactor/include/Proactor.hpp"
int listener;
int isRunning = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void init() {
    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    struct addrinfo hints, *ai, *p;

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
}

int run() {
    isRunning = 1;
    pthread_t accept_thread = startProactor(&listener, proactorFunc(handleAcceptClient));
    pthread_detach(accept_thread);
    std::cout << "accepting-thread, Address: " << &accept_thread << " started.\n" << std::endl;
    return 1;
}

void stop() {
    isRunning = 0;
    close(listener);
    std::cout << "Server stopped.\n";
}

void start() {
    init();
    run();
}

void handleRequest(void* arg) {
    int clientfd = *(int*)arg;
    char buf[256]; // buffer for client data
    int nbytes;
    while (isRunning) {
        if ((nbytes = recv(clientfd, buf, sizeof buf - 1, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
                // connection closed
                printf("Socket %d hung up\n", clientfd);
            } else {
                perror("recv");
            }
            break;
        }
        buf[nbytes] = '\0';
        pthread_mutex_lock(&mtx);
        handleCommand(clientfd, buf);
        pthread_mutex_unlock(&mtx);
    }
    close(clientfd); // bye!

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


void handleAcceptClient(void* arg) {
    int fd_listener = *(int*)arg;
    std::cout << "Accepted connection THREAD, listening on socket " << fd_listener << std::endl;
    int newfd;
    addrlen = sizeof(remoteaddr);
    while (isRunning) {
        if ((newfd = accept(fd_listener, (struct sockaddr *) &remoteaddr, &addrlen)) < 0) {
            perror("accept");
            continue;
        }
        pthread_t client_thread = startProactor(&newfd, proactorFunc(handleRequest));
        pthread_detach(client_thread);
        std::cout << "client-thread, address: " << &client_thread << " started with socket" << newfd << "\n" <<
                std::endl;
    }
}

int main() {
    std::cout << "Starting Convex Hull Proactor Server on port " << PORT << std::endl;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try {
        start();
        while (isRunning) {
            sleep(1);
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
