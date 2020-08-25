#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define FILE_PATH_SIZE 48

int main(int argc, char *argv[])
{
    char filePath[FILE_PATH_SIZE];
    int count;

    while ((count = read(0, filePath, FILE_PATH_SIZE)) > 0)
    {
        if (count == -1)
        {
            perror("read()");
            exit(EXIT_FAILURE);
        }
        int len = strlen(filePath);
        char buff[100];
        sprintf(buff, "%s length: %d\n", filePath, len);
        if (write(1, buff, 100) == -1)
        {
            perror("write()");
            exit(EXIT_FAILURE);
        }
    }
    // char *line = NULL;
    // size_t linecap = 0;
    // ssize_t linelen;
    // int linecount = 0;
    // while ((linelen = getline(&line, &linecap, stdin)) > 0) {
    //     linecount++;
    //     printf("Line %d: ", linecount);
    //     fwrite(line, linelen, 1, stdout);
    // }
    return 0;
}
