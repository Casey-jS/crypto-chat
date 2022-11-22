#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// NOTE ** use -lpthread arg when compiling

// arg/param must be void pointer
void* handleClient(void* arg){
    int clientSocket = *(int*)arg; // cast as int pointer and dereference to get int
    char line[5000];
    recv(clientSocket, line, 5000, 0);
    close(clientSocket);
}

int main(int argc, char** argv){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr, clientaddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9876);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(sockfd, 10);

    while(1){
        socklen_t len = sizeof(clientaddr);
        int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        pthread_t child;
        pthread_create(&child, NULL, handleClient, &clientsocket); // fourth param: what is passed into third param
        pthread_detach(child); // let the thread do its thing, we don't care about it anymore
    }
}