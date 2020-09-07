// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "view.h"

int main(int argc, char *argv[])
{

    int total_files = check_total_files(argc, argv);

    shm_info mem_info = NULL;

    void *shm_ptr = connect_to_shm(&mem_info);

    print_results(shm_ptr, mem_info, total_files);

    shm_disconnect(shm_ptr, mem_info);

    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////

//check_total_files: chequea la cantidad de archivos 
//dependiendo el modo en el que se invoco (con o sin |)
int check_total_files(int argc, char *argv[])
{
    if (argc == 1)
    {
        char response[5];
        if (read(STDIN, response, 5) < 0)
        {
            perror("read()");
            exit(EXIT_FAILURE);
        };
        return atoi(response);
    }
    else if (argc == 2)
    {
        return atoi(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: %s <file_quantity>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

//connect_to_shm: conexion a la memoria
void *connect_to_shm(shm_info *mem_info)
{
    int fd_shm = open_shm(SHM_NAME, O_RDWR, S_IRWXU);
    void *ptr_shm = mapping_shm(NULL, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    *mem_info = (shm_info)ptr_shm;
    return ptr_shm;
}

//open_shm: intenta abrir la instancia de shared memory
int open_shm(const char *name, int flag, mode_t mode)
{
    int fd_shm = shm_open(name, flag, mode);
    if (fd_shm == ERROR)
    {

        perror("open_shm()");

        exit(EXIT_FAILURE);
    }
    return fd_shm;
}

//mapping_shm: intenta mapear en memoria el espacio
void *mapping_shm(void *addr, int prot, int flags, int fd, off_t offset)
{
    void *ptr_shm = mmap(addr, SHM_MAX_SIZE, prot, flags, fd, offset);
    if (ptr_shm == MAP_FAILED)
    {
        perror("mapping_shm()");
        exit(EXIT_FAILURE);
    }
    return ptr_shm;
}

//print_results: imprime los resultados desde la memoria
void print_results(void *ptr_shm, shm_info mem_info, int total_files)
{
    int current = 1;
    size_t offset = sizeof(t_shm_info);

    while (current <= total_files)
    {

        if (mem_info->count == 0)
        {
            if (sem_wait(&mem_info->empty) < 0)
            {
                perror("sem_wait()");

                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (sem_wait(&mem_info->semaphore) < 0)
            {
                perror("sem_wait()");

                exit(EXIT_FAILURE);
            }
            else
            {

                printf("%d) %s", current, (char *)ptr_shm + offset);
                putchar('\n');
                offset += RESULT_MAX_INFO_TOTAL;
                current++;
                (mem_info->count)--;
                if (sem_post(&mem_info->semaphore) < 0)
                {
                    perror("sem_post()");

                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

//shm_discconect: desconecta y libera la instancia de la memoria
void shm_disconnect(void *ptr_shm, shm_info mem_info)
{
    if (munmap(ptr_shm, SHM_MAX_SIZE) == ERROR)
    {
        perror("munmap()");
        exit(EXIT_FAILURE);
    }
    if (shm_unlink(SHM_NAME) == ERROR)
    {
        perror("shm_unlink()");
        exit(EXIT_FAILURE);
    };
}