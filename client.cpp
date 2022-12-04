// Compile with
// g++ client.cpp validation.cpp crypt.cpp -o Client -lpthread -lcrypto

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include "validation.h"
#include <sys/types.h>
#include <signal.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include "crypt.h"

using namespace std;

struct threadArgs {
	int sockfd;
	unsigned char key[32];
};

// Use thread to receive from server
void* receiveFromServer(void* arg) {
	struct threadArgs *arguments = (struct threadArgs*)arg; 
	ssize_t len_recv = 0;
	unsigned char content[5000];
	unsigned char ciphertext[5000];
	unsigned char decryptedtext[5000];
	unsigned char iv[16];

	while(1){
		memset(content, 0, 5000);
		len_recv = recv(arguments->sockfd, content, 5000, 0); 

		// First part of the message is who it is from
		if(len_recv > 0) {
			memcpy(&iv, content, 16);
			memcpy(&ciphertext, content+16, len_recv-16);

			int decryptedtext_len = decrypt(ciphertext, len_recv-16, arguments->key, iv, decryptedtext);
			decryptedtext[decryptedtext_len] = '\0';
			char fromUser[10];
			memcpy(&fromUser, decryptedtext, 10);
			char msg[4990];
			memcpy(&msg, decryptedtext+10, 4990);

			cout << endl << "<" << fromUser <<"> " << msg << endl;

			if(strcmp(fromUser, "SERVER") == 0 &&
				strcmp(msg,"You have been kicked from the server.") == 0){
			  kill(0, SIGINT);
			}
		}
	}
}

void printCommands() {
    cout << endl << "========================================" << endl;
    cout << "Commands to use: " << endl;
    cout << "1) Broadcast message" << endl;
    cout << "2) Private message" << endl;
    cout << "3) List of clients on the server" << endl;
    cout << "4) Become an admin" << endl;
    cout << "5) Kick a user from the server (for admins only)" << endl;
    cout << "6) Make another user an admin (for admins only)" << endl;
    cout << "q) Disconnect" << endl;
    cout << "========================================" << endl << endl;
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

	unsigned char line[5000];
	unsigned char key[32];
    unsigned char iv[16];
    unsigned char ciphertext[5000];
    unsigned char decryptedtext[5000];
    int decryptedtext_len, ciphertext_len;

    // sets up library
    OpenSSL_add_all_algorithms();
    // Generate a random key
    RAND_bytes(key,32); 
    RAND_bytes(iv,16); 

    // Read public key from file
    EVP_PKEY *pubkey;
    FILE* pubf = fopen("RSApub.pem","rb");
    pubkey = PEM_read_PUBKEY(pubf,NULL,NULL,NULL);

    unsigned char user[10];
    cout << "Enter a username: ";
    cin >> user;

    // Encrypt client key using public key
    unsigned char encrypted_key[256];
    int encryptedkey_len = rsa_encrypt(key, 32, pubkey, encrypted_key);
    // Send encrypted key 
    send(sockfd, encrypted_key, encryptedkey_len, 0);

	// Send iv and encrypted username
	ciphertext_len = encrypt(user, 10, key, iv, ciphertext);
	memcpy(&line, iv, 16);
	memcpy(&line[16], ciphertext, ciphertext_len);
    send(sockfd, line, 16+ciphertext_len, 0);
	// clear line
    memset(line, 0, 5000);
    
    printCommands();

    // Create a thread for receiving messages
	struct threadArgs arguments;
	arguments.sockfd = sockfd;
	memcpy(&arguments.key, key, 32);
    pthread_t child_recv;
    pthread_create(&child_recv, NULL, receiveFromServer, &arguments);
    pthread_detach(child_recv);

    while(1) {
		// Generate new iv for each message sent
		RAND_bytes(iv,16); 
        char command = '0';
        cout << "Enter a command: ";
        cin >> command;
        int validCommand = handle_command(line, command, user);
        if(validCommand == 9) {
			ciphertext_len = encrypt(line, strlen((char *)line), key, iv, ciphertext);
			memcpy(&line, iv, 16);
			memcpy(&line[16], ciphertext, ciphertext_len);
            send(sockfd, line, 16+ciphertext_len, 0);
            break;
        }
        if(validCommand == 99) {
            continue;
        }

		ciphertext_len = encrypt(line, 4900, key, iv, ciphertext);
		memcpy(&line, iv, 16);
		memcpy(&line[16], ciphertext, ciphertext_len);
		send(sockfd, line, 16+ciphertext_len, 0);
    }
    close(sockfd);

    return 0;
}
