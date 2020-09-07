#include "utils.h"

void *mapping_shm(void *addr, int prot, int flags, int fd, off_t offset)
{
    void *ptr_shm = mmap(addr, SHM_MAX_SIZE, prot, flags, fd, offset);
    if (ptr_shm == MAP_FAILED)
    {
        perror("mapping_shm()");
        if (shm_unlink(SHM_NAME) == ERROR)
        {
            perror("shm_unlink()");
        };
        exit(EXIT_FAILURE);
    }
    return ptr_shm;
}