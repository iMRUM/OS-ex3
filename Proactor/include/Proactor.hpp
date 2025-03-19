#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>

typedef void* (*proactorFunc) (void* sockfd);
// starts new proactor and returns proactor thread id. 
pthread_t startProactor (int* sockfd, proactorFunc threadFunc);
// stops proactor by threadid 
int stopProactor(pthread_t tid);