This is part of a larger project to simulate a UPenn LC4 Processor.    
**Does not simulate I/O operations, can directly access memory simulating I/O

Input - Binary .objs file of instructions to execute    
Output - Log of control signals in CPU after executing one datapath cycle

Output Format   
(Program Counter value) (Binary Instruction) (Register File Write Enable Value) (Target Register) (Data Written) (NZP Write Enable Value) (Data Written) (Data Memory Write Enable Value) (Target Register) (Data Written)   
  
Code Execution:   
LC4.h - Shared at the start of the project, contains information on Machine State struct. Specifices elements of processor to be simulated in this project. Only aspect of project I did not complete.   
trace2.c - Load data to memory with loader.c ReadObjectFile() and simulate processer using LC4.c UpdateMachineState() function.   
loader.c - Read in object file data to memory at given address. See PennSim_.obj_Format.png to understand how data from an object file is parsed into memory. For this project, only Code and Data sections of the object file were relevant and other sections were skipped.   
LC4.c - Functions to control elements of LC4 processor. Please focus on UpdateMachineState() to understand overall function of the file, helper functions can be ignored. Helper functions for basic logical, branch, arithmetic, comparative, jump, shift/mod operations. Reference LC4-ISA-Instructions.pdf to see how UpdateMachineState() decides what helper functions need to be called for each instruction.       

