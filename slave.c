#include "slave.h"

int main(int argc, char *argv[])
{
    read_files();
    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////

void read_files()
{
    char file_paths[FILES_PER_PROCESS * FILE_PATH_MAX_SIZE];
    int count;

    while ((count = read(STDIN, file_paths, FILES_PER_PROCESS * FILE_PATH_MAX_SIZE)) > 0)
    {
        if (count == -1)
        {
            perror("read()");
            exit(EXIT_FAILURE);
        }

        file_paths[count] = '\0';
        char file_path[FILE_PATH_MAX_SIZE];
        char c;
        for (int i = 1, j = 0; i <= count; i++)
        {
            c = file_paths[i];
            if (c == '@' || i == count)
            {

                file_path[j] = '\0';
                process_file(file_path);
                j = 0;
            }
            else
            {
                file_path[j++] = c;
            }
        }
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

    if (sprintf(command, "minisat %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", file_path) < 0)
    {
        perror("sprintf()");
        exit(EXIT_FAILURE);
    }

    FILE *result = popen(command, "r");

    if (result == NULL)
    {
        perror("popen()");
        exit(EXIT_FAILURE);
    }
    char c;

    if (sprintf(output, "%s:\n", file_path) < 0)
    {
        perror("sprintf()");
        exit(EXIT_FAILURE);
    };
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
