#include "master.h"

#define SLAVE_CANT_PORCENTAGE 10
#define FILES_PER_PROCESS 1
#define RESULT_SIZE 128
#define ERROR -1
#define max(x, y) ((x) > (y) ? (x) : (y))

int main(int argc, char *argv[])
{

    //SHARED-MEMORY 
    void * shm_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(shm_ptr);

    //----------------------------------------------------------

    //IPC MASTER-SLAVE
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int file_qty = argc - 1;
    printf("FILE CANT: %d\n", file_qty);
    //CALCULATE SLAVE
    int slave_qty = (int)ceil(ceil((double)(file_qty * SLAVE_CANT_PORCENTAGE) / 100) / FILES_PER_PROCESS);

    //CREATE READ AND WRITE PIPES
    int write_to_slave_fds[slave_qty][2];
    int read_from_slave_fds[slave_qty][2];

    //CREATE SLAVES
    int pid;
    for (int i = 0; i < slave_qty; i++)
    {
        if (pipe(read_from_slave_fds[i]) == ERROR)
        {
            perror("read from slave pipe()");
            exit(EXIT_FAILURE);
        }
        if (pipe(write_to_slave_fds[i]) == ERROR)
        {
            perror("write to slave pipe()");
            exit(EXIT_FAILURE);
        }

        if ((pid = fork()) == ERROR)
        {
            perror("fork()");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            //SLAVE CODE
            if (close(0) == ERROR)
            {
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if (dup(write_to_slave_fds[i][0]) == ERROR)
            {
                perror("dup()");
                exit(EXIT_FAILURE);
            }
            if (close(1) == ERROR)
            {
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if (dup(read_from_slave_fds[i][1]) == ERROR)
            {
                perror("dup()");
                exit(EXIT_FAILURE);
            }

            if (close(write_to_slave_fds[i][1]) == ERROR)
            {
                perror("close()");
                exit(EXIT_FAILURE);
            }
            if (close(read_from_slave_fds[i][0]) == ERROR)
            {
                perror("close()");
                exit(EXIT_FAILURE);
            }

            char buff[2];
            if (sprintf(buff, "%d", i) == ERROR)
            {
                perror("sprintf()");
                exit(EXIT_FAILURE);
            };
            printf("slave:%d\n", getpid());
            if (execl("./slave", "./slave", buff, NULL) < 0)
            {
                perror("exec()");
                exit(EXIT_FAILURE);
            }
            printf("slave:%d\n", getpid());
        }
        //PARENT CODE
        if (close(write_to_slave_fds[i][0]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
        if (close(read_from_slave_fds[i][1]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
    }

    //PROCESS FILES
    int sent_files = 0;
    int processed_files = 0;
    fd_set read_fds_set;
    int ready;
    int ndfs = -1;
    char **filePaths = argv + 1;

    //FIRST FILE DISPATCH
    for (int i = 0; i < slave_qty && sent_files < file_qty; i++)
    {
        for (int j = 0; j < FILES_PER_PROCESS; j++)
        {
            char *file = filePaths[sent_files++];

            if (file != NULL)
            {
                size_t len = (size_t)strlen(file);
                char buff[len + 1];
                strcpy(buff, file);
                buff[len] = '\0';

                printf("%d.ENVIO EL FILE %s AL SLAVE N°%d\n", sent_files, file, i);

                if (write(write_to_slave_fds[i][1], file, len + 1) == ERROR)
                {
                    perror("write() to slave");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    //PROCESS FILES
    while (processed_files < file_qty)
    {
        //INITIALIZE FD SET FOR SELECT
        // ndfs = initializeReadFdSet(&read_fds_set, read_from_slave_fds, slave_qty);
        FD_ZERO(&read_fds_set);
        for (size_t i = 0; i < slave_qty; i++)
        {
            int fd = read_from_slave_fds[i][0];

            FD_SET(fd, &read_fds_set);
            ndfs = max(ndfs, fd);
        }

        //WAIT FOR RESULTS
        ready = select(ndfs + 1, &read_fds_set, NULL, NULL, NULL);

        if (ready == ERROR)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }

        //CHECK ALL RESULTS
        for (int i = 0; i < slave_qty; i++)
        {
            int fd = read_from_slave_fds[i][0];
            if (FD_ISSET(fd, &read_fds_set))
            {
                printf("fd ready: %d\n", fd);
                processed_files++;
                //RESULT FROM SLAVE i IS READY TO BE READ
                char result[RESULT_SIZE];

                int count;
                if ((count = read(fd, result, RESULT_SIZE)) == ERROR)
                {
                    perror("read() results");
                    exit(EXIT_FAILURE);
                }
                printf("result: %s\n", result);
                if (sent_files < file_qty)
                {
                    char *file = filePaths[sent_files++];

                    if (file != NULL)
                    {
                        size_t len = (size_t)strlen(file);
                        char buff[len + 1];
                        strcpy(buff, file);
                        buff[len] = '\0';

                        printf("%d.ENVIO EL FILE %s AL SLAVE N°%d\n", sent_files, file, i);

                        if (write(write_to_slave_fds[i][1], file, len + 1) == ERROR)
                        {
                            perror("write() to slave");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
        //END PROCESS FILES
    }

    //CLOSE PIPES
    for (int i = 0; i < slave_qty; i++)
    {
        if (close(write_to_slave_fds[i][1]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
        if (close(read_from_slave_fds[i][0]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
    }

    //FOR SHM 
    //send_more_files(total_files_number, pipes, shm_ptr, mem_info, files);
    //shutdown_shm(mem_info, shm_ptr) --> podriamos cerrar pipes con esto tmb
    return 0;
}
