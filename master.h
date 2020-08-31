#ifndef _MASTER_H_
#define _MASTER_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define RESULT_MAX_SIZE 256
#define SLAVE_CANT_PORCENTAGE 10
#define FILES_PER_PROCESS 1
#define ERROR -1
#define STDOUT 1
#define STDIN 0
#define READ_PIPE 0
#define WRITE_PIPE 1
#define max(x, y) ((x) > (y) ? (x) : (y))
//FOR SHARED-MEMORY
#define SHM_NAME "/shm"
#define MAX_FILE_NAME 128 //provisorio
#define RESULT_LENGTH 384 //provisorio; no se si usar el RESULT_MAX_SIZE o usar uno nuevo
#define SHM_MAX_FILES 1000 //provisorio
#define RESULTS_INFO_SIZE ((MAX_FILE_NAME + RESULT_LENGTH) * sizeof(char))
#define SHM_MAX_SIZE RESULTS_INFO_SIZE*SHM_MAX_FILES

struct Node
{
    char result[RESULT_MAX_SIZE];
    time_t mod_time;
    struct Node *next;
} Node;

typedef struct{
    //con el ptr manejo desde el principio, con offset al último elemento agregado voy yendo para adelante
    //agrego flag para ver si termino
    size_t offset, has_finished;
    sem_t semaphore; // shm = estacionamiento, le pongo el sem en la estructura
}t_shm_info;

typedef t_shm_info * shm_info;

void close_master_pipes(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
void config_master_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void config_slave_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void send_files_to_slaves(char **file_paths, int read_from_slave_fds[][2], int write_to_slave_fds[][2], int slave_qty, int file_qty);
void create_slaves(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
int sortedInsert(struct Node **head, struct Node *node);
void dispatch_file(int write_to_slave_fds[][2], int write_index, char **file_paths, int *sent_files);
//SHARED-MEMORY
void * create_shared_memory(); 
shm_info initialize_shared_memory(void * shm_ptr); 
void clear_shared_memory(void * shm_ptr, shm_info mem_info);
void write_result_to_shm(void * shm_ptr, shm_info mem_info, char * result);
void finish_program(shm_info mem_info, void * shm_ptr);
#endif