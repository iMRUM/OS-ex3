#ifndef SERVER_HPP
#define SERVER_HPP
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
#define PORT "9034"   // port we're listening on

// Function to get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


class Server {
protected:
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

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    void init() {
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
    }
    virtual int run();
    virtual void stop();
public:
    void start() {
        init();
        if(!run()) {
            stop();
        }
    }
};



#endif //SERVER_HPP
