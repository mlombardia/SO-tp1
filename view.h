#ifndef _VIEW_H_
#define _VIEW_H_
#include "utils.h"

int check_total_files(int argc, char *argv[]);
void *connect_to_shm(shm_info *mem_info);
int open_shm(const char *name, int flag, mode_t mode);
void *mapping_shm(void *addr, int prot, int flags, int fd, off_t offset);
void print_results(void *ptr_shm, shm_info mem_info, int total_files);
void shm_disconnect(void *ptr_shm, shm_info mem_info);
#endif