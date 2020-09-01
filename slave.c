#include "slave.h"

int main(int argc, char *argv[])
{
    read_files();
    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////

void read_files()
{
    char file_path[FILE_PATH_MAX_SIZE];
    int count;

    while ((count = read(STDIN, file_path, FILE_PATH_MAX_SIZE)) > 0)
    {
        if (count == -1)
        {
            perror("read()");
            exit(EXIT_FAILURE);
        }
        file_path[count] = '\0';
        process_file(file_path);

        // char buff[RESULT_MAX_SIZE];
        // //sprintf(buff, "%s length: %d\n", file_path, count);

        // if (write(1, buff, strlen(buff)) == -1)
        // {
        //     perror("write()");
        //     exit(EXIT_FAILURE);
        // }
    }
}

void process_file(char *file_path)
{
    char output[RESULT_MAX_SIZE + FILE_PATH_MAX_SIZE + 3];
    call_minisat(file_path, output);
    send_result(output);
}

void call_minisat(char *file_path, char *output)
{
    int file_len = strlen(file_path);
    char command[file_len + 85];

    sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", file_path);

    FILE *result = popen(command, "r");

    if (result == NULL)
    {
        perror("popen()");
        exit(EXIT_FAILURE);
    }
    char c;

    sprintf(output, "%s:\n", file_path);
    int i = strlen(output);
    do
    {
        c = fgetc(result);

        if (feof(result))
        {

            output[i] = '\0';
            break;
        }
        output[i++] = c;
    } while (i < RESULT_MAX_SIZE);
}

void send_result(char *output)
{
    if (output == NULL)
    {
        perror("send_result()");
        exit(EXIT_FAILURE);
    }
    int length = strlen(output);
    if (write(STDOUT, output, length + 1) == -1)
    {
        perror("write()");
        exit(EXIT_FAILURE);
    }
}
