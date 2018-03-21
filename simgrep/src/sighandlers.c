#include "sighandlers.h"

void sigint_handler(int signo) {

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

int block_sigint() {

	struct sigaction sig;

	/* Handling SIGTERM signal */
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sig.sa_handler = sigint_handler;

	if (sigaction(SIGINT, &sig, NULL) < 0) {
		printf("Error blocking SIGINT!\n");
		return 1;
	}

	return 0;
}
