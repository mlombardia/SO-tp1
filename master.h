#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


#define SHM_NAME "/shm"
#define MAX_FILE_NAME 128 //provisorio
#define RESULT_LENGTH 384 //provisorio
#define SHM_MAX_FILES 1000 //provisorio
#define RESULTS_INFO_SIZE ((MAX_FILE_NAME + RESULT_LENGTH) * sizeof(char))
#define SHM_MAX_SIZE RESULTS_INFO_SIZE*SHM_MAX_FILES

typedef struct{
    //con el ptr manejo desde el principio, con offset al último elemento agregado voy yendo para adelante
    //agrego flag para ver si termino
    size_t offset, has_finished;
    sem_t semaphore; // shm = estacionamiento, le pongo el sem en la estructura
}t_shm_info;

typedef t_shm_info * shm_info;

void * create_shared_memory(); 
shm_info initialize_shared_memory(void * shm_ptr); 
void clear_shared_memory(void * shm_ptr, shm_info mem_info);
void write_result_to_shm(void * shm_ptr, shm_info mem_info, char * result);
void finish_program(shm_info mem_info, void * shm_ptr);