#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <stdarg.h>

#define MAX_ROOM_SEATS  9999
#define MAX_CLI_SEATS   99
#define WIDTH_PID       5
#define WIDTH_XXNN      5
#define WIDTH_SEAT      4

#define MAX -1          // wanted_seats > MAX_CLI_SEATS
#define NST -2          // invalid num of seat_ids
#define IID -3          // invalid seat_ids
#define ERR -4          // another parameter errors
#define NAV -5          // at least 1 seat not available
#define FUL -6          // full room
#define TMO -7          // timeout

#define FIFO_REQ        "/tmp/requests"
#define SLOG_FILE       "slog.txt"
#define SBOOK_FILE      "sbook.txt"
#define CLOG_FILE       "clog.txt"
#define CBOOK_FILE      "cbook.txt"

#define DELAY() usleep(1000)

typedef struct Answer
{
    int errorCode;
    int numReservedSeats;
    int reservedSeats[MAX_CLI_SEATS];
} Answer;

typedef struct Request
{
    int clientId;
    int numWantedSeats;
    int numPrefSeats;
    int wantedSeats[MAX_CLI_SEATS];
} Request;

typedef struct Seat
{
    int clientId;
    pid_t ticketOfficeId;
} Seat;

typedef struct Room
{
    int numRoomSeats;
    Seat **seats;
} Room;

typedef struct Client
{
    Request *req;
    Answer *ans;
    char *fifoAns;
    int fdFifoAns;
    int fdFifoReq;
    int timeout;
} Client;

typedef struct Server
{
    Room *room;
    int numTicketOffices;
    pthread_t *tids;
    int openTime;
    int fdFifoReq;
} Server;

typedef struct ThreadArgs
{
    Server *sv;
    pthread_t tid;
    Request *req;
    Answer *ans;
} ThreadArgs;

// Utility functions
void createFifo(char *fifoname, mode_t perm);
int openFifo(char *fifoname, mode_t mode);
void closeFifo(char *fifoname, int fd);
FILE *openFile(char *filename);
void writeFile(char *txt, FILE *file);
void closeFile(char *filename, FILE *file);
const char *getError(int error);
int parseString(char *str, int *intArr);
void installAlarm(void (*handler)(int), int time);