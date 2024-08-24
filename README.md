Simulate UPenn LC4 Processor. **Does not simulate I/O operations, can directly access memory simulating I/O

Input - Binary .objs file of instructions to execute

Output - Part 1: txt file with trace of modifications made to memory, Part 2: log of instructions executed.

Part 1:
Read .obj file into memory, then parse .obj files into data and code in memory.
Use makefile make trace to create .o files, Execute using command ./trace output_filename.txt first.obj second.obj third.obj

Output format 

address: (hexadecimal address) contents: (hexadecimal instruction)


Part 2:
Implement functions meant to simulate a cycle of an LC4 Processor. Execute code instructions stored in memory.
Executed using command 

Output Format

(Program Counter value) (Binary Instruction) (Register File Write Enable Value) (Target Register) (Data Written) (NZP Write Enable Value) (Data Written) (Data Memory Write Enable Value) (Target Register) (Data Written)
Use makefile make trace to create .o files, execute using command ./lc4 output_filename.txt first.obj second.obj third.obj
