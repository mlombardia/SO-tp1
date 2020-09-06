#include "master.h"

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //INIT SHARED MEMORY
    void *shm_ptr = create_shared_memory();
    shm_info mem_info = initialize_shared_memory(shm_ptr);

    //OPEN or CREATE file for results
    FILE *results = open_file();

    //SEND VIEW PARAMETERS TO STDOUT AND WAIT 2 SECONDS FOR VIEW TO APPEAR
    prepare_param_for_view(argc);
    sleep(2);

    //FILE PATHS POINTER
    char **file_paths = argv + 1;

    //FILES QUANTITY
    int file_qty = argc - 1;

    //CALCULATE SLAVE QUANTITY
    int slave_qty = (int)ceil(ceil((double)(file_qty * SLAVE_QTY_PORCENTAGE) / 100) / INITIAL_FILE_DISPATCH_QUANTITY);

    //CREATE READ AND WRITE PIPES
    int write_to_slave_fds[slave_qty][2];
    int read_from_slave_fds[slave_qty][2];

    create_slaves(slave_qty, read_from_slave_fds, write_to_slave_fds);

    send_files_to_slaves(results, file_paths, read_from_slave_fds, write_to_slave_fds, slave_qty, file_qty, shm_ptr, mem_info);

    close_pipes(slave_qty, read_from_slave_fds, write_to_slave_fds);

    close_file(results);

    clear_shared_memory(shm_ptr, mem_info);
    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////

//prepare_param_for_view: prepara y envía la cantidad de archivos por STDOUT para que el proceso vista pueda leerlo.
void prepare_param_for_view(int argc)
{
    char size_for_view[5];
    int len = sprintf(size_for_view, "%d\n", argc - 1);
    write(STDOUT_FILENO, size_for_view, len);
}

//close_pipes: cierra los pipes utilizados por los procesos esclavos
void close_pipes(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
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

//send_files_to_slaves: distribuye los archivos entre los esclavos, y recibe sus resultados,
//guardandolos en la shared memory y en el archivo results.txt
void send_files_to_slaves(FILE *results, char **file_paths, int read_from_slave_fds[][2], int write_to_slave_fds[][2], int slave_qty, int file_qty, void *shm_ptr, shm_info mem_info)
{
    int sent_files = 0;
    int processed_files = 0;
    fd_set read_fds_set;
    int ndfs = -1;

    //INITIAL FILE DISPATCH

    for (int i = 0; i < slave_qty && sent_files < file_qty; i++)
    {
        for (int j = 0; j < INITIAL_FILE_DISPATCH_QUANTITY; j++)
        {
            dispatch_file(write_to_slave_fds, i, file_paths, &sent_files);
        }
    }
    //PROCESS FILES
    while (processed_files < file_qty)
    {
        int ready;
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
            exit(EXIT_FAILURE);
        }
        //CHECK ALL RESULTS
        for (int i = 0; i < slave_qty; i++)
        {

            int fd = read_from_slave_fds[i][0];
            if (FD_ISSET(fd, &read_fds_set))
            {
                //RESULT FROM SLAVE i IS READY TO BE READ
                processed_files++;
                char result[RESULT_MAX_SIZE];

                int count;
                if ((count = read(fd, result, RESULT_MAX_SIZE)) == ERROR)
                {
                    perror("read() results");
                    exit(EXIT_FAILURE);
                }
                result[count] = '\0';
                int slave_file_count = 0;

                //GET THE NUMBER OF FILES PROCCESSED BY THE SLAVE
                if (sscanf(result, "%d@", &slave_file_count) < 0)
                {
                    perror("sprintf()");
                    exit(EXIT_FAILURE);
                }

                int digits = digit_count(slave_file_count);
                write_result_to_shm(shm_ptr, mem_info, result + digits + 1);
                append_file(results, result + digits + 1);

                if (sent_files < file_qty && slave_file_count >= INITIAL_FILE_DISPATCH_QUANTITY)
                {
                    //IF SLAVE FINISHED PROCCESSING INITIAL FILES=>SEND 1 MORE FILE
                    dispatch_file(write_to_slave_fds, i, file_paths, &sent_files);
                }
            }
        } //END PROCESS FILES
    }
}

//append_file: escribe sobre el archivo apuntado por file, la cadena apuntada por s
void append_file(FILE *file, char *s)
{
    if (fprintf(file, "%s\n", s) < 0)
    {
        perror("fprintf()");
        exit(EXIT_FAILURE);
    };
}
//open_file: abre el archivo results.txt y si no existe, lo crea
FILE *open_file()
{
    FILE *results;

    if ((results = fopen("results.txt", "a")) == NULL)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
    return results;
}
//close_file: cierra el archivo apuntado por file
void close_file(FILE *file)
{
    if (fclose(file) == EOF)
    {
        perror("fclose()");
        exit(EXIT_FAILURE);
    }
}

//create_slaves: crea los procesos esclavos y crea los pipes
void create_slaves(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2])
{

    for (int i = 0; i < slave_qty; i++)
    {
        int pid;
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

//config_master_pipes: cierra los pipes que el proceso master no utiliza
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

//config_slave_pipes: configura los pipes de los esclavos
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

//dispatch_file: entrega un archivo al esclavo que le corresponda el write_index
void dispatch_file(int write_to_slave_fds[][2], int write_index, char **file_paths, int *sent_files)
{
    char *file = file_paths[(*sent_files)++];

    if (file != NULL)
    {

        size_t len = (size_t)strlen(file);
        char aux[len + 2];
        if (sprintf(aux, "@%s", file) < 0)
        {
            perror("sprintf()");
            exit(EXIT_FAILURE);
        }

        if (write(write_to_slave_fds[write_index][1], aux, len + 1) == ERROR)
        {
            perror("write()");
            exit(EXIT_FAILURE);
        }
    }
}

//create_shared_memory: crea la memoria compartida y le asigna un tamaño
void *create_shared_memory()
{
    void *shm_ptr = NULL;
    int shmid = 0;
    shmid = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
    if (shmid == ERROR)
    {
        perror("smh_open/(");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shmid, SHM_MAX_SIZE) == -1)
    {
        perror("ftruncate()");
        if (shm_unlink(SHM_NAME) == ERROR)
        {
            perror("shm_unlink()");
        };
        exit(EXIT_FAILURE);
    }

    if ((shm_ptr = (void *)mmap(NULL, SHM_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0)) == MAP_FAILED)
    {
        perror("mmap()");
        if (shm_unlink(SHM_NAME) == ERROR)
        {
            perror("shm_unlink()");
        };
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    return shm_ptr;
}
//initialize_shared_memory: inicializa los valores en la estructura compartida
//shm_info y los semáforos dentro de esta
shm_info initialize_shared_memory(void *shm_ptr)
{
    t_shm_info shm_info;
    //apunto a donde arranca
    shm_info.offset = sizeof(t_shm_info);
    shm_info.has_finished = 0;
    shm_info.count = 0;

    if (sem_init(&shm_info.semaphore, 1, 1) < 0)
    {
        perror("sem_init()");
        if (shm_unlink(SHM_NAME) == ERROR)
        {
            perror("shm_unlink()");
        };
        exit(EXIT_FAILURE);
    }

    if (sem_init(&shm_info.empty, 1, 0) < 0)
    {
        perror("sem_init()");
        if (shm_unlink(SHM_NAME) == ERROR)
        {
            perror("shm_unlink()");
        };
        exit(EXIT_FAILURE);
    }
    memcpy(shm_ptr, &shm_info, sizeof(t_shm_info));
    return shm_ptr;
}

//clear_shared_memory: libera la memoria compartida y destruye los semáforos
void clear_shared_memory(void *shm_ptr, shm_info mem_info)
{
    if (sem_destroy(&mem_info->semaphore) == ERROR)
    {
        perror("sem_destroy()");
        exit(EXIT_FAILURE);
    };
    if (sem_destroy(&mem_info->semaphore) == ERROR)
    {
        perror("sem_destroy()");
        exit(EXIT_FAILURE);
    };
    if (munmap(shm_ptr, SHM_MAX_SIZE) == ERROR)
    {
        perror("munmap()");
        exit(EXIT_FAILURE);
    }
}

//write_result_to_shm: escribe el resultado de los esclavos en la memoria compartida
//teniendo en cuenta que nadie este escribiendo en ese momento
void write_result_to_shm(void *shm_ptr, shm_info mem_info, char *result)
{

    if (sem_wait(&mem_info->semaphore) < 0)
    {
        perror("sem_wait()");
        clear_shared_memory(shm_ptr, mem_info);
        exit(EXIT_FAILURE);
    }
    else
    {
        strcpy((char *)shm_ptr + mem_info->offset, result);
        mem_info->offset += RESULT_MAX_INFO_TOTAL;
        (mem_info->count)++;

        if (sem_post(&mem_info->semaphore) < 0)
        {
            perror("sem_post()");
            clear_shared_memory(shm_ptr, mem_info);
            exit(EXIT_FAILURE);
        }
        if (mem_info->count == 1)
        {
            if (sem_post(&mem_info->empty) < 0)
            {
                perror("sem_post()");
                clear_shared_memory(shm_ptr, mem_info);
                exit(EXIT_FAILURE);
            }
        }
    }
}

//digit_count: cuenta los digitos de un numero
int digit_count(int n)
{
    if (n == 0)
    {
        return 1;
    }
    int count = 0;
    while (n > 0)
    {
        n /= 10;
        count++;
    }
    return count;
}