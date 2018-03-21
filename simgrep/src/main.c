#include "utils.h"
#include "sighandlers.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int main(int argc, char** argv) {

	/* check for invalid command inputs */
	int invalidInputs = check_inputs(argc, argv);
	if (invalidInputs) {
		printf("Usage: %s [options] pattern [file/dir]\n", argv[0]);
		return 1;
	}

	/* create/re-open log file */
	open_logFile();

	/* block SIGINT */
	block_sigint();

	while(1) {
		write_logFile();
		sleep(2);
	}

	return 0;
}
