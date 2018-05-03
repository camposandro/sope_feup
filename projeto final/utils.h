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

#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4

#define MAX -1  // wanted_seats > MAX_CLI_SEATS
#define NST -2  // invalid num of seat_ids
#define IID -3  // invalid seat_ids
#define ERR -4  // another parameter errors
#define NAV -5  // at least 1 seat not available
#define FUL -6  // full room

#define FIFONAME    "/tmp/requests"
#define SLOG_FILE   "slog.txt"
#define SBOOK_FILE  "sbook.txt"   

#define DELAY() sleep(1)

typedef struct Request
{
    int client_id;
    int num_wanted_seats;
    int *wanted_seats;
} Request;

typedef struct Seat
{
    int client_id;
    pid_t ticket_office_id;
} Seat;

typedef struct Room
{
    int num_room_seats;
    Seat **seats;
} Room;

typedef struct Server
{
    Room *room;
    int num_ticket_offices;
    int open_time;
    int fifo_requests;
    FILE* slog_file;
    FILE* sbook_file;
} Server;

Server *create_server(int argc, char **argv);
void print_sv_info(Server *sv);

Room *room_init(int num_room_seats);
int isSeatFree(Seat **seats, int seat_num);
void bookSeat(Seat **seats, int seat_num, int client_id);
void freeSeat(Seat **seats, int seat_num);

int create_fifo();
void close_fifo();

pthread_t *create_threads(Server *sv);
void install_alarm(int open_time);
void *read_request(void *arg);
void join_threads();

FILE* open_file(char *filename);
void close_files(Server *sv);