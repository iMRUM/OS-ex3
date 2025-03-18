#include "../include/Proactor.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void proactor::start_proactor(int listener, proactorFunc client_handler) {
    eventLoopThread = thread([this, listener, client_handler]() {
        while (true) {
            struct sockaddr_storage remoteaddr;  // client address
            socklen_t addrlen;
            int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
            if (newfd == -1) {
                perror("accept");
                continue;
            }

            char remoteIP[INET6_ADDRSTRLEN];
            inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN);
          std::cout << "server: got new connection" << std::endl;

            thread t(client_handler, newfd, std::ref(this->mtx));
            t.detach();
        }
    });

    eventLoopThread.join();
}

void proactor::stop_proactor() {
    // kill the event loop thread
    eventLoopThread.~thread();
    mtx.unlock();
}

proactor::~proactor() {
    stop_proactor();
}