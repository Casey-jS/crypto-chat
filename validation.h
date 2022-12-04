#ifndef VALIDATION_H
#define VALIDATION_H

#include <iostream>
#include <string.h>

int handle_command(unsigned char (&line)[5000], char command, unsigned char user[10]);
bool valid_username(std::string username);

#endif
