#include "utils.h"

void createFifo(char *fifoname, mode_t perm)
{
    while (mkfifo(fifoname, perm) < 0)
    {
        if (errno == EEXIST)
            unlink(fifoname);
        else
        {
            perror(fifoname);
            exit(1);
        }
    }
}

int openFifo(char *fifoname, mode_t mode)
{
    int fd = open(fifoname, mode);
    if (fd == -1)
    {
        perror(fifoname);
        exit(1);
    }

    return fd;
}

void closeFifo(char *fifoname, int fd)
{
    close(fd);
    if (unlink(fifoname) < 0)
    {
        perror(fifoname);
        exit(1);
    }
}

FILE *openFile(char *filename)
{
    FILE *file = fopen(filename, "w");

    if (file != NULL)
        return file;
    else
    {
        perror(filename);
        exit(1);
    }
}

void writeFile(char *txt, FILE *file)
{
    if (file != NULL)
        fprintf(file, "%s", txt);
}

void closeFile(char *filename, FILE *file)
{
    if (fclose(file) != 0)
        perror(filename);
}

void installAlarm(void (*handler)(int), int time)
{
    struct sigaction action;
    sigset_t sigmask;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGALRM, &action, NULL);

    sigfillset(&sigmask);
    sigdelset(&sigmask, SIGALRM);
    sigdelset(&sigmask, SIGINT);

    alarm(time);

    sigsuspend(&sigmask);
}

const char *getError(int error)
{
    char *err = (char *) malloc(3 * sizeof(char));
    switch (error)
    {
    case -1:
        strcpy(err, "MAX");
        break;
    case -2:
        strcpy(err, "NST");
        break;
    case -3:
        strcpy(err, "IID");
        break;
    case -4:
        strcpy(err, "ERR");
        break;
    case -5:
        strcpy(err, "NAV");
        break;
    case -6:
        strcpy(err, "FUL");
        break;
    default:
        return NULL;
    }

    return err;
}