#include "utils.h"

#define UNKNOWN -1

/** Files descriptor */
int fd_logFile = UNKNOWN;
int fd_fileToSearch = UNKNOWN;

int check_inputs(int argc, char** argv) {

	char* options[] = { "-i","-l","-n","-c","-w","-r" };

	if (argc < 2 || argc > 4)
		return 1;
	else if (argc == 2)
		return 0;
	else {
		/* need to improve this part:
		 * - with argc == 3 there may or not be options
		 * - with argc == 4 there's always an option ?
		 */
		for (size_t i = 0; i < 6; i++)
			if (strcmp(options[i], argv[1]) == 0)
				return 0;
	}

	return 1;
}

int create_logFile() {

	const char* logFileName = getenv("LOGFILENAME");
	if (logFileName == NULL) {
		perror("LOGFILENAME");
		return 1;
	}

	fd_logFile = open(logFileName, O_CREAT | O_WRONLY | O_APPEND, 0646);
	if (fd_logFile == UNKNOWN) {
		perror(logFileName);
		return 1;
	}

	return 0;
}

int write_logFile(char* str) {

	if (fd_logFile != UNKNOWN) {
		write(fd_logFile, str, strlen(str));
		return 0;
	}
	return 1;
}

int close_logFile() {

	if (close(fd_logFile) == -1) {
		return 1;
	}
	return 0;
}

int open_file(const char* filename) {

	fd_fileToSearch = open(filename, O_RDONLY, 0644);
	if (fd_fileToSearch == UNKNOWN) {
		perror(filename);
		return 1;
	}

	return 0;
}

void search_pattern(char* pattern, char* file) {

	char str[100];
	int i = 0, k = 0, count = 0;

	/* open file to search */
	open_file(file);

	/* read 100 chars at a time */
	while (read(fd_fileToSearch, str, 99)) {

		/* add terminating char */
		str[99] = '\0';

		/* search for substring */
		while (str[i] != '\0') {

			if (str[i] == pattern[k]) k++;
			else k = 0;

			/* pattern found */
			if (k == strlen(pattern)) {
				count++;
				k = 0;
			}
			i++;
		}

		i = 0;
	}

	printf("Number of patterns found = %d\n", count);

	/* closing text file */
	close(fd_fileToSearch);
}

