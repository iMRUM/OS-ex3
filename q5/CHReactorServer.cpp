#include "CHReactorServer.hpp"
/*
 *When client is accepted with unique fd, its corresponding reactorFuncd[fd] is set to handleCommand.
 *then, the relevant methods will correspond.
 *
 */
// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


void handleRequest(int clientfd) {
    nbytes = recv(clientfd,buf,sizeof(buf),0);
    if (nbytes<=0) {
        if (nbytes == 0) {
            std::cout << "selectserver: socket " << clientfd << " hung up" << std::endl;
        } else {
            perror("recv");
        }
        close(clientfd);
        removeFdFromReactor(reactor_p, clientfd);
        return;
    }
    buf[nbytes]='\0';
    handleCommand(clientfd, buf);
}

void handleCommand(int clientfd, const std::string &input_command) {
    std::string command;
    std::istringstream iss(input_command);
    std::string response;
    iss >> command;
    if (isWaitingForPoints) {
        if (command.find(',') != std::string::npos){}
        else {
            response = "Invalid point format. Insert point as x, y."
        }
    }else {
        if (command == "Newgraph") {//input of n lines!
            //        std::vector<std::string> points_str_vector;
            int n;
            if (iss >> n) {
                calculator.commandNewGraph(n);
                isWaitingForPoints = n;
                response = "Insert "+ n + " points as x, y. line by line.\n";
                send(clientfd, response.c_str(), response.size(), 0);
                for (int i = 0; i<n; i++) {
                    handleRequest(clientfd);
                }
            }else {
                response = "Invalid Newgraph command. Usage: Newgraph n";
                send(clientfd, response.c_str(), response.length(), 0);
            }
        }
        else if (command == "CH") {}
        else if (command == "Newpoint") {}
        else if (command == "Removepoint") {}
    }

}

void handleCommandCh(int clientfd) {
}

void handleCommandAddPoint(int clientfd) {
}

void handleCommandRmPoint(int clientfd) {
}

void handleAcceptClient(int listener) {
    int clientfd = accept(listener, NULL, NULL);
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
    addFdToReactor(reactor_p, listener, handleAcceptClient);
}

void start() {
    init();
    if(run()) {
        stop();
        return;
    }
    exit(2);
}

int run() {
    if (!runReactor(reactor_p)) {
        return 1;
    }
    fprintf(stderr, "reactor failure: failed to runReactor\n");
    return 0;
}

void stop() {
    std::cout << "CHReactorServer::stop " << std::endl;
}
