#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){

    if(argc == 1){
        char response[5];
        read(STDIN_FILENO, response, 5);
        printf("Hola soy el view y lo que me pasaron por master es %d\n", atoi(response));
    } else if (argc == 2){
        printf("Hola soy el view y lo que me llego de parametro es %d\n", atoi(argv[1]));
    }else
    {
        printf("Error");
        return -1;
    }
    
    
    
}