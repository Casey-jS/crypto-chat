#include "validation.h"
#include <iostream>
#include <string.h>
#include <cwctype> // for validating the username
#include <regex>

bool is_admin = true;
int handle_command(char *line, char command, char user[10]) {

    memcpy(&line[0], &command, 1);
    memcpy(&line[1], user, 10);
    if(command == 'q') {
        line[0] = '9';
        std::cout << "Disconnecting from the server" << std::endl;
        return 9;
    }
    std::string msg;
    char otherUser[10];
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
    else if (command == '4') {
      std::cout << "Enter password: (5-16 characters containing one letter and one number)" << std::endl;
      char password[4900];
      std::cin >> password;
      memcpy(&line[11], password, 4900);

      return 4;
    }
    else if (command == '5') {
        if (!is_admin){
            std::cout << "You do not have admin rights." << std::endl;
            return 99;
        }
        // if the user IS an admin
        else {
            std::cout << "User to kick: ";
            std::cin >> otherUser;
            if(strcmp(user, otherUser) == 0) {
                return 9;
            }
            memcpy(&line[11], otherUser, 10);
        }
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


// must have 1 letter, 1 num, one special character
bool valid_password(std::string password) {

    if (password.length() < 5 || password.length() > 16){ return false; }

    std::regex valid_password("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{5,16}");

    if(std::regex_match(password, valid_password)){ return true; }

    return false;
}


bool become_admin() {

    std::cout << "Enter password: (5-16 characters containing one letter and one number)" << std::endl;

    std::string password;
    std::cin >> password;
    bool valid = false;
    if (valid_password(password)){
        is_admin = true;
        // do other stuff probably
        return true;
    }
    else{
        std::cout << "Invalid password format" << std::endl; // probably want to do this in a while loop instead
        return false;
    }

}
