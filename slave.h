#ifndef _SLAVE_H_
#define _SLAVE_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

void read_files();
void process_file(char *file_path, int *file_number);
void call_minisat(char *file_path, int *file_number, char *output);
void send_result(char *output);
#endif