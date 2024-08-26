/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv)
{
    printf("trace 2\n");
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

    //initialize CPU
    Reset(CPU);

    txt = argv[1];
    txt_file = fopen(txt, "wb");
    if (txt_file == NULL) {
        printf("txt file error\n");
        return 1;
    }
    
    //recieve obj files as arguments to command line
    i = 2;
    while (i < argc) {
        filename = argv[i];
        result = ReadObjectFile(filename, CPU);
        i++;
    }
    
    //after all data from .obj files is in memory, update CPU with new info while program counter is not out of range of data space
    int w = 0;
    while (1) {
      //PC is at end of memory
      if (CPU->PC == 0x80FF) {
        break;
      }

      result = UpdateMachineState(CPU, txt_file);

      printf("PC : %04X\n", CPU->PC);

      if (result == 3) {
        printf("error in UpdateMachineState");
        break;
      }
      w++;
    }

    fclose(txt_file);
    return 0;
}
