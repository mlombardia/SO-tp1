#include "view.h"

int main(int argc, char *argv[]){

    int total_files;
    if(argc == 1){
        char response[5];
        read(STDIN_FILENO, response, 5);
        total_files = atoi(response);
        printf("Hola soy el view y lo que me pasaron por master es %d\n", total_files);
    } else if (argc == 2){
        total_files = atoi(argv[1]);
        printf("Hola soy el view y lo que me llego de parametro es %d\n", total_files);
    }else
    {
        printf("Error");
        return -1;
    }

    shm_info mem_info = NULL;

    void * ptr_shm = connect_to_shm(&mem_info);

    print_results(ptr_shm, mem_info, total_files);

    shm_disconnect(ptr_shm, mem_info);
}

//===============================IMPLEMENTATION================================================
    
    void * connect_to_shm(shm_info *mem_info){
        int fd_shm = open_shm(SHM_NAME, O_RDWR, S_IRWXU);
        void *ptr_shm = mapping_shm(NULL, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm,0);
        *mem_info = (shm_info) ptr_shm;
        return ptr_shm;
    }

    int open_shm(const char *name, int flag, mode_t mode){
        int fd_shm = shm_open(name, flag, mode);
        if (fd_shm == -1){
            printf("shm_open error: ");
            printf("%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        return fd_shm;
    }

    void * mapping_shm(void *addr, int prot, int flags, int fd, off_t offset){
        void *ptr_shm = mmap(addr, SHM_MAX_SIZE, prot, flags, fd, offset);
        if(ptr_shm == (void *)-1){
            printf("Error\n");
            printf("%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        return ptr_shm;
    }

    void print_results(void *ptr_shm, shm_info mem_info, int total_files){
        int current=0;
        //int offset = sizeof(t_shm_info);
        while(current < total_files){ 
            printf("%s", (char *) ptr_shm + mem_info->offset);
            mem_info->offset += RESULT_MAX_INFO_TOTAL;
            //offset += RESULT_MAX_INFO_TOTAL;
            current++;
        }
    }
    
    void shm_disconnect(void *ptr_shm, shm_info mem_info){
        if(munmap(ptr_shm, SHM_MAX_SIZE) == -1){
            perror("munmap");
            exit(EXIT_FAILURE);
        }
    }