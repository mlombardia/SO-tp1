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
#define RESULT_MAX_SIZE 256
#define SLAVE_CANT_PORCENTAGE 10
#define FILES_PER_PROCESS 1
#define ERROR -1
#define STDOUT 1
#define STDIN 0
#define READ_PIPE 0
#define WRITE_PIPE 1
#define max(x, y) ((x) > (y) ? (x) : (y))

void close_master_pipes(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
void config_master_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void config_slave_pipes(int i, int write_to_slave_fds[][2], int read_from_slave_fds[][2]);
void send_files_to_slaves(char **file_paths, int read_from_slave_fds[][2], int write_to_slave_fds[][2], int slave_qty, int file_qty);
void create_slaves(int slave_qty, int read_from_slave_fds[][2], int write_to_slave_fds[][2]);
void dispatch_file(int write_to_slave_fds[][2], int write_index, char **file_paths, int *sent_files);
#endif