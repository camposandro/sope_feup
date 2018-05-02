#include "utils.h"

int main(int argc, char **argv)
{
    Server *sv = (Server *)malloc(sizeof(Server));
    sv->room = process_args(argc, argv);

    printSvInfo(sv);

    // creating & opening fifo "requests"
    sv->fifo_requests = create_fifo();

    // creating & opening slog.txt
    sv->slog_file = open_file(SLOG_FILE);

    // creating num_ticket_office threads
    pthread_t *tids = create_threads(sv->room);

    // TODO: implement alarm for open_time

    // closeing and destroying fifo "requests"
    close_fifo(sv->fifo_requests);

    // TODO: wait for threads to end
    join_threads();

    // TODO: register info - write on slog.txt

    return 0;
}

Room *process_args(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: $ server <num_room_seats>"
               " <num_ticket_offices> <open_time>\n");

        exit(1);
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
    room->seats = (Seat **)malloc(room->num_room_seats * sizeof(Seat *));
    for (size_t i = 0; i < room->num_room_seats; i++)
        room->seats[i] = NULL;
}

int isSeatFree(Seat **seats, int seat_num)
{
    int isFree = 0;
    // -- secção crítica!
    if (seats[seat_num] == NULL)
    {
        isFree = 1;
        DELAY();
    }
    return isFree;
}

void bookSeat(Seat **seats, int seat_num, int client_id)
{
    // -- secção crítica!
    seats[seat_num] = (Seat *)malloc(sizeof(Seat));
    seats[seat_num]->client_id = client_id;
    seats[seat_num]->ticket_office_id = pthread_self();

    DELAY();
}

void freeSeat(Seat **seats, int seat_num)
{
    // -- secção crítica!
    free(seats[seat_num]);

    DELAY();
}

int create_fifo()
{
    if (mkfifo(FIFONAME, S_IRUSR | S_IWUSR) < 0)
    {
        if (errno == EEXIST)
            printf("FIFO %s already exists!\n", FIFONAME);
        else
            perror(FIFONAME);

        exit(1);
    }

    int fd = open(FIFONAME, O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        perror(FIFONAME);
        exit(1);
    }

    return fd;
}

void close_fifo(int fd)
{
    close(fd);

    if (unlink(FIFONAME) < 0)
    {
        printf("Error while destroying FIFO %s!\n", FIFONAME);
        exit(1);
    }
}

pthread_t *create_threads(Room *room)
{
    int num_threads = room->num_ticket_offices;
    pthread_t *tids = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    for (size_t i = 0; i < num_threads; i++)
    {
        int rc = pthread_create(&tids[i], NULL, thread_func, room);
        if (rc)
        {
            printf("Could not create thread - error code %d\n", rc);
            exit(1);
        }
    }

    return tids;
}

void *thread_func(void *arg)
{
}

void join_threads()
{
}

FILE *open_file(char *filename)
{
    FILE *file = fopen(filename, "a");

    if (file != NULL)
        return file;
    else
    {
        perror(filename);
        exit(1);
    }
}

void printSvInfo(Server *sv)
{
    printf("num_room_seats: %d\n"
           "num_ticket_offices: %d\n"
           "open_time: %d\n",
           sv->room->num_room_seats,
           sv->room->num_ticket_offices,
           sv->room->open_time);
}