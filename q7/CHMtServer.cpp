#include "CHMtServer.hpp"

int listener;
int isRunning = 0;

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
    std::thread accept_thread(handleAcceptClient, listener);
    accept_thread.detach();
    std::cout << "accepting-thread, ID: " << accept_thread.get_id() << " started.\n" << std::endl;
}

void stop() {
}

void start() {
}

void handleRequest(int clientfd) {
    char buf[256]; // buffer for client data
    int nbytes;
    if ((nbytes = recv(i, buf, sizeof buf - 1, 0)) <= 0) {
        // got error or connection closed by client
        if (nbytes == 0) {
            // connection closed
            printf("Socket %d hung up\n", i);
        } else {
            perror("recv");
        }
        close(i); // bye!
        FD_CLR(i, &master); // remove from master set
    }else {
        buf[nbytes] = '\0';
        handleCommand(clientfd, buf);
    }
}

void handleCommand(int clientfd, const std::string &input_command) {
}

void handleAcceptClient(int fd_listener) {
    int newfd;
    while (isRunning) {
        if (newfd = accept(fd_listener, (struct sockaddr *) &remoteaddr, &addrlen) < 0) {
            perror("accept");
            continue;
        }
        std::thread client_thread(handleRequest, newfd);
        std::cout << "client-thread, ID: " << client_thread.get_id() << " started with socket" << newfd << "\n" <<
                std::endl;
    }
}
