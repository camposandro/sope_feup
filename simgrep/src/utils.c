#include "utils.h"

#define UNKNOWN -1

int fd_logFile = UNKNOWN;

int check_inputs(int argc, char** argv) {

	char* options[] = { "-i","-l","-n","-c","-w","-r" };

	if (argc < 2 || argc > 4)
		return 1;
	else if (argc == 2)
		return 0;
	else {
		for (size_t i = 0; i < 6; i++)
			if (strcmp(options[i], argv[1]) == 0)
				return 0;
	}

	return 1;
}

int open_logFile() {

	const char* logFileName = getenv("LOGFILENAME");
	fd_logFile = open(logFileName, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd_logFile == -1) {
		perror(logFileName);
		return 1;
	}

	return 0;
}

int write_logFile() {

	if (fd_logFile != UNKNOWN) {
		write(fd_logFile, "Iteration\n", 10);
	}

	return 0;
}
