//
// Created by imry on 3/11/25.
//

#include "../include/Reactor.hpp"

#include <iostream>


/*
* struct reactor {
    fd_set fds;       / Master set of file descriptors /
    int max_fd;              / Highest file descriptor value /
    int running;             / Flag to control reactor loop /
    reactorFunc r_funcs[MAX_FDS]; / Array of callback functions /
};
 */
void * startReactor() {
    reactor_t* reactor = (reactor_t*)malloc(sizeof(reactor_t));
    if (reactor == NULL) {
        perror("Failed to allocate memory for reactor");
        return NULL;
    }

    // Initialize the reactor structure
    FD_ZERO(&reactor->fds);
    reactor->max_fd = -1;
    reactor->running = 1;
    memset(reactor->r_funcs, 0, sizeof(reactor->r_funcs));

    return (void*)reactor;
}

int addFdToReactor(void *reactor, int fd, reactorFunc func) {
    reactor_t* r = (reactor_t*)reactor;

    if (r == NULL || fd < 0 || fd >= MAX_FDS || func == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Add the file descriptor to the set
    FD_SET(fd, &r->fds);

    // Update max_fd if necessary
    if (fd > r->max_fd) {
        r->max_fd = fd;
    }

    // Store the callback function
    r->r_funcs[fd] = func;

    return 0;
}

int removeFdFromReactor(void *reactor, int fd) {
    reactor_t* r = (reactor_t*)reactor;

    if (r == NULL || fd < 0 || fd >= MAX_FDS) {
        errno = EINVAL;
        return -1;
    }

    // Clear the file descriptor from the set
    FD_CLR(fd, &r->fds);

    // Clear the callback
    r->r_funcs[fd] = NULL;
    // Recalculate max_fd if necessary
    if (fd == r->max_fd) {
        // Start from the previous max_fd and search downward
        r->max_fd = -1;
        for (int i = fd - 1; i >= 0; i--) {
            if (FD_ISSET(i, &r->fds)) {// Found the new highest fd
                r->max_fd = i;
                break;
            }
        }
    }
    std::cout<<"removed client from socket "<<fd<<". max socket is "<<r->max_fd<<"\n";
    return r->max_fd;
}

int runReactor(void *reactor) {
    reactor_t* r = (reactor_t*)reactor;

    if (r == NULL) {
        errno = EINVAL;
        return -1;
    }

    while (r->running) {
        fd_set read_fds = r->fds;  // to preserve the master set

        // Wait for activity on one of the sockets
        if (select(r->max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("runReactor: select");
            return -1;
        }

        // Check all sockets with activity
        for (int i = 0; i <= r->max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (r->r_funcs[i] != NULL) {
                    r->r_funcs[i](i);  // Call the callback function
                }
            }
        }
    }

    return 0;
}

int stopReactor(void *reactor) {
    reactor_t* r = (reactor_t*)reactor;
    if (r == NULL) {
        errno = EINVAL;
        return -1;
    }
    r->running = 0;
    FD_ZERO(&r->fds);
    memset(r->r_funcs, 0, sizeof(r->r_funcs));
    free(r);
    return 0;
}

