#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "master.h"

void * connect_to_shm(shm_info * mem_info);
void mem_disconnect(void *ptr_shm, shm_info mem_info);