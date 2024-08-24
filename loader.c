/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  FILE* objFile;
  int header[2];
  int addr[2];
  int num[2];
  int instr[2];
  int head;
  int address;
  int length;
  unsigned int c;
  int i = 0;
  int symbol;

  //open object file to read
  objFile = fopen(filename, "rb");

  //if fopen returns null pointer, file was not opened. Return error value. 
  if (objFile == NULL) {
    printf("obj file error\n");
    return 2;
  }

  //while file has more data
  while (!feof(objFile)) {
    i = 0;
    length = 0;
    address = 0;
    head = 0;

    //get header info
    c = fgetc(objFile);
    header[0] = c;
    c = fgetc(objFile);
    header[1] = c;
    head = (int)header[0]*16*16+ header[1];
    //printf("%X \n", head);
    //printf("PSR: %d\n", CPU->PSR);
    
    //if header is c3b7, skip through address and next <length> characters
    if (head == 50103) {
        c = fgetc(objFile);
      addr[0] = c;
      c = fgetc(objFile);
      addr[1] = c;
      address = (int)addr[0]*16*16 + addr[1];
      //printf("%X ", address);

      c = fgetc(objFile);
      num[0] = c;
      c = fgetc(objFile);
      num[1] = c;
      length = (int)num[0]*16*16 +num[1];
      //printf("%d\n", length);
      while ( i < length) {
        c = fgetc(objFile);
        i++;
      }
    //if header is F17E, skip through <length> characters
    } else if (head == 61822) {
      c = fgetc(objFile);
      num[0] = c;
      c = fgetc(objFile);
      num[1] = c;
      length = (int)num[0]*16*16 +num[1];
      //printf("%d\n", length);
      while ( i < length) {
        //printf("%X ", head);
        c = fgetc(objFile);
        i++;
      }
    //if head is 715E, skip through address, line, file index
    } else if (head == 29022) {
      //address
      c = fgetc(objFile);
      addr[0] = c;
      c = fgetc(objFile);
      addr[1] = c;
      address = (int)addr[0]*16*16 + addr[1];
      //line
      c = fgetc(objFile);
      num[0] = c;
      c = fgetc(objFile);
      num[1] = c;

      //file index
      c = fgetc(objFile);
      num[0] = c;
      c = fgetc(objFile);
      num[1] = c;
    }
    //otherwise header is CADE or DADA, proceed to change machine state memory
    else {
      c = fgetc(objFile);
      addr[0] = c;
      c = fgetc(objFile);
      addr[1] = c;
      address = (int)addr[0]*16*16 + addr[1];
      //printf("%X ", address);

      c = fgetc(objFile);
      num[0] = c;
      c = fgetc(objFile);
      num[1] = c;
      length = (int)num[0]*16*16 +num[1];
      //printf("%d\n", length);

      //if address  > 7FFF  && PSR[15] == 0 , quit code
      if (address > 32767 && CPU->PSR <= 32767) {
          printf("Invalid permission");
          fclose(objFile);
          return 2;
      }

      //write to memory
      while(i < length) {
        if (feof(objFile)) {
            break;
        }
        c = fgetc(objFile);
        instr[0] = c;

        if (feof(objFile)) {
          break;
        }
        c = fgetc(objFile);
        instr[1] = c;

        //instr[0] = 5b, instr[1] = 42, 5b00 + 42 = 5b42
        CPU->memory[address] = (int)instr[0]*16*16 + (int)instr[1];
          
        address++;
        i++;
      }
    }
  }
  
  fclose(objFile);
  return 0;
}
