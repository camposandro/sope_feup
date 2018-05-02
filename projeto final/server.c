#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

// Macros definition
#define INVNUMARGS 1

#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4

#define MAX -1 // wanted_seats > MAX_CLI_SEATS
#define NST -2 // invalid num of seat_ids
#define IID -3 // invalid seat_ids
#define ERR -4 // another parameter errors
#define NAV -5 // at least 1 seat not available
#define FUL -6 // full room

#define DELAY(time) sleep(time)

typedef struct Seat
{
    int available;
    int *client_id;
    pid_t *ticket_office_id;
} Seat;

typedef struct Room
{
    int num_room_seats;
    int num_ticket_offices;
    int open_time;
    Seat *seats;
} Room;

// function prototypes
Room *process_args(int argc, char **argv);
void room_init(Room *room);
int isSeatFree(Seat *seats, int seat_num);
void bookSeat(Seat *seats, int seat_num, int client_id);
void freeSeat(Seat *seats, int seat_num, int client_id);

int main(int argc, char **argv)
{
    Room *room = process_args(argc, argv);

    printf("num_room_seats: %d\nnum_ticket_offices: %d\nopen_time: %d\n",
           room->num_room_seats, room->num_ticket_offices, room->open_time);

    return 0;
}

Room *process_args(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: $ server <num_room_seats>"
               " <num_ticket_offices> <open_time>\n");

        exit(INVNUMARGS);
    }

    Room *room = (Room *)malloc(sizeof(Room));

    room->num_room_seats = atoi(argv[1]);
    room->num_ticket_offices = atoi(argv[2]);
    room->open_time = atoi(argv[3]);

    room_init(room);

    return room;
}

void room_init(Room *room)
{
    room->seats = (Seat *)malloc(room->num_room_seats);
    for (size_t i = 0; i < room->num_room_seats; i++)
        room->seats[i].available = 1;
}

int isSeatFree(Seat *seats, int seat_num)
{
    return seats[seat_num].available;
}

void bookSeat(Seat *seats, int seat_num, int client_id)
{
    seats[seat_num].client_id = (int *)malloc(sizeof(int));
    seats[seat_num].ticket_office_id = (pid_t *)malloc(sizeof(pid_t));

    seats[seat_num].available = 0;
    *(seats[seat_num].client_id) = client_id;
    *(seats[seat_num].ticket_office_id) = getpid();
}

void freeSeat(Seat *seats, int seat_num, int client_id)
{
    seats[seat_num].client_id = (int *)malloc(sizeof(int));
    seats[seat_num].ticket_office_id = (pid_t *)malloc(sizeof(pid_t));

    seats[seat_num].available = 1;
    *(seats[seat_num].client_id) = client_id;
    *(seats[seat_num].ticket_office_id) = getpid();
}
