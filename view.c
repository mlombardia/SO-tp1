#include "view.h"

int main(int argc, char *argv[])
{
    shm_info mem_info = NULL;
    void * ptr_shm = connect_to_shm(&mem_info);
    //aca vendria la parte de imprimir datos
    mem_disconnect(ptr_shm, mem_info);
    return 0;
}