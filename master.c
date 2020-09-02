#include "master.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //INIT SHARED MEMORY
    //void *shm_ptr = create_shared_memory();
    //shm_info mem_info = initialize_shared_memory(shm_ptr);

    //FILE PATHS POINTER
    char **file_paths = argv + 1;

    //FILES QUANTITY
    int file_qty = argc - 1;
    printf("FILE CANT: %d\n", file_qty);

    //CALCULATE SLAVE QUANTITY
    int slave_qty = (int)ceil(ceil((double)(file_qty * SLAVE_CANT_PORCENTAGE) / 100) / FILES_PER_PROCESS);

    //CREATE READ AND WRITE PIPES
    int write_to_slave_fds[slave_qty][2];
    int read_from_slave_fds[slave_qty][2];

    //CREATE SLAVES AND INITIALIZE PIPES
    create_slaves(slave_qty, read_from_slave_fds, write_to_slave_fds);

    //SEND FILES TO SLAVES
    send_files_to_slaves(file_paths, read_from_slave_fds, write_to_slave_fds, slave_qty, file_qty);

    //CLOSE PIPES
    close_master_pipes(slave_qty, read_from_slave_fds, write_to_slave_fds);
    //finish_program(mem_info, shm_ptr); //si no incluimos lo de pipes despues le cambio 'program' a 'shm'
    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////
void close_master_pipes(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
{
    for (int i = 0; i < slave_qty; i++)
    {
        if (close(write_to_slave_fds[i][WRITE_PIPE]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
        if (close(read_from_slave_fds[i][READ_PIPE]) == ERROR)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
    }
}

void send_files_to_slaves(char **file_paths, int read_from_slave_fds[][2], int write_to_slave_fds[][2], int slave_qty, int file_qty)
{
    //PROCESS FILES
    int sent_files = 0;
    int processed_files = 0;
    fd_set read_fds_set;
    int ready;
    int ndfs = -1;

    //FIRST FILE DISPATCH
    for (int i = 0; i < slave_qty && sent_files < file_qty; i++)
    {
        for (int j = 0; j < FILES_PER_PROCESS; j++)
        {
            dispatch_file(write_to_slave_fds, i, file_paths, &sent_files);
        }
    }
    //PROCESS FILES
    while (processed_files < file_qty)
    {
        //INITIALIZE FD SET FOR SELECT
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
        printf("READY SELECT\n");
        //CHECK ALL RESULTS
        //struct Node *head = NULL;
        for (int i = 0; i < slave_qty; i++)
        {
            int fd = read_from_slave_fds[i][0];
            if (FD_ISSET(fd, &read_fds_set))
            {
                printf("fd ready: %d\n", fd);
                processed_files++;
                //RESULT FROM SLAVE i IS READY TO BE READ
                char result[RESULT_MAX_SIZE];

                //READ RESULT
                int count;
                if ((count = read(fd, result, RESULT_MAX_SIZE)) == ERROR)
                {
                    perror("read() results");
                    exit(EXIT_FAILURE);
                }

                result[count] = '\0';

                printf("%s\n", result);

                if (sent_files < file_qty)
                {
                    dispatch_file(write_to_slave_fds, i, file_paths, &sent_files);
                }
            }
        }

        sleep(2); //END PROCESS FILES
    }
}

void create_slaves(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
{

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
            config_slave_pipes(i, read_from_slave_fds, write_to_slave_fds);

            if (execl("./slave", "./slave", NULL) < 0)
            {
                perror("exec()");
                exit(EXIT_FAILURE);
            }
        }
        //PARENT CODE
        config_master_pipes(i, read_from_slave_fds, write_to_slave_fds);
    }
}

void config_master_pipes(int i, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
{
    if (close(write_to_slave_fds[i][READ_PIPE]) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }
    if (close(read_from_slave_fds[i][WRITE_PIPE]) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}

void config_slave_pipes(int i, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
{
    //CLOSE SLAVE STANDARD INPUT FILE DESCRIPTOR
    if (close(STDIN) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }

    if (dup(write_to_slave_fds[i][READ_PIPE]) == ERROR)
    {
        perror("dup()");
        exit(EXIT_FAILURE);
    }
    if (close(STDOUT) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }
    if (dup(read_from_slave_fds[i][WRITE_PIPE]) == ERROR)
    {
        perror("dup()");
        exit(EXIT_FAILURE);
    }

    if (close(write_to_slave_fds[i][WRITE_PIPE]) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }
    if (close(read_from_slave_fds[i][READ_PIPE]) == ERROR)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}

void dispatch_file(int write_to_slave_fds[][2], int write_index, char **file_paths, int *sent_files)
{
    char *file = file_paths[(*sent_files)++];

    if (file != NULL)
    {

        size_t len = (size_t)strlen(file);

        printf("%d.ENVIO EL FILE %s AL SLAVE N°%d\n", *sent_files, file, write_index);

        if (write(write_to_slave_fds[write_index][1], file, len) == ERROR)
        {
            perror("write() to slave");
            exit(EXIT_FAILURE);
        }
    }
}
//parte shm - cambiar en base a lo hablado
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
        //clear_shared_memory(shm_ptr, mem_info);
        exit(EXIT_FAILURE);
    }
}

void finish_program(shm_info mem_info, void * shm_ptr){
    mem_info->has_finished = 1;
    //metemos cerrado de pipes y frees necesarios aca?
    //clear_shared_memory(shm_ptr, mem_info);
}
