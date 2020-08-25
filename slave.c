#include <stdio.h>

int main(int argc, char* argv[]) {
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int linecount = 0;
    while ((linelen = getline(&line, &linecap, stdin)) > 0) {
        linecount++;
        printf("Line %d: ", linecount);
        fwrite(line, linelen, 1, stdout);
    }
    return 0;
}
