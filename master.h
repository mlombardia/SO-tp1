#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <semaphore.h>

typedef struct{
    //con el ptr manejo desde el principio, con offset al último elemento agregado voy yendo para adelante
    size_t offset;
    sem_t semaphore; // shm = estacionamiento, le pongo el sem en la estructura
}t_shm_info;

typedef t_shm_info * shm_info;

void * create_shared_memory(); //impl
shm_info initialize_shared_memory(void * shm_ptr); //impl
void clear_shared_memory(void * shm_ptr, shm_info mem_info); //impl, necesaria para el shutdown