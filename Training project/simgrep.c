#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UNDEFINED -1
int logFile = UNDEFINED;

struct Options {
	int caseInsensitive;
	int filenamesOnly;
	int linesNumbers;
	int numLines;
	int fullWord;
	int recursive;
	char* pattern;
	char* fileDir;
};

int createLogFile();
int writeLogFile(char* str);
int closeLogFile();
int openFile(const char* filename);
void processCmd(int argc, char** argv);
void treeSearch(char* path);
void patternSearch(char* filename);
void sigintHandler(int signo);
int blockSigint();


struct Options opt = {
		0, 0, 0, 0, 0, 0, NULL, NULL
};

int main(int argc, char** argv) {
	//createLogFile();
	blockSigint();
	processCmd(argc, argv);
	treeSearch(opt.fileDir);
	return 0;
}

int createLogFile() {
	const char* logFileName = getenv("LOGFILENAME");
	if (logFileName == NULL) {
		printf("LOGFILENAME environment variable not found!\n");
		exit(1);
	}

	logFile = open(logFileName, O_CREAT | O_WRONLY | O_APPEND, 0646);
	if (logFile == UNDEFINED) {
		perror(logFileName);
		return 1;
	}

	return 0;
}

int writeLogFile(char* str) {
	if (logFile != UNDEFINED) {
		write(logFile, str, strlen(str));
		return 0;
	}
	return 1;
}

int closeLogFile() {
	if (close(logFile) == -1)
		return 1;
	return 0;
}

int openFile(const char* filename) {
	int file = STDIN_FILENO;

	if (filename != NULL) {
		file = open(filename, O_RDONLY, 0644);
		if (file == UNDEFINED) {
			perror(filename);
			return -1;
		}
	}

	return file;
}

int isOption(char* str) {
	char* options[] = {
			"-i", "-l", "-n", "-c", "-w", "-r"
	};

	for (size_t i = 0; i < 6; i++)
		if (!strcmp(options[i], str))
			return 1;
	return 0;
}

void processCmd(int argc, char** argv) {

	for (int i = 1; i < argc; i++) {
		if (isOption(argv[i])) {
			switch(argv[i][1]) {
			case 'i':
				opt.caseInsensitive = 1;
				break;
			case 'l':
				opt.filenamesOnly = 1;
				break;
			case 'n':
				opt.linesNumbers = 1;
				break;
			case 'c':
				opt.numLines = 1;
				break;
			case 'w':
				opt.fullWord = 1;
				break;
			case 'r':
				opt.recursive = 1;
				break;
			}
		}
		else {
			if (opt.pattern == NULL) {
				opt.pattern = malloc(50 * sizeof(char));
				strcpy(opt.pattern, argv[i]);
			}
			else if (opt.fileDir == NULL) {
				opt.fileDir = malloc(50 * sizeof(char));
				strcpy(opt.fileDir, argv[i]);
			}
		}
	}

	if (opt.pattern == NULL) {
		printf("Usage: %s [options] pattern [file/dir]\n", argv[0]);
		exit(1);
	} else if (opt.fileDir == NULL) {
		patternSearch(NULL);
	}
}


void treeSearch(char* path) {

	DIR *dirp;
	struct dirent *direntp;
	struct stat stat_buf;
	pid_t fd;

	if (lstat(path, &stat_buf) != 0) exit(1);
	if (S_ISREG(stat_buf.st_mode)) {
		patternSearch(path);
		return;
	}

	if ((dirp = opendir(path)) == NULL) {
		perror(path);
		exit(2);
	}

	if (chdir(path) != 0) {
		perror(path);
		exit(3);
	}

	while ((direntp = readdir(dirp)) != NULL) {

		if (lstat(direntp->d_name, &stat_buf) != 0) exit(4);

		if (S_ISREG(stat_buf.st_mode)) {

			patternSearch(direntp->d_name);

		} else if (S_ISDIR(stat_buf.st_mode)) {

			if (strcmp(direntp->d_name, ".") &&
					strcmp(direntp->d_name, "..") &&
					opt.recursive) {

				switch(fd = fork()) {
				case 0:
					treeSearch(direntp->d_name);
					break;
				case -1:
					perror("fork error!");
					exit(5);
					break;
				}
			}
		}
	}

	closedir(dirp);
}

void patternSearch(char* filename) {

	char str[100];
	char* pattern = opt.pattern;
	int i = 0, k = 0;

	char lineBuff[100];
	int buffIndex = 0;

	int patternFound = 0;
	int line = 1;
	int numLines = 0;
	int wordFound = 0;

	int file = openFile(filename);

	while (read(file, str, 99)) {
		str[99] = '\0';

		while (str[i] != '\0') {
			lineBuff[buffIndex++] = str[i];

			// -i
			if (opt.caseInsensitive) {
				if (tolower(str[i] == tolower(pattern[k]))) k++;
				else k = 0;
			} else {
				if (str[i] == pattern[k]) k++;
				else k = 0;
			}

			if (k == strlen(pattern)) {
				// -w
				if ((str[i-k] == ' ' || str[i-k] == '.') &&
						(str[i+1] == ' ' || str[i+1] == '.')) {
					wordFound = 1;
				} else wordFound = 0;

				// -l
				if (opt.filenamesOnly && numLines < 1)
					printf("%s\n", filename);
				// -n
				else if (opt.linesNumbers) {
					printf("%d:", line);
				}

				patternFound = 1;
				k = 0;
			}

			if (str[i] == '\n' || str[i] == '.') {
				buffIndex = 0;

				if (!(opt.filenamesOnly || opt.numLines)) {

					if (patternFound || (wordFound && opt.fullWord)) {

						int idx = 0;
						while (lineBuff[idx] != '\n') {
							printf("%c", lineBuff[idx]);
							if(lineBuff[idx] == '.'){
								printf("\n");
								break;
							}
							idx++;
						}
					}
				}

				if (str[i] == '\n') {
					if (patternFound) {
						numLines++;
						patternFound = 0;
						wordFound = 0;
					}
					line++;
				}
			}

			i++;
		}

		i = 0;
	}

	// -c
	if (opt.numLines) printf("%d\n", numLines);

	close(file);
}

void sigintHandler(int signo) {

	char answer;
	do {
		printf("Are you sure you want to terminate the program? (Y/N) ");
		scanf("%c", &answer);
		answer = toupper(answer);
		if (answer == 'Y') {
			printf("Exiting as required ...\n");
			exit(0);
		}
	} while (answer != 'Y' && answer != 'N');
}

int blockSigint() {

	struct sigaction sig;

	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sig.sa_handler = sigintHandler;

	if (sigaction(SIGINT, &sig, NULL) < 0) {
		printf("Error blocking SIGINT!\n");
		return 1;
	}

	return 0;
}


