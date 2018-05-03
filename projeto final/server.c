#include "utils.h"

int alarm_received = 0;

int main(int argc, char **argv)
{
    // creating server
    Server *sv = create_server(argc, argv);

    // creating & opening slog.txt
    sv->slog_file = open_file(SLOG_FILE);

    // creating & opening fifo "requests"
    sv->fifo_requests = create_fifo();

    // creating num_ticket_office threads
    pthread_t *tids = create_threads(sv);

    // implementing alarm for open_time
    install_alarm(sv->open_time);

    // closing and destroying fifo "requests"
    close_fifo(sv->fifo_requests);

    // TODO: wait for threads to end
    join_threads();

    // TODO: register info - write on slog.txt

    // TODO: register info - write on sbook.txt
    sv->sbook_file = open_file(SBOOK_FILE);

    // closing files
    close_files(sv);

    return 0;
}

Server *create_server(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: $ server <num_room_seats>"
               " <num_ticket_offices> <open_time>\n");

        exit(1);
    }

    Server *sv = (Server *)malloc(sizeof(Server));
    sv->num_ticket_offices = atoi(argv[2]);
    sv->open_time = atoi(argv[3]);
    sv->room = room_init(atoi(argv[1]));

    print_sv_info(sv);
    return sv;
}

void print_sv_info(Server *sv)
{
    printf("num_room_seats: %d\n"
           "num_ticket_offices: %d\n"
           "open_time: %d\n",
           sv->room->num_room_seats,
           sv->num_ticket_offices,
           sv->open_time);
}

Room *room_init(int num_room_seats)
{
    Room *room = (Room *)malloc(sizeof(Room));
    room->num_room_seats = num_room_seats;

    room->seats = (Seat **)malloc(room->num_room_seats * sizeof(Seat *));
    for (size_t i = 0; i < room->num_room_seats; i++)
        room->seats[i] = NULL;

    return room;
}

int is_room_full(Room *room)
{
    for (size_t i = 0; i < room->num_room_seats; i++)
        if (room->seats[i] == NULL)
            return 0;
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
    while (mkfifo(FIFONAME, S_IRUSR | S_IWUSR) < 0)
    {
        if (errno == EEXIST)
            unlink(FIFONAME);
        else
        {
            perror(FIFONAME);
            exit(1);
        }
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

pthread_t *create_threads(Server *sv)
{
    int num_threads = sv->num_ticket_offices;
    pthread_t *tids = (pthread_t *)malloc(num_threads * sizeof(pthread_t));

    for (size_t i = 0; i < num_threads; i++)
    {
        int rc = pthread_create(&tids[i], NULL, read_request, (void *)sv);
        if (rc)
        {
            printf("Could not create thread - error code %d\n", rc);
            exit(1);
        }
    }

    return tids;
}

void *read_request(void *arg)
{
    int ans; // answer for client
    Server *sv = (Server *)arg;
    Request *req = (Request *)malloc(sizeof(Request));

    while (read(sv->fifo_requests, req, sizeof(Request)) < 0)
    {
        if (req->num_wanted_seats < 1 ||
            req->num_wanted_seats > MAX_CLI_SEATS)
            ans = MAX;

        int num_preferred_seats = sizeof(*req->wanted_seats) / sizeof(int);
        if (num_preferred_seats < req->num_wanted_seats ||
            num_preferred_seats > MAX_CLI_SEATS)
            ans = NST;

        if (is_room_full(sv->room))
            ans = FUL;

        int num_available_seats = 0;
        for (size_t i = 0; i < num_preferred_seats; i++)
            if (isSeatFree(sv->room->seats, req->wanted_seats[i]))
                num_available_seats++;
        if (num_available_seats < req->num_wanted_seats)
            ans = NAV;

        // TODO: valid request - process it

        // TODO: send info to client through fifo

        DELAY();
    }
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

void close_files(Server *sv)
{
    if (fclose(sv->sbook_file) != 0)
        perror(SBOOK_FILE);

    if (fclose(sv->slog_file) != 0)
        perror(SLOG_FILE);
}

void alarm_handler(int signum)
{
    printf("Server closing ...\n");
}

void install_alarm(int open_time)
{
    struct sigaction action;
    sigset_t sigmask;

    action.sa_handler = alarm_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);

    sigfillset(&sigmask);
    sigdelset(&sigmask, SIGALRM);

    alarm(open_time);

    sigsuspend(&sigmask);
}