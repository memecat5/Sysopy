#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

// windows....
int getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            printf("bad alloc, czy cos\n");
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}


int main(int argc, char* argv[]){
    // Paths for input and output directories
    char * input_directory_path;
    char * output_directory_path;

    // Program accepts 2 arguments - paths for input and output directories
    // !!! remember about '/' at the end !!!
    // If no arguments are given, defaults are used
    if(argc == 1){
        // Default paths
        input_directory_path = "art/";
        output_directory_path = "out/";
    } else if(argc == 3){
        input_directory_path = argv[1];
        output_directory_path = argv[2];
    } else{
        printf("Wrong argument count!\n");
        return -1;
    }

    mkdir(output_directory_path);

    // Open directory with input text files
    DIR* input_directory = opendir(input_directory_path);
    if(input_directory == NULL){
        printf("Error while reading directory!");
        return -1;
    }

    // Get first file in the directory
    struct dirent* input_dir_file = readdir(input_directory);
    if(input_dir_file == NULL){
        printf("File not read properly!");
        return -1;
    }

    FILE* input_file;
    FILE* output_file;

    char* line = NULL;
    size_t read_limit = 0;

    char * reversed_line = NULL;

    size_t line_length = 0;


    while(input_dir_file != NULL){
        // Create path to the file from cwd
        char * current_file_path = malloc(64);
        strcpy(current_file_path, input_directory_path);
        strcat(current_file_path, input_dir_file->d_name);
        //pritnf(input_dir_file->d_type);
        // Check if current file is a text file
        if(strcmp(input_dir_file->d_name + strlen(input_dir_file->d_name) - 4, ".txt") == 0){
            // Open file for reading
            input_file = fopen(current_file_path, "r");
            if(input_file == NULL){
                printf("File %s didn't open properly!\n", input_dir_file->d_name);
            } else {
                // Create path to output file (output-dir/out_ + input-file-name)
                char * output_file_path = malloc(64);
                strcpy(output_file_path, output_directory_path);
                strcat(output_file_path, "out_");
                strcat(output_file_path, input_dir_file->d_name);
                
                // Open output file for writing or create a new one
                output_file = fopen(output_file_path, "w+");

                while(getline(&line, &read_limit, input_file) > 0){

                    line_length = strlen(line);
                    reversed_line = malloc(sizeof(char) * (line_length));

                    //Reversing line without '\n', it still has to go at the end
                    for(int i = 0; i < (int) line_length - 1; i++){
                        reversed_line[i] = (char) line[line_length - 2 - i];
                    }
                    reversed_line[line_length - 1] = '\n';
                    
                    // Write reversed line
                    fwrite(reversed_line, sizeof(char), line_length, output_file);

                    // Freeing memory for lines
                    // I could allocate and free outside of the loop but that would require assuming
                    // line lenght is limited, which is not specified in the task
                    free(line);
                    line = NULL;
                    free(reversed_line);
                    reversed_line = NULL;
                }

                // Close files
                fclose(input_file);
                fclose(output_file);

                free(output_file_path);
                output_file_path = NULL;
            }
        }   
        

        free(current_file_path);
        current_file_path = NULL;
        
        // Select next file from input directory
        input_dir_file = readdir(input_directory);
    }

    closedir(input_directory);
}