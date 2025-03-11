/**
* @file reactor.h
 * @brief Reactor pattern implementation for managing file descriptors
 */

#ifndef REACTOR_H
#define REACTOR_H

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
/**
 * @brief Function type to be called when a file descriptor is ready
 */
typedef void (*reactorFunc)(int fd);

/**
 * @brief Reactor structure for managing file descriptors
 */
#define MAX_FDS 1024  /* Maximum number of file descriptors to manage */
#define NULL nullptr

struct reactor {
    fd_set fds;       /* Master set of file descriptors */
    int max_fd;              /* Highest file descriptor value */
    int running;             /* Flag to control reactor loop */
    reactorFunc r_funcs[MAX_FDS]; /* Array of callback functions */
    /* For poll implementation, we would use struct pollfd fds[MAX_FDS] instead */
};

typedef struct reactor reactor_t;

/**
 * @brief Creates and starts a new reactor
 *
 * @return pointer to the created reactor or NULL on failure
 */
void* startReactor();

/**
 * @brief Adds a file descriptor to the reactor for monitoring
 *
 * @param reactor pointer to the reactor
 * @param fd file descriptor to monitor
 * @param func callback function to execute when fd is ready
 * @return 0 on success, -1 on failure
 */
int addFdToReactor(void* reactor, int fd, reactorFunc func);

/**
 * @brief Removes a file descriptor from the reactor
 *
 * @param reactor pointer to the reactor
 * @param fd file descriptor to remove
 * @return updated max_fd on success, -1 when last fd was removed or failure occurred
 */
int removeFdFromReactor(void* reactor, int fd);

/**
 * @brief Stops the reactor and frees associated resources
 *
 * @param reactor pointer to the reactor
 * @return 0 on success, -1 on failure
 */
int stopReactor(void* reactor);

#endif /* REACTOR_H */