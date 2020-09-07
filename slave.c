// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "slave.h"

int main(int argc, char *argv[])
{
    read_files();

    return 0;
}

///////////////////////////////////////FUNCTIONS IMPLEMENTATIONS/////////////////////////////////////////////////////////////

//read_files: lee los path de los archivos para resolverlos
void read_files()
{
    int read_size = INITIAL_FILE_DISPATCH_QUANTITY * FILE_PATH_MAX_SIZE;
    char file_paths[read_size];
    int count;
    int file_number = 0;
    while ((count = read(STDIN, file_paths, read_size)) > 0)
    {
        if (count == ERROR)
        {
            perror("read()");
            exit(EXIT_FAILURE);
        }
        file_paths[count] = '@';

        char file_path[FILE_PATH_MAX_SIZE];
        int i;
        int j = 0;

        //PARSE
        for (i = 1; i <= count; i++)
        {
            char c = file_paths[i];
            if (c == '@')
            {

                file_path[j] = '\0';
                file_number++;

                process_file(file_path, &file_number);

                j = 0;
            }
            else
            {
                file_path[j++] = c;
            }
        }
    }
}

//process_file: procesa los archivos
void process_file(char *file_path, int *file_number)
{

    char output[RESULT_MAX_SIZE + FILE_PATH_MAX_SIZE + 4];
    call_minisat(file_path, file_number, output);
    send_result(output);
}

//call_minisat: resuelve el archivo con minisat
void call_minisat(char *file_path, int *file_number, char *output)
{
    int file_len = strlen(file_path);
    char command[file_len + 85];
    int output_size = RESULT_MAX_SIZE + FILE_PATH_MAX_SIZE + 4;
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

    if (sprintf(output, "%d@%s:\n", *file_number, file_path) < 0)
    {
        perror("sprintf()");
        exit(EXIT_FAILURE);
    };
    int i = strlen(output);
    do
    {
        char c = fgetc(result);

        if (feof(result))
        {
            output[i] = '\0';
            break;
        }
        output[i++] = c;
    } while (i < output_size);
}

//send_result: envia el resultado a master
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
