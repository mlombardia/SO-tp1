#ifndef _SLAVE_H_
#define _SLAVE_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define FILE_PATH_MAX_SIZE 128
#define INITIAL_FILE_DISPATCH_QUANTITY 3
#define RESULT_MAX_SIZE 384
#define STDOUT 1
#define STDIN 0
void read_files();
void process_file(char *file_path, int *file_number);
void call_minisat(char *file_path, int *file_number, char *output);
void send_result(char *output);
#endif