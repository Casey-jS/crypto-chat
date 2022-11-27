#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <cwctype> // for validating the username
#include <regex>
#include <string>

using namespace std;

bool is_admin = false;

bool valid_username(string username){

    if (username.length() < 4 || username.length() > 15){ return false; }

    regex valid_name("^[a-zA-Z0-9-_]{4,15}");

    if (regex_match(username, valid_name)){ return true; }

    return false;
}

// must have 1 letter, 1 num, one special character
bool valid_password(string password){

    if (password.length() < 6 || password.length() > 16){ return false; }

    regex valid_password("^(?=.*[A-Za-z])(?=.*\d)[A-Za-z\d]{6,16}");

    if(regex_match(password, valid_password)){ return true; }

    return false;
}

// this function will kick the user that is running this client
// i.e. another user uses command !kick {username}
// client must detect that it has been kicked immediately
void handle_kick(){
    cout << "You have been kicked from the chat." << endl;
    exit(1);
}
// this function will be called when the user runs !admin
// should prompt for password input (hidden)
bool become_admin(){

    cout << "Enter password: (6-16 characters containing one letter and one number)" << endl;

    string password;
    cin >> password;
    bool valid = false;
    if (valid_password(password)){
        is_admin = true;
        // do other stuff probably
        return true;
    }
    else{ 
        cout << "Invalid password format" << endl; // probably want to do this in a while loop instead
        return false;
    }

}

// send a packet with the client's ip addr in the header 
void send_message(string client_addr, string message){

}

string handle_command(string command){

    if (command == "!admin"){
        if (become_admin() == true){
            return "Admin access granted";
        }
    }

    if (!is_admin){
        return "You do not have admin rights. Run !admin to execute this command";
    }
    // if the user IS an admin
    else{
        if (command == "!kick"){
            // need to figure out how to specify which user to kick
            // also, only check characters before a whitespace (so for "!kick user1", only "!kick" will be registered as the command)
        }
    }


    return "Invalid command";
}

int main(int arc, char** argv){
    string username;
    bool valid = false;

    while (!valid){
        cout << "Enter a username: (4-15 alphanumeric characters, plus _ or -)" << endl;
        cin >> username;
        valid = valid_username(username);
        if (!valid){ cout << "Invalid username!" << endl; }
    }
    cout << "Valid username entered." << endl;

    // main loop
    while(true){
        string input;
        cin >> input;

        if (input[0] == '!'){
            cout << handle_command(input) << endl; // handle_command should return a string indicating what happened as result of command
            
        }
        else{
            send_message(input);
        }
    }


    

}