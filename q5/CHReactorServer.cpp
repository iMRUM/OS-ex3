#include "CHReactorServer.hpp"
/*
 *When client is accepted with unique fd, its corresponding reactorFuncd[fd] is set to handleCommand.
 *then, the relevant methods will correspond.
 *
 */
// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void CHReactorServer::handleRequest(int fd) {
    std::cout << "CHReactorServer::handleRequest "<< fd << std::endl;
}

void CHReactorServer::handleCommand(int clientfd) {
}

void CHReactorServer::handleAddPoint(int clientfd) {
}

void CHReactorServer::handleAcceptClient(int fd) {
}

void CHReactorServer::init() {
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv, listener;
    struct addrinfo hints, *ai, *p;
    reactorInstance = static_cast<reactor_t *>(startReactor());
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
    addFdToReactor(reactorInstance, listener, handleAcceptClient);
}

void CHReactorServer::start() {
}

int CHReactorServer::run() {
}

void CHReactorServer::stop() {
    std::cout << "CHReactorServer::stop " << std::endl;
}
