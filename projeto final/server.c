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
    installAlarm(alarmHandler, sv->openTime);

    // signal threads to be cancelled
    terminateThreads(sv);

    // creating & opening sbook.txt
    sbookFile = openFile(SBOOK_FILE);

    // registering reserved seats on sbook.txt
    writeSbook(sv);

    char closeMsg[13] = "SERVER CLOSED";
    writeFile(closeMsg, slogFile);

    // freeing all resources
    freeResources(sv);

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
    {
        if (room->seats[i] == NULL)
        {
            full = 0;
            break;
        }
    }

    DELAY();

    pthread_mutex_unlock(&writeMutex);

    return full;
}

int isSeatFree(Seat **seats, int seatNum)
{
    int free = 0;

    pthread_mutex_lock(&writeMutex);

    if (seats[seatNum - 1] == NULL)
        free = 1;

    DELAY();

    pthread_mutex_unlock(&writeMutex);

    return free;
}

void bookSeat(Seat **seats, int seatNum, int clientId)
{
    pthread_mutex_lock(&writeMutex);
    pthread_mutex_lock(&readMutex);

    seats[seatNum - 1] = (Seat *)malloc(sizeof(Seat));
    seats[seatNum - 1]->clientId = clientId;
    seats[seatNum - 1]->ticketOfficeId = pthread_self();

    DELAY();

    pthread_mutex_unlock(&readMutex);
    pthread_mutex_unlock(&writeMutex);
}

void freeSeat(Seat **seats, int seatNum)
{
    pthread_mutex_lock(&writeMutex);
    pthread_mutex_lock(&readMutex);

    free(seats[seatNum - 1]);

    DELAY();

    pthread_mutex_unlock(&readMutex);
    pthread_mutex_unlock(&writeMutex);
}

void createThreads(Server *sv)
{
    int numThreads = sv->numTicketOffices;
    sv->tids = (pthread_t *)malloc(numThreads * sizeof(pthread_t));

    for (size_t i = 1; i <= numThreads; i++)
    {
        ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        args->sv = sv;
        args->tid = i;

        int rc = pthread_create(&sv->tids[i - 1], NULL, readRequest, (void *)args);
        if (rc)
        {
            printf("Could not create thread - error code %d\n", rc);
            exit(1);
        }

        char openMsg[10];
        sprintf(openMsg, "%02lu-OPEN\n", i);
        writeFile(openMsg, slogFile);
    }
}

void *readRequest(void *parameter)
{
    ThreadArgs *args = (ThreadArgs *)parameter;
    Server *sv = args->sv;
    Answer *ans = args->ans;

    while (!endThread)
    {
        Request *req = (Request *)malloc(sizeof(Request));
        Answer *ans = (Answer *)malloc(sizeof(Answer));

        pthread_mutex_lock(&writeMutex);
        int fifoRead = read(sv->fdFifoReq, req, sizeof(Request));
        pthread_mutex_unlock(&writeMutex);

        args->req = req;
        args->ans = ans;

        if (fifoRead > 0)
        {
            printf("Request read!\n");
            int validity = verifyRequest(args);

            if (validity == 1)
            {
                printf("Valid request!\n");
                ans->errorCode = 0;
                handleReservation(args);
            }
            else
            {
                printf("Not a valid request!\n");
                ans->errorCode = validity;
            }

            sendAnswer(args);
            writeSlog(args);

            free(req);
            free(ans);
        }

        DELAY();
    }
}

int verifyRequest(ThreadArgs *args)
{
    Server *sv = args->sv;
    Request *req = args->req;

    if (isRoomFull(sv->room))
        return FUL;

    if (req->numWantedSeats < 1 ||
        req->numWantedSeats > MAX_CLI_SEATS)
        return MAX;

    if (req->numPrefSeats < req->numWantedSeats ||
        req->numPrefSeats > MAX_CLI_SEATS)
        return NST;

    for (size_t i = 0; i < req->numPrefSeats; i++)
        if (req->wantedSeats[i] < 1 ||
            req->wantedSeats[i] > sv->room->numRoomSeats)
            return IID;

    int numAvailableSeats = 0;
    for (size_t j = 0; j < req->numPrefSeats; j++)
        if (isSeatFree(sv->room->seats, req->wantedSeats[j]))
            numAvailableSeats++;

    if (numAvailableSeats < req->numWantedSeats)
        return NAV;

    return 1;
}

void handleReservation(ThreadArgs *args)
{
    Seat **seats = args->sv->room->seats;
    Request *req = args->req;
    Answer *ans = args->ans;

    ans->numReservedSeats = 0;

    int idx = 0;
    while (ans->numReservedSeats < req->numWantedSeats)
    {
        int currentSeat = req->wantedSeats[idx];
        if (isSeatFree(seats, currentSeat))
        {
            bookSeat(seats, currentSeat, req->clientId);
            ans->reservedSeats[ans->numReservedSeats] = currentSeat;
            ans->numReservedSeats++;
        }

        idx++;
    }
}

void sendAnswer(ThreadArgs *args)
{
    char clientStr[12];
    sprintf(clientStr, "%d", args->req->clientId);

    char fifoAns[20] = "/tmp/ans";
    strcat(fifoAns, clientStr);

    int clientFifo = openFifo(fifoAns, O_WRONLY);

    if (write(clientFifo, args->ans, sizeof(Answer)) < 0)
        printf("Answer could not be sent!\n");
}

void writeSlog(ThreadArgs *args)
{
    Request *req = args->req;
    Answer *ans = args->ans;

    char line[50];
    sprintf(line, "%02lu-%d-%02d: ", args->tid, req->clientId, req->numWantedSeats);

    for (int i = 0; i < req->numPrefSeats; i++)
        sprintf(line + strlen(line), "%04d ", req->wantedSeats[i]);

    sprintf(line + strlen(line), "- ");

    if (!ans->errorCode)
    {
        for (int j = 0; j < ans->numReservedSeats; j++)
            sprintf(line + strlen(line), "%04d ", ans->reservedSeats[j]);
    }
    else {
        sprintf(line + strlen(line), "%s", getError(ans->errorCode));
    }

    sprintf(line + strlen(line), "\n");

    writeFile(line, slogFile);
}

void terminateThreads(Server *sv)
{
    pthread_mutex_lock(&readMutex);
    endThread = 1;
    pthread_mutex_unlock(&readMutex);

    int numThreads = sv->numTicketOffices;
    for (size_t i = 1; i <= numThreads; i++)
    {
        pthread_join(sv->tids[i - 1], NULL);

        char closeMsg[10];
        sprintf(closeMsg, "%02lu-CLOSED\n", i);
        writeFile(closeMsg, slogFile);
    }
}

void writeSbook(Server *sv)
{
    Room *room = sv->room;
    for (int i = 1; i <= room->numRoomSeats; i++)
    {
        if (room->seats[i - 1] != NULL)
        {
            char seat[4];
            sprintf(seat, "%04d\n", i);
            writeFile(seat, sbookFile);
        }
    }
}

void freeResources(Server *sv)
{
    closeFifo(FIFO_REQ, sv->fdFifoReq);
    closeFile(SBOOK_FILE, sbookFile);
    closeFile(SLOG_FILE, slogFile);
    free(sv);
}

void alarmHandler(int signum)
{
    printf("Server closing ...\n");
}