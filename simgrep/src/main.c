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
	create_logFile();

	/* block SIGINT */
	block_sigint();

	/* start process forking here */

	/* search for pattern */
	if (argc == 2) {
		search_pattern(argv[1], "test");
	} else {
		search_pattern(argv[2], "test");
	}

	/* closes log file */
	close_logFile();

	return 0;
}
