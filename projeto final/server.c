#include "server.h"

int main(int argc, char **argv)
{
    // creating server
    Server *sv = createServer(argc, argv);

    // creating & opening slog.txt
    slogFile = openFile(SLOG_FILE);

    // creating & opening fifo "requests"
    createFifo(FIFO_REQ, S_IRUSR | S_IWUSR);
    sv->fdFifoReq = openFifo(FIFO_REQ, O_RDONLY | O_NONBLOCK);

    // creating num_ticket_office threads
    createThreads(sv);

    // implementing alarm for open_time
    installAlarm(serverAlarmHandler, sv->openTime);

    // signal threads to be cancelled
    terminateThreads(sv);

    // closing and destroying fifo "requests"
    closeFifo(FIFO_REQ, sv->fdFifoReq);

    // TODO: register info - write on sbook.txt
    sbookFile = openFile(SBOOK_FILE);

    // closing server
    char closeMsg[13] = "SERVER CLOSED";
    writeFile(closeMsg, slogFile);

    // closing files
    closeFile(SBOOK_FILE, sbookFile);
    closeFile(SLOG_FILE, slogFile);

    return 0;
}

Server *createServer(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: $ server <num_room_seats>"
               " <num_ticket_offices> <open_time>\n");

        exit(1);
    }

    Server *sv = (Server *)malloc(sizeof(Server));
    sv->numTicketOffices = atoi(argv[2]);
    sv->openTime = atoi(argv[3]);
    sv->room = roomInit(atoi(argv[1]));

    printSvInfo(sv);
    return sv;
}

void printSvInfo(Server *sv)
{
    printf("num_room_seats: %d\n"
           "num_ticket_offices: %d\n"
           "open_time: %d\n",
           sv->room->numRoomSeats,
           sv->numTicketOffices,
           sv->openTime);
}

Room *roomInit(int numRoomSeats)
{
    Room *room = (Room *)malloc(sizeof(Room));
    room->numRoomSeats = numRoomSeats;

    room->seats = (Seat **)malloc(room->numRoomSeats * sizeof(Seat *));
    for (size_t i = 0; i < room->numRoomSeats; i++)
        room->seats[i] = NULL;

    return room;
}

int isRoomFull(Room *room)
{
    int full = 1;

    pthread_mutex_lock(&writeMutex);

    for (size_t i = 0; i < room->numRoomSeats; i++)
        if (room->seats[i] == NULL)
        {
            full = 0;
            break;
        }

    DELAY();

    pthread_mutex_unlock(&writeMutex);

    return full;
}

int isSeatFree(Seat **seats, int seatNum)
{
    int free = 0;

    pthread_mutex_lock(&writeMutex);

    if (seats[seatNum] == NULL)
        free = 1;

    DELAY();

    pthread_mutex_unlock(&writeMutex);

    return free;
}

void bookSeat(Seat **seats, int seatNum, int clientId)
{
    pthread_mutex_lock(&writeMutex);
    pthread_mutex_lock(&readMutex);

    seats[seatNum] = (Seat *)malloc(sizeof(Seat));
    seats[seatNum]->clientId = clientId;
    seats[seatNum]->ticketOfficeId = pthread_self();

    DELAY();

    pthread_mutex_unlock(&readMutex);
    pthread_mutex_unlock(&writeMutex);
}

void freeSeat(Seat **seats, int seatNum)
{
    pthread_mutex_lock(&writeMutex);
    pthread_mutex_lock(&readMutex);

    free(seats[seatNum]);

    DELAY();

    pthread_mutex_unlock(&readMutex);
    pthread_mutex_unlock(&writeMutex);
}

void createThreads(Server *sv)
{
    int numThreads = sv->numTicketOffices;
    sv->tids = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

    for (size_t i = 0; i < numThreads; i++)
    {
        int rc = pthread_create(&sv->tids[i], NULL, readRequest, (void *)sv);
        if (rc)
        {
            printf("Could not create thread - error code %d\n", rc);
            exit(1);
        }

        char openMsg[10];
        sprintf(openMsg, "%02lu-OPEN\n", i + 1);
        writeFile(openMsg, slogFile);
    }
}

void *readRequest(void *arg)
{
    Server *sv = (Server *)arg;
    Room *room = sv->room;

    Request *req = (Request *) malloc(sizeof(Request));

    while (!endThread)
    {
        pthread_mutex_lock(&writeMutex);
        int fifoRead = read(sv->fdFifoReq, req, sizeof(Request));
        pthread_mutex_unlock(&writeMutex);

        if (fifoRead > 0)
        {
            printf("Request read!\n");
            int validReq = (verifyRequest(sv, req) == 1);

            if (validReq)
            {
                printf("Valid request!\n");
                Answer *ans = handleReservation(sv, req);
                sendAnswer(req, ans);
            }
            else
            { 
                printf("Not a valid request!\n");
                // TODO: need to parse answer to string !
            }

            // TODO: register info - write on slog.txt
        }

        DELAY();
    }
}

int verifyRequest(Server *sv, Request *req)
{
    if (isRoomFull(sv->room)) 
        return FUL;

    if (req->numWantedSeats < 1 ||
        req->numWantedSeats > MAX_CLI_SEATS)
        return MAX;

    if (req->numPrefSeats < req->numWantedSeats ||
        req->numPrefSeats > MAX_CLI_SEATS)
        return NST;

    int numAvailableSeats = 0;
    for (size_t i = 0; i < req->numPrefSeats; i++) {
        if (isSeatFree(sv->room->seats, req->wantedSeats[i]))
            numAvailableSeats++;
    }
    if (numAvailableSeats < req->numWantedSeats)
        return NAV;

    return 1;
}

Answer *handleReservation(Server *sv, Request *req)
{
    Seat **seats = sv->room->seats;
    Answer *ans = (Answer *)malloc(sizeof(Answer));

    int index = 0;
    ans->numReservedSeats = 0;
    while (ans->numReservedSeats < req->numWantedSeats)
    {
        if (isSeatFree(seats, req->wantedSeats[index])) {
            bookSeat(seats, req->wantedSeats[index], req->clientId);
            ans->reservedSeats[ans->numReservedSeats] = req->wantedSeats[index];
            ans->numReservedSeats++;
        }
        index++;
    }

    return ans;
}

void sendAnswer(Request *req, Answer *ans)
{
    char clientStr[12];
    sprintf(clientStr, "%d", req->clientId);

    char fifoAns[20] = "/tmp/ans";
    strcat(fifoAns, clientStr);
    printf("fifoAns: %s\n", fifoAns);

    int clientFifo = openFifo(fifoAns, O_WRONLY);
    if (write(clientFifo, ans, sizeof(Answer)) < 0)
        printf("Answer could not be sent!\n");
}

void terminateThreads(Server *sv)
{
    pthread_mutex_lock(&readMutex);
    endThread = 1;
    pthread_mutex_unlock(&readMutex);

    int numThreads = sv->numTicketOffices;
    for (size_t i = 0; i < numThreads; i++)
    {
        pthread_join(sv->tids[i], NULL);

        char closeMsg[10];
        sprintf(closeMsg, "%02lu-CLOSED\n", i + 1);
        writeFile(closeMsg, slogFile);
    }
}

void serverAlarmHandler(int signum)
{
    printf("Server closing ...\n");
}