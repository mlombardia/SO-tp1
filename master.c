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

    finish_program(mem_info, shm_ptr); //si no incluimos lo de pipes despues le cambio 'program' a 'shm'
    return 0;
}

//==============================IMPLEMENTATION===================================================
void * create_shared_memory(){
    void * shm_ptr = NULL;
    //creo la memoria compartida
    int shmid = 0;
    shmid = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
    if(shmid < 0){
        perror("smh_open");
        exit(EXIT_FAILURE);
    }
    //chequeo si estamos ok con el tema del tamaño de la memoria 
    if(ftruncate(shmid, SHM_MAX_SIZE) == -1){
        perror("can't truncate");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    //chequeo si se puede conectar
    if ((shm_ptr = (void *) mmap(NULL, SHM_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0)) == MAP_FAILED){
        perror("can't attach memory");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    return shm_ptr;
}

shm_info initialize_shared_memory(void * shm_ptr){
    t_shm_info shm_info;
    //apunto a donde arranca
    shm_info.offset = sizeof(t_shm_info);
    shm_info.has_finished = 0;

    if(sem_init(&shm_info.semaphore, 1, 0) < 0){
        perror("Error initializing semaphore");
        unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    memcpy(shm_ptr, &shm_info, sizeof(t_shm_info));
    return shm_ptr;
}

void clear_shared_memory(void * shm_ptr, shm_info mem_info){
    sem_destroy(&mem_info->semaphore);
    munmap(shm_ptr, SHM_MAX_SIZE);
    shm_unlink(SHM_NAME);
}

void write_result_to_shm(void * shm_ptr, shm_info mem_info, char * result){
    strcpy((char *) shm_ptr + mem_info->offset, result);
    mem_info->offset += RESULTS_INFO_SIZE;
    if (sem_post(&mem_info->semaphore) < 0){
        perror("Error in wait");
        clear_shared_memory(shm_ptr, mem_info);
        exit(EXIT_FAILURE);
    }
}

void finish_program(shm_info mem_info, void * shm_ptr){
    mem_info->has_finished = 1;
    //metemos cerrado de pipes y frees necesarios aca?
    clear_shared_memory(shm_ptr, mem_info);
}