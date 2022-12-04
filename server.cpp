// Compile with
// g++ server.cpp crypt.cpp -o Server -lcrypto

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <list>
#include <algorithm>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include "crypt.h"

using namespace std;

#define PASSWORD "password1"

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
    list<string> admins;
	map<int, string> clientKeys;

	unsigned char line[5000];
	unsigned char key[32];
    unsigned char iv[16];
    unsigned char ciphertext[5000];
    unsigned char decryptedtext[5000];
    int decryptedtext_len, ciphertext_len;
    EVP_PKEY *privkey;
    FILE* privf = fopen("RSApriv.pem","rb");
    privkey = PEM_read_PrivateKey(privf,NULL,NULL,NULL);
    unsigned char encrypted_key[256];
	unsigned char decrypted_key[32];
    OpenSSL_add_all_algorithms();

    while(1) {
        fd_set tmpset=sockets;
        int r = select(FD_SETSIZE, &tmpset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &tmpset)) {
            socklen_t len = sizeof(struct sockaddr_in);
            int clientsocket = accept(sockfd, 
                    (struct sockaddr*) &clientaddr, &len);
            FD_SET(clientsocket,&sockets);


            // Receive encrypted key from client
            int encryptedkey_len = recv(clientsocket, encrypted_key, 256, 0);
            // Receive iv and encrypted username from client
            ciphertext_len = recv(clientsocket, line, 5000, 0) - 16;
			memcpy(&iv, line, 16);
			memcpy(&ciphertext, line+16, ciphertext_len);
            
			// Decrypt client's key
            int decryptedkey_len = rsa_decrypt(encrypted_key, encryptedkey_len, privkey, decrypted_key);
			// Decrypt username
            decryptedtext_len = decrypt(ciphertext, ciphertext_len, decrypted_key, iv, decryptedtext);
            decryptedtext[decryptedtext_len] = '\0';

            string clientName = string((char *)decryptedtext);
			clientKeys.insert(make_pair(clientsocket, string((char *)decrypted_key)));
            clients.insert(make_pair(clientName, clientsocket));
            cout << clientName << " connected to the server on socket " << clientsocket << endl;
            cout << clients.size() << " client(s) connected to the server: [";
            bool firstClient = true;
            for(const auto &cName : clients) {
                if(firstClient) {
                    firstClient = false;
                    cout << cName.first;
                }
                else 
                    cout << ", " << cName.first;
            }
            cout << "]" << endl;

			memset(line, 0, 5000);

			memcpy(&line, "SERVER", 10);
			memcpy(&line[10], "Welcome to the server", 1000);
			ciphertext_len = encrypt(line, 1010, decrypted_key, iv, ciphertext);
			memcpy(&line, iv, 16);
			memcpy(&line[16], ciphertext, ciphertext_len);
			send(clientsocket, line, 16+ciphertext_len, 0);
        }

        for(int i = 0; i < FD_SETSIZE; ++i) {
            if(FD_ISSET(i, &tmpset) && i != sockfd) {
                int n = recv(i, line, 5000, 0);

				memcpy(&iv, line, 16);
				// convert key from string back to unsigned char array
				memcpy(&key, clientKeys[i].c_str(), 32);

				// Decrypt message
				decryptedtext_len = decrypt(line+16, n-16, key, iv, decryptedtext);
				decryptedtext[decryptedtext_len] = '\0';
				memcpy(&line, decryptedtext, 5000); 

                unsigned char msg[4900];
                char user[10];
                int command = 0;
                // convert char to int
                command = line[0] - '0';
                memcpy(&user, line+1, 10);


                // Broadcast message
                if(command == 1) {
                    // Put the sender in the msg
                    memcpy(&msg, line+1, 10);
                    memcpy(&msg[10], line+11, 4900);
                    cout << "<" << user << " -> All> " << msg+10 << endl;

                    // Send msg to all client connected to the server 
                    for(const auto &client : clients) {
						// convert key from string back to unsigned char array
						memcpy(&key, clientKeys[client.second].c_str(), 32);

						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
                        send(client.second, line, 16+ciphertext_len, 0);
                    }

                }
                // Send private message
                else if(command == 2) {
                    char dmUser[10];
                    memcpy(&dmUser, line+11, 10);
                    if(clients.count(string(dmUser)) == 0) {
						memcpy(&msg, "SERVER", 10);
						memcpy(&msg[10], "User does not exist", 4890);
						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
                        send(clients[string(user)], line, 16+ciphertext_len, 0);
                    }
                    else {
						// Put the sender in the msg
						memcpy(&msg, line+1, 10);
						memcpy(&msg[10], line+21, 4890);
                        cout << "<" << user << " -> " << dmUser << "> " << msg+10 << endl;
						// convert key from string back to unsigned char array
						memcpy(&key, clientKeys[clients[string(dmUser)]].c_str(), 32);
						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
                        send(clients[string(dmUser)], line, 16+ciphertext_len, 0);
                    }

                }
                // get list of clients
                else if(command == 3) {
                    cout << "Sending list of clients to " << user << endl;
                    // Put the sender in the msg
                    memcpy(&msg, "SERVER", 10);
                    string clientList = "";
                    for(const auto &client : clients) {
                        clientList += client.first + " ";
                    }
                    memcpy(&msg[10], "Clients on the server: ", 23);
                    memcpy(&msg[33], clientList.c_str(), 4867);
					ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
					memcpy(&line, iv, 16);
					memcpy(&line[16], ciphertext, ciphertext_len);
                    send(i, line, 16+ciphertext_len, 0);
                }
                // Become admin
                else if(command == 4){
					memcpy(&msg, "SERVER", 10);
                    memcpy(&msg[10], line+11, 4900);
                    if((strcmp((char *)msg+10, PASSWORD)) == 0){
                        memcpy(&msg[10], "GRANTED admin permissions", 4900);
                        admins.push_back(string(user));
                        cout << user << " is now an admin" << endl;
                    }
                    else
                        memcpy(&msg[10], "DENIED", 4900);

					ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
					memcpy(&line, iv, 16);
					memcpy(&line[16], ciphertext, ciphertext_len);

                    send(i, line, 16+ciphertext_len, 0);
                }
                // Make another user admin
                else if(command == 6) {
                    char otherUser[10];
                    memcpy(&msg, "SERVER", 10);
                    memcpy(&otherUser, line+11, 10);
                    // if client using the command is not an admin
                    if(!(find(admins.begin(), admins.end(), string(user)) != admins.end())) {
                        memcpy(&msg[10], "You need to be an admin to use this command.", 100);
						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
						send(i, line, 16+ciphertext_len, 0);
                        continue;
                    }
                    // If other client is on the server and not already an admin
                    if(clients.count(string(otherUser)) > 0 
                        && !(find(admins.begin(), admins.end(), string(otherUser)) != admins.end())) {
                        admins.push_back(string(otherUser));
                        memcpy(&msg[10], "You have been given admin permissions.", 100);

						// convert key from string back to unsigned char array
						memcpy(&key, clientKeys[clients[string(otherUser)]].c_str(), 32);
						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
						send(clients[string(otherUser)], line, 16+ciphertext_len, 0);
                    }
                    else {
                        memcpy(&msg[10], "That user is not on the server or is already an admin.", 100);
						ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
						memcpy(&line, iv, 16);
						memcpy(&line[16], ciphertext, ciphertext_len);
						send(i, line, 16+ciphertext_len, 0);
                    }
                }
                // Close client's connection
                else if(command == 5 || command == 9) {
                    string rmUser = string(user);
                    // Kick user
                    if(command == 5) {
                        char kickUser[10];
                        memcpy(&msg, "SERVER", 10);
                        // if client is an admin
                        if(find(admins.begin(), admins.end(), rmUser) != admins.end()) {
                            memcpy(&kickUser, line+11, 10);
                            rmUser = string(kickUser);
                            // if client being kicked does not exist
                            if(clients.count(rmUser) == 0) {
                                cout << "User does not exist" << endl;
                                memcpy(&msg[10], "User does not exist.", 50);
								ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
								memcpy(&line, iv, 16);
								memcpy(&line[16], ciphertext, ciphertext_len);
								send(i, line, 16+ciphertext_len, 0);
                                continue;
                            }
							cout << user << " kicked " << rmUser << " from the server." << endl;
                            memcpy(&msg[10], "You have been kicked from the server.", 110);
							// convert key from string back to unsigned char array
							memcpy(&key, clientKeys[clients[rmUser]].c_str(), 32);
							ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
							memcpy(&line, iv, 16);
							memcpy(&line[16], ciphertext, ciphertext_len);
							send(clients[rmUser], line, 16+ciphertext_len, 0);
                        }
                        // Client is not an admin
                        else {
                            memcpy(&msg[10], "You do not have permission to kick users", 100);
							ciphertext_len = encrypt(msg, 4900, key, iv, ciphertext);
							memcpy(&line, iv, 16);
							memcpy(&line[16], ciphertext, ciphertext_len);
							send(clients[user], line, 16+ciphertext_len, 0);
                            continue;
                        }
                    }

                    // Close client's socket
                    FD_CLR(clients[rmUser],&sockets);
                    close(clients[rmUser]);
                    cout << "Closed socket " << clients[rmUser] << ", for client: " << rmUser << endl;
                    // Remove user from list of clients connected
					clientKeys.erase(clients[rmUser]);
                    clients.erase(rmUser);
					// if client was an admin, remove them from admin list
					if(find(admins.begin(), admins.end(), rmUser) != admins.end())
						admins.remove(rmUser);

                    cout << clients.size() << " client(s) still on the server: [";
                    bool firstClient = true;
                    for(const auto &cName : clients) {
                        if(firstClient) {
                            firstClient = false;
                            cout << cName.first;
                        }
                        else 
                            cout << ", " << cName.first;
                    }
                    cout << "]" << endl;
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
