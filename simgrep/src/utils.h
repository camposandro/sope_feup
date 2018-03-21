#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#ifndef UTILS_H_
#define UTILS_H_

int check_inputs(int argc, char** argv);

int open_logFile();

int write_logFile();

#endif /* UTILS_H_ */
