#include "validation.h"
#include <iostream>
#include <string.h>
#include <cwctype> // for validating the username
#include <regex>
#include <termios.h>

int handle_command(unsigned char (&line)[5000], char command, unsigned char user[10]) {

    memcpy(&line[0], &command, 1);
    memcpy(&line[1], user, 10);
    if(command == 'q') {
        line[0] = '9';
        std::cout << "Disconnecting from the server" << std::endl;
        return 9;
    }
    std::string msg;
    unsigned char otherUser[10];
    // Broadcast message
    if(command == '1') {
        std::cout << "Enter a message: ";
        getline(std::cin >> std::ws, msg);
        memcpy(&line[11], msg.c_str(), 4900);
    }
    // Private message
    else if (command == '2') {
        std::cout << "User to DM: ";
        std::cin >> otherUser;
        memcpy(&line[11], otherUser, 10);
        std::cout << "Enter a message: ";
        getline(std::cin >> std::ws, msg);
        memcpy(&line[21], msg.c_str(), 4900);
    }
    // List of clients
    else if (command == '3') {
        return 0;
    }
    // Become admin
    else if (command == '4') {
        std::cout << "Enter admin password: ";

        struct termios t;
        tcgetattr(fileno(stdin), &t);
        t.c_lflag &= ~ECHO;
        tcsetattr(fileno(stdin), 0, &t);

        char password[4900];
        std::cin >> password;

        t.c_lflag |= ECHO;
        tcsetattr(fileno(stdin), 0, &t);

        memcpy(&line[11], password, 4900);
    }
    // Kick user
    else if (command == '5') {
        std::cout << "User to kick: ";
        std::cin >> otherUser;
        memcpy(&line[11], otherUser, 10);
    }
    // Give another client admin
    else if (command == '6') {
        char newUsername[10];
        std::cout << "Enter a user to give admin permissions: ";
        std::cin >> newUsername;
        memcpy(&line[11], newUsername, 10);
    }
    else {
        std::cout << "Invalid command" << std::endl;
        return 99;
    }

    return 0;
}


bool valid_username(std::string username) {

    if (username.length() < 4 || username.length() > 15){ return false; }

    std::regex valid_name("^[a-zA-Z0-9-_]{4,15}");

    if (std::regex_match(username, valid_name)){ return true; }

    return false;
}
