#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define SLAVE_CANT_PORCENTAGE 10
#define FILES_PER_PROCESS 2
#define max(x, y) ((x) > (y) ? (x) : (y))

int initializeReadFdSet(fd_set *set, int fds[][2], int length);

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    return 0;
    int file_qty = arc - 1;
    //CALCULATE SLAVE
    int slave_qty = ceil(ceil(file_qty * SLAVE_CANT_PORCENTAGE / 100) / FILES_PER_PROCESS);
    printf("slave_cant %d\n", slave_qty);

    //CREATE READ AND WRITE PIPES
    int write_fds[slave_qty][2];
    int read_fds[slave_qty][2];
    for (int i = 0; i < slave_qty; i++)
    {
        if (pipe(read_fds[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        close(read_fds[i][1]);
        if (pipe(write_fds[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        close(read_fds[i][0]);
    }

    //PROCESS FILES
    int processed_files = 0;
    fd_set writefds;
    int ready;
    int ndfs = -1;
    while (processed_files < file_qty)
    {
        //INITIALIZE FD SET FOR SELECT
        ndfs = initializeReadFdSet(&writefds, read_fds, slave_qty);

        //CREATE SLAVES

        //WAIT FOR RESULTS
        ready = select(ndfs, NULL, &writefds, NULL, NULL);

        if (ready == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }

        //READ RESULTS
    }
}

int initializeReadFdSet(fd_set *set, int fds[][2], int length)
{
    FD_ZERO(set);
    int ndfs = -1;

    for (size_t i = 0; i < length; i++)
    {
        int fd = fds[i][0];
        FD_SET(fd, set);
        ndfs = max(ndfs, fd);
    }
    return ndfs + 1;
}
