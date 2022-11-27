#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <map>

using namespace std;

int main(int argc, char **argv) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(sockfd, &sockets);

    int port_num;;
	printf("Enter a port number: ");
	scanf("%d", &port_num);

    struct sockaddr_in serveraddr, clientaddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port_num);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    listen(sockfd, 10);
    
    // client's username and socket
    map<string, int> clients;


    while(1) {
        fd_set tmpset=sockets;
        int r = select(FD_SETSIZE, &tmpset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &tmpset)) {
            socklen_t len = sizeof(struct sockaddr_in);
            int clientsocket = accept(sockfd, 
                    (struct sockaddr*) &clientaddr, &len);
            FD_SET(clientsocket,&sockets);
            char line[10];
            int m = recv(clientsocket, line, 10, 0);
            string clientName = string(line);
            clients.insert(make_pair(clientName, clientsocket));
            cout << "New client connected on socket " << sockfd << endl;
            cout << "Number of clients connected to the server: " << clients.size() << endl;
            cout << "List of clients: ";
            for(const auto &cName : clients)
                cout << cName.first << ", ";
            cout << endl;
        }

        for(int i = 0; i < FD_SETSIZE; ++i) {
            if(FD_ISSET(i, &tmpset) && i != sockfd) {
                char line[5000];
                int n = recv(i, line, 5000, 0);

                char msg[4900];
                char user[10];
                int command = 0;
                // convert char to int
                command = line[0] - '0';
                if(n > 1) {
                    memcpy(&user, line+1, 10);
                }
                // Broadcast message
                if(command == 1) {
                    memcpy(&msg, line+11, 4900);
                    cout << string(user) << " -> All > " << msg << endl;

                    // Send msg to all client connected to the server 
                    for(const auto &client : clients) {
                        send(client.second, msg, 5000, 0);
                    }

                }
                // Send private message
                else if(command == 2) {
                    char dmUser[10];
                    memcpy(&dmUser, line+11, 10);
                    memcpy(&msg, line+21, 4900);
                    if(clients.count(string(dmUser)) == 0) {
                        string dmError = "User does not exist";
                        send(clients[string(user)], dmError.c_str(), 5000, 0);
                    }
                    else {
                        cout << string(user) << " -> " << string(dmUser) << " > " << msg << endl;
                        send(clients[string(dmUser)], msg, 5000, 0);
                    }

                }
                // get list of clients
                else if(command == 3) {
                    cout << "Sending list of clients to " << string(user) << endl;
                    string clientList = "";
                    for(const auto &client : clients) {
                        clientList += client.first + " ";
                    }
                    send(i, clientList.c_str(), 5000, 0);

                }
                // Close client's connection
                else if(command == 5 || command == 9) {
                    string rmUser = string(user);
                    // Kick user
                    if(command == 5) {
                        char kickUser[10];
                        memcpy(&kickUser, line+11, 10);
                        rmUser = string(kickUser);
                        cout << "Kicked " << rmUser << " from server" << endl;
                    }
                    // Close client's socket
                    FD_CLR(clients[rmUser],&sockets);
                    close(clients[rmUser]);
                    cout << "Closed socket " << clients[rmUser] << endl;
                    // Remove user from list of clients connected
                    clients.erase(rmUser);
                }
                // No new message
                else
                    continue;

                // Set command to zero to prevent sending the same message
                line[0] = '0';
            }
        }
    }


    return 0;
}
