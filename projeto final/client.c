#include "client.h"

int main(int argc, char **argv)
{
    // creating client
    Client *client = createClient(argc, argv);

    // creating & opening clog.txt
    clogFile = openFile(CLOG_FILE);

    // creating & opening answer fifo
    client->fdFifoAns = createAnsFifo(client);

    // opening fifo requests
    client->fdFifoReq = openFifo(FIFO_REQ, O_WRONLY);

    // sending request
    sendRequest(client);

    // waiting for server's answer
    waitAnswer(client);

    // creating & opening cbook.txt
    cbookFile = openFile(CBOOK_FILE);

    // writing to cbook.txt
    writeCbook(client);

    // freeing resources
    freeResources(client);

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
    req->numPrefSeats = parseString(argv[3], req->wantedSeats);

    Client *client = (Client *)malloc(sizeof(Client));
    client->req = req;
    client->ans = NULL;
    client->fifoAns = NULL;
    client->timeout = atoi(argv[1]);

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
    {
        printf("Request sent!\n");
    }
}

void waitAnswer(Client *client)
{
    double timeDiff = 0;
    time_t start, stop;

    client->ans = (Answer *)malloc(sizeof(Answer));

    time(&start);
    while (1)
    {
        time(&stop);
        timeDiff = difftime(stop, start);
        if (timeDiff >= client->timeout)
        {
            client->ans->errorCode = TMO;
            printf("Client timed out!\n");
            break;
        }

        pthread_mutex_lock(&readMutex);
        int fifoRead = read(client->fdFifoAns, client->ans, sizeof(Answer));
        pthread_mutex_unlock(&readMutex);

        if (fifoRead > 0)
        {
            printf("Answer received!\n");
            writeClog(client);
            break;
        }

        DELAY();
    }
}

void writeClog(Client *client)
{
    Answer *ans = client->ans;
    char line[50];

    sprintf(line, "%05d ", client->req->clientId);

    if (ans->errorCode != 0)
    {
        char *err = malloc(3 * sizeof(char));
        strcpy(err, getError(ans->errorCode));
        printf("err: %s\n", err);
        sprintf(line + strlen(line), "%s", err);
    }
    else
    {
        for (int i = 1; i <= ans->numReservedSeats; i++)
        {
            if (ans->errorCode == 0)
            {
                sprintf(line + strlen(line), "%02d", i);
                sprintf(line + strlen(line), ".");
                sprintf(line + strlen(line), "%02d ", ans->numReservedSeats);
                sprintf(line + strlen(line), "%04d\n", ans->reservedSeats[i - 1]);
            }
        }
    }

    writeFile(line, clogFile);
}

void writeCbook(Client *client)
{

    Answer *ans = client->ans;

    for (int i = 1; i <= ans->numReservedSeats; i++)
    {
        char seat[4];
        sprintf(seat, "%04d\n", ans->reservedSeats[i - 1]);
        writeFile(seat, cbookFile);
    }
}

void freeResources(Client *client)
{
    closeFifo(client->fifoAns, client->fdFifoAns);
    closeFile(CLOG_FILE, clogFile);
    closeFile(CBOOK_FILE, cbookFile);
    free(client);
}