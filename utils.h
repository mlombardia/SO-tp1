#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/shm"
#define RESULT_MAX_SIZE 384
#define FILE_PATH_MAX_SIZE 128
#define RESULT_MAX_INFO_TOTAL (RESULT_MAX_SIZE + FILE_PATH_MAX_SIZE)
#define SHM_MAX_FILES 1000
#define SHM_MAX_SIZE RESULT_MAX_INFO_TOTAL *SHM_MAX_FILES
#define INITIAL_FILE_DISPATCH_QUANTITY 5
#define STDOUT 1
#define STDIN 0
#define ERROR -1
typedef struct
{
    //con el ptr manejo desde el principio, con offset al último elemento agregado voy yendo para adelante
    //agrego flag para ver si termino
    size_t offset, has_finished;
    int count;
    sem_t semaphore, empty; // shm = estacionamiento, le pongo el sem en la estructura
} t_shm_info;

typedef t_shm_info *shm_info;


void *mapping_shm(void *addr, int prot, int flags, int fd, off_t offset);
#endif