#ifndef _SLAVE_H_
#define _SLAVE_H_
#include "utils.h"

void read_files();
void process_file(char *file_path, int *file_number, pid_t pid);
void call_minisat(char *file_path, int *file_number, char *output, pid_t pid);
void send_result(char *output);
#endif