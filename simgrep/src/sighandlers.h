#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef SIGHANDLERS_H_
#define SIGHANDLERS_H_

void sigint_handler(int signo);

int block_sigint();

#endif /* SIGHANDLERS_H_ */
