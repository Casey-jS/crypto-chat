// Compile with
// g++ client.cpp validation.cpp -o Client -lpthread

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include "validation.h"

using namespace std;

// Use thread to receive from server
void* receiveFromServer(void* arg) {
    int sock = *(int*) arg;
    ssize_t len_recv = 0;
    char content[5000];

    len_recv = recv(sock, content, 5000, 0); 
    cout << endl << "received from server: " << content << endl;
}

void printCommands() {
    cout << "Commands to use: " << endl;
    cout << "1) Broadcast message" << endl;
    cout << "2) Private message" << endl;
    cout << "3) List of clients on the server" << endl;
    cout << "4) Become an admin" << endl;
    cout << "5) Kick a user from the server (for admins only)" << endl;
    cout << "q) Disconnect" << endl;
}

int connectToServer() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int port_num;
    char ip_addr[20];
    cout << "Enter an IP address: ";
    fgets(ip_addr,20,stdin);
    cout << "Enter a port number: ";
    cin >> port_num;

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port_num);
    serveraddr.sin_addr.s_addr = inet_addr(ip_addr);

    int n = connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    if(n<0) {
        printf("There was a problem connecting\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}


int main(int argc, char **argv) {
    int sockfd = connectToServer();
    if(sockfd == -1)
        return 1;

    char user[10];
    cout << "Enter a username: ";
    cin >> user;
    
    send(sockfd, user, 10, 0);
    
    printCommands();

    while(1) {

        pthread_t child_recv;
        pthread_create(&child_recv, NULL, receiveFromServer, &sockfd);
        pthread_detach(child_recv);

        char command = '0';
        char line[5000];
        cout << "Enter a command: ";
        cin >> command;
        int validCommand = handle_command(line, command, user);
        if(validCommand == 9) {
            send(sockfd, line, 1, 0);
            break;
        }
        if(validCommand == 99) {
            continue;
        }

        send(sockfd, line, 5000, 0);
    }
    close(sockfd);

    return 0;
}
