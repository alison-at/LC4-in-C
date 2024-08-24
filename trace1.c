/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
    int i;
    int write = 0;
    //initialize MachineState
    MachineState myCPU;
    CPU = &myCPU;

    //get filename from commandline
    char* filename = NULL ;
    char* txt  = NULL;
    FILE* txt_file;
    int result;

    //zeros out every memory location
    for(i = 0; i < 65536; i++) {
        CPU->memory[i] = 0;
    }

    txt = argv[1];
    txt_file = fopen(txt, "wb");
    if (txt_file == NULL) {
        printf("txt file error\n");
        return 1;
    }

    i = 2;
    while (i < argc) {
        filename = argv[i];
        result = ReadObjectFile(filename, CPU);
        //free(filename);
        i++;
    }

    //change txt file

    for (i = 0; i < 65536; i++) {
        if (CPU->memory[i] != 0) {
        //write = fwrite(&(CPU->memory[i]), 2, 1, txt_file);
            fprintf(txt_file, "address: %05d contents: 0x%04X\n",  i, CPU->memory[i]);
        }
    }
    
    return 0;
}
