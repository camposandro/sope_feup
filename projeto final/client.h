#include "utils.h"

pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;

// Client files
FILE *clogFile;
FILE *cbookFile;

// Client function prototypes
Client *createClient(int argc, char **argv);
int createAnsFifo();
void sendRequest(Client *client);
void waitAnswer(Client *client);
void clientAlarmHandler(int signum);
void freeResources(Client *client);