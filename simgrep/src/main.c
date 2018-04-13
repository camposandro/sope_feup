#include "utils.h"
#include "sighandlers.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

typedef struct {
	int i;
	int l;
	int n;
	int c;
	int w;
	int r;
} options;

int main(int argc, char** argv) {

	char optionsUsed[5];

	options opt = { .i = 0, .l = 0, .n = 0, .c = 0, .w = 0, .r = 0 };

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

	for (int i = 1; i < argc; i++) {

		if (strcmp(argv[i], '-i')) {
			opt.i = 1;
		}

		if (strcmp(argv[i], '-l')) {
			opt.l = 1;
		}

		if (strcmp(argv[i], '-n')) {
			opt.n = 1;
		}

		if (strcmp(argv[i], '-c')) {
			opt.c = 1;
		}

		if (strcmp(argv[i], '-w')) {
			opt.w = 1;
		}

		if (strcmp(argv[i], '-r')) {
			opt.r = 1;
		}
	}

	search_pattern(argv[1], "test", &opt);

	/* closes log file */
	close_logFile();

	return 0;
}
