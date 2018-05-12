#include "utils.h"

// Mutexes
int endThread = 0;
pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;

// Server files
FILE *slogFile;
FILE *sbookFile;

// Server function prototypes
Server *createServer(int argc, char **argv);
void printSvInfo(Server *sv);
Room *roomInit(int num_room_seats);
int isSeatFree(Seat **seats, int seat_num);
void bookSeat(Seat **seats, int seat_num, int client_id);
void freeSeat(Seat **seats, int seat_num);
void createThreads(Server *sv);
void serverAlarmHandler(int signum);
void *readRequest(void *arg);
int verifyRequest(ThreadArgs *args);
void handleReservation(ThreadArgs *args);
void writeSlog(ThreadArgs *args);
void sendAnswer(ThreadArgs *args);
void terminateThreads(Server *sv);
void writeSbook(Server *sv);
void freeResources(Server *sv);