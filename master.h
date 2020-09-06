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
#include "utils.h"

#define SLAVE_QTY_PORCENTAGE 20

#define READ_PIPE 0
#define WRITE_PIPE 1
#define max(x, y) ((x) > (y) ? (x) : (y))

void prepare_param_for_view(int argc);
int digitCount(int n);
void close_master_pipes(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
void config_master_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void config_slave_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void send_files_to_slaves(FILE *results, char **file_paths, int read_from_slave_fds[][2], int write_to_slave_fds[][2], int slave_qty, int file_qty, void *shm_ptr, shm_info mem_info);
void create_slaves(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
void dispatch_file(int write_to_slave_fds[][2], int write_index, char **file_paths, int *sent_files);
void append_file(FILE *file, char *s);
FILE *open_file();
void close_file(FILE *file);
//SHARED-MEMORY
void *create_shared_memory();
shm_info initialize_shared_memory(void *shm_ptr);
void clear_shared_memory(void *shm_ptr, shm_info mem_info);
void write_result_to_shm(void *shm_ptr, shm_info mem_info, char *result);
#endif