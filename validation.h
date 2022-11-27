#ifndef VALIDATION_H
#define VALIDATION_H

#include <iostream>
#include <string.h>

int handle_command(char *line, char command, char user[10]);
bool valid_username(std::string username);
bool valid_password(std::string password);
bool become_admin();

#endif
