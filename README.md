Simulate UPenn LC4 Processor. **Does not simulate I/O operations, can directly access memory simulating I/O

Input - Binary .objs file of instructions to execute
Output - Part 1: txt file with trace of modifications made to memory, Part 2: log of instructions executed.

Part 1:
Read .obj file into memory, then parse .obj files into data and code in memory.

Output format   
address: (hexadecimal address) contents: (hexadecimal instruction)   
Use makefile make trace to create .o files, execute using command ./trace output_filename.txt first.obj second.obj third.obj

Part 2:
Implement functions meant to simulate a cycle of an LC4 Processor. Execute code instructions stored in memory.

Output Format   
(Program Counter value) (Binary Instruction) (Register File Write Enable Value) (Target Register) (Data Written) (NZP Write Enable Value) (Data Written) (Data Memory Write Enable Value) (Target Register) (Data Written)   
Use makefile make trace to create .o files, execute using command ./lc4 output_filename.txt first.obj second.obj third.obj   

File:   
loader.c - Read in object file data to memory at given address.  
LC4.c - Functions to control elements of LC4 processor. Helper functions for basic logical, branch, arithmetic, comparative, jump, shift/mod operations        
trace1.c  - Execute part one, load data to memory with loader.c   
trace2.c - Execute part two, load data to memory with loader.c and simulate processer using LC4.c   

