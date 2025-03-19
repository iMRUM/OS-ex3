#include "../include/Proactor.hpp"
pthread_t startProactor(int* sockfd, proactorFunc threadFunc)
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, threadFunc, sockfd) != 0) {
        perror("pthread_create");
        close(*sockfd);
    }
    return tid;
}

int stopProactor(pthread_t tid)
{
    return pthread_cancel(tid);
}
