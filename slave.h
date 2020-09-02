#ifndef _SLAVE_H_
#define _SLAVE_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define FILE_PATH_MAX_SIZE 128
#define FILES_PER_PROCESS 5
#define RESULT_MAX_SIZE 384
#define STDOUT 1
#define STDIN 0
void read_files();
void process_file(char *file_path);
void call_minisat(char *file_path, char *output);
void send_result(char *output);
#endif