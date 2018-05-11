#include "client.h"

int main(int argc, char **argv)
{
    printf("** Running process %d (PGID %d) **\n", getpid(), getpgrp());

    // creating client
    Client *client = createClient(argc, argv);

    // creating & opening answer fifo
    client->fdFifoAns = createAnsFifo(client);

    // opening fifo requests
    client->fdFifoReq = openFifo(FIFO_REQ, O_WRONLY);

    // sending request
    sendRequest(client);

    // installing alarm for timeout
    // install_alarm(client_alarm_handler, client->timeout);

    // waiting for server's answer
    waitAnswer(client);

    // closing answer's fifo
    closeFifo(client->fifoAns, client->fdFifoAns);

    // creating & opening clog.txt & cbook.txt

    // writing to files

    // closing files
    closeFile(CLOG_FILE, clogFile);
    closeFile(CBOOK_FILE, cbookFile);

    return 0;
}

Client *createClient(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: $ client <time_out>"
               " <num_wanted_seats> <pref_seat_list>\n");

        exit(1);
    }

    Request *req = (Request *)malloc(sizeof(Request));
    req->clientId = getpid();
    req->numWantedSeats = atoi(argv[2]);

    // getting string from cmd line
    char seatsString[strlen(argv[3])];
    strcpy(seatsString, argv[3]);

    // reserving space & storing for favorite seats
    req->numPrefSeats = strlen(argv[3]) / 2 + 1;
    for (int i = 0, j = 0; j < req->numPrefSeats; i += 2, j++)
        req->wantedSeats[j] = (int)(seatsString[i] - '0');

    Client *client = (Client *)malloc(sizeof(Client));
    client->timeout = atoi(argv[1]);
    client->req = req;

    return client;
}

void setFifoname(Client *client)
{
    if (client->fifoAns == NULL)
    {
        char clientPid[10];
        sprintf(clientPid, "%d", getpid());
        char fifoPath[20] = "/tmp/ans";
        strcat(fifoPath, clientPid);

        client->fifoAns = (char *)malloc(strlen(fifoPath) * sizeof(char));
        strcpy(client->fifoAns, fifoPath);
    }
}

int createAnsFifo(Client *client)
{
    setFifoname(client);
    createFifo(client->fifoAns, S_IRUSR | S_IWUSR);
    return openFifo(client->fifoAns, O_RDONLY | O_NONBLOCK);
}

void sendRequest(Client *client)
{
    Request *req = client->req;

    int fifoWrite = write(client->fdFifoReq, req, sizeof(Request));
    if (fifoWrite < 0)
    {
        printf("Error while writing request to FIFO ...\n");
        exit(1);
    }
    else
        printf("Request sent!\n");
}

void waitAnswer(Client *client)
{
    Answer *ans = (Answer *)malloc(sizeof(Answer));

    while (1)
    {
        pthread_mutex_lock(&readMutex);
        int fifoRead = read(client->fdFifoAns, ans, sizeof(Answer));
        pthread_mutex_unlock(&readMutex);

        if (fifoRead > 0)
            printf("Answer received\n");
        else break;

        DELAY();
    }
}

void clientAlarmHandler(int signum)
{
    printf("Client timeout ...\n");
}