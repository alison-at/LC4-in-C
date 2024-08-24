/*
 * LC4.c: Defines simulator functions for executing instructions
 */
#include "LC4.h"
#include <stdio.h>
#include <math.h>

#define INSN_OP(I) ((I) >> 12)

//shift right 9, AND with 0000111
#define INSN_11_9(I) (((I) >> 9) & 0x7)

#define INSN_15_12(I) (((I) >> 12) & 0xF)

//AND 0000 0001 1111 1111
#define INSN_12_0(I) ((I)&0x1FF)

//shift right 3, AND 0111
#define INSN_5_3(I) (((I)>>3) & 0x7)

#define INSN_8_6(I) (((I)>>6) & 0x7)

#define INSN_2_0(I) ((I)&0x7)

#define INSN_4_5(I) (((I) >> 4)&3)

//AND 0001 1111
#define INSN_4_0(I) ((I)&0x1F)

//AND 0011 1111
#define INSN_5_0(I) ((I)&0x3F)

//AND 0000 1111
#define INSN_3_0(I) ((I)&0xF)

//shift right 11, AND with 0001
#define INSN_11(I) (((I)>>11)& 0x1)

// AND 0111 1111 1111
#define INSN_10_0(I) ((I) & 0x7FF)

//shift right 4, AND 0011
#define INSN_5_4(I) (((I) >>4) & 0x3)

#define INSN_15(I) (((I)>>15)& 0x1)

//AND with 1111 1111
#define INSN_7_0(I) ((I)&0xFF)

//AND with 0111 1111
#define INSN_6_0(I) ((I)&0x7F)

//AND with 0001 1111 1111
#define INSN_8_0(I) ((I)&0x1FF)

#define INSN_8_7(I) (((I) >> 7) &0x3)

/*
 * Reset the machine state as Pennsim would do
 Look at code from class to fill out
 */
void Reset(MachineState* CPU)
{
    //initial PC Value
    CPU->PC = 0x8200;
    CPU->PSR = 0x8002;

    //zeros out every memory location
    for(int i = 0; i < 65536; i++) {
        CPU->memory[i] = 0;
    }

    //zeros out every register
    for(int i = 0; i < 8; i++) {
        CPU->R[i] = 0;
    }

    ClearSignals(CPU);
}

/*
 * Clear all of the control signals (set to 0)
   helper to reset
 */
void ClearSignals(MachineState* CPU)
{
  CPU->rsMux_CTL = 0;
  CPU->rdMux_CTL = 0;
  CPU->rtMux_CTL = 0;
  CPU->regFile_WE = 0;
  CPU->NZP_WE = 0;
  CPU->DATA_WE = 0;
  CPU->regInputVal = 0;
  CPU->dmemAddr = 0;
  CPU->dmemValue = 0;
}

/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output)
{
  unsigned short instr = 0;
  unsigned short shifter = 0;
  int idx;
  int binDigit = 0;
  fprintf(output, "%04X ", CPU->PC);

  //retrieve instruction at PC in memory
  instr = CPU->memory[CPU->PC];
  //translate hex to binary, write to file
  for(int i = 0; i < 16; i++) {
    idx = 15-i;
    shifter = instr >> idx;

    //if shifter is odd, binDigit is 1, if shifter is even, binDigit is 0. Odd means bit 0 is 1.
    binDigit = shifter%2;
    fprintf(output, "%d", binDigit);
  }

  //1 bit register WE
  fprintf(output, " %01X", CPU->regFile_WE);

  //handle reg WE cases: if 0, destination reg = 0
  if (CPU->regFile_WE == 0) {
    fprintf(output, " 0");
  } 
  //if rd_MUX_CTL = 0, instr[11-9] is destination reg. If 1, if rdMuxCTL = 1, is 7
  else {
    if (CPU->rdMux_CTL == 1) {
      fprintf(output, " 7");
    } else {
      fprintf(output, " %X", INSN_11_9(instr));
    }
  }

  fprintf(output, " %04X", CPU->regInputVal);

  if (CPU->NZP_WE == 0) {
    fprintf(output, " %X 0", CPU->NZP_WE);
  } else {
    fprintf(output, " %X %X", CPU->NZP_WE, CPU->NZPVal);
  }
 
  fprintf(output, " %X %04X %04X\n", CPU->DATA_WE, CPU->dmemAddr, CPU->dmemValue);
  return;
}

/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output)
{
  unsigned short instr = CPU->memory[CPU->PC];
  unsigned short opCode = INSN_15_12(instr);
  unsigned short rd;
  unsigned short rs;
  unsigned short rt;
  short sum;
  unsigned short int prevPC = CPU->PC;
  unsigned short int nextPC;
  short imm;
  unsigned short uimm;
  //printf("instruction %X opCode %X  \n" , instr, opCode);

  switch (opCode) {
    case 0x0:
      //branch instruction
      BranchOp(CPU, output);
      break;
    case 0x01: 
      //arith instruction
      ArithmeticOp(CPU, output);
      break;
    case 0x05:
      //logical instruction
      LogicalOp(CPU, output);
      break;
    case 0x06:
      //LDR instruction
      rd = INSN_11_9(instr);
      rs = INSN_8_6(instr);
      imm = (short)INSN_5_0(instr);

      //if bit 5 = 1, make imm negative
      if (imm >= 32) {
        imm = imm +0xFFC0;
      }
      
      //set control signals
      ClearSignals(CPU);
      CPU->NZP_WE = 1;
      CPU->regFile_WE = 1;
      CPU->dmemAddr = (short)CPU->R[rs] + imm;

      //check for permission to access OS data
      if (CPU->dmemAddr >= 0xA000 && CPU->PSR < 0x8000) {
        printf("Invalid permissions");
        return 3;
      }

      CPU->dmemValue = CPU->memory[CPU->dmemAddr];
      CPU->R[rd] = CPU->dmemValue;
      CPU->PC = CPU->PC +1; 
      CPU->regInputVal = CPU->R[rd];
      SetNZP(CPU, CPU->memory[CPU->dmemAddr]);
      
      break;
    case 0x07:
      //STR instruction
      imm = (short)INSN_5_0(instr);
      rt = INSN_11_9(instr);
      rs = INSN_8_6(instr);
      
      //if bit 5 = 1, make imm negative
      if (imm >= 0x0020) {
        imm = (imm) + 0xFFC0;
        
      }

      ClearSignals(CPU);
      CPU->DATA_WE = 1;

      CPU->dmemAddr = (short)CPU->R[rs] + imm;
      //check for permission to access OS data
      if (CPU->dmemAddr >= 0xA000 && CPU->PSR < 0x8000) {
        printf("Invalid permissions");
        return 3;
      }

      //printf("dmem %X PSR %04X\n", CPU->dmemAddr, CPU->PSR);
      CPU->memory[CPU->dmemAddr] = CPU->R[rt];
      CPU->dmemValue = CPU->R[rt];
      CPU->PC = (CPU->PC) +1;
      break;
    case 0x09:
      //const instruction
      rd = INSN_11_9(instr);
       
      imm = (short)INSN_8_0(instr);
      if (imm > 256) {
        imm = imm +0xFE00;
      }

      //set control signals
      ClearSignals(CPU);
      CPU->regFile_WE = 1;
      CPU->NZP_WE = 1;

      CPU->R[rd] = imm;
      CPU->PC = (CPU->PC) +1;
      CPU->regInputVal = imm;
      SetNZP(CPU, imm);
      break;
    case 0x0D:
      //hiconst instruction
      imm = (short)INSN_7_0(instr);
      rd = INSN_11_9(instr);

      //if bit 7 = 1, make imm negative
      if (imm >= 128) {
        imm = imm +0xFF00;
      }

      
      //set control signals
      ClearSignals(CPU);
      CPU->regFile_WE = 1;
      CPU->NZP_WE = 1;

      CPU->R[rd] = (CPU->R[rd] & 0xFF) | (imm <<8);
      CPU->PC = CPU->PC +1;
      CPU->regInputVal = CPU->R[rd];

      SetNZP(CPU, imm);
      break;
    case 0x02:
      //compare instruction
      ComparativeOp(CPU, output);
      break;
    case 0x0A:
      //shift instruction OR mod arith instruction
      if (INSN_4_5(instr) == 0x3) {
        ArithmeticOp(CPU, output);
      } else {
        ShiftModOp(CPU, output);
      }
      break;
    case 0x04:
      //Jump subroutine instruction
      JSROp(CPU, output);
      break;
    case 0x0C:
      //Jump instruction
      JumpOp(CPU, output);
      break;
    case 0x0F:
      //TRAP instruction
      uimm = INSN_7_0(instr);
      
      //set control signals
      ClearSignals(CPU);
      CPU->rdMux_CTL= 1;
      CPU->regFile_WE = 1;
      CPU->NZP_WE = 1;
      CPU -> R[7] = (CPU->PC)+1;
      
      //if PSR bit 15 = 0, make 1
      if (CPU->PSR < 0x8000) {
        CPU->PSR = CPU->PSR +0x8000;
      }

      //ensures that msb of 2 bits is 1
      CPU->PC = ((0x8000) | uimm);
      //OR PSR with 1000 0000 0000 0000 to force PSR MSB to 1
      CPU->PSR = (CPU->PSR | 0x8000);
      CPU->regInputVal = CPU->R[7];
      SetNZP(CPU, CPU->R[7]);
      break;
    case 0x08:
      //RTI instruction
      //set control signals
      ClearSignals(CPU);
      CPU->PC = CPU->R[7];
      //printf("PSR %04X \n",CPU->PSR);
      
      //0 out PSRbit 15, if PSR msb = 1
      if((CPU->PSR >= 0x8000)  ) {
        //subtract 0x8000 
        CPU->PSR = ((CPU->PSR) - 0x8000);
        //printf("New PSR %04X \n",CPU->PSR);
      }
      break;
    default:
      //NOP
      CPU->PC = (CPU->PC)+1;
      break;
  }

  nextPC = CPU->PC;
  CPU->PC = prevPC;
  //use prev pc to line up instruction and control signals
  WriteOut(CPU, output);
  CPU->PC = nextPC;
  return 0;
}



//////////////// PARSING HELPER FUNCTIONS ///////////////////////////



/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subOp = INSN_11_9(instr);
    short offset = (short) INSN_8_0(instr);

    //if bit 8 = 1, set to negative
    if (offset >= 256) {
      //printf("negative ");
      offset = offset + 0xFE00;
    }

    //printf("Offset %d subop %X NZPVal %X\n", offset,subOp, CPU->NZPVal);

    switch (subOp){
      case 0x0:
        //NOP
        CPU->PC = (CPU->PC)+1;
        //printf("nop\n");
        break;
      case 0x1:
        //BRp
        if (CPU->NZPVal == 1) {
          CPU->PC = (CPU->PC)+1+offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brp\n");
        break;
      case 0x2:
        //BRz
        if (CPU->NZPVal == 2) {
          CPU->PC = (CPU->PC)+1+ offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brz\n");
        break;
      case 0x3:
        //BRzp 
        if (CPU->NZPVal == 2 || CPU->NZPVal == 1) {
          CPU->PC = (CPU->PC)+1+ offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brzp\n");
        break;
      case 0x4:
        //BRn
        if (CPU->NZPVal  == 4) {
          CPU->PC = (CPU->PC)+1+offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brn\n");
        break;
      case 0x5:
        //BRnp
        if (CPU->NZPVal  == 4 || CPU->NZPVal == 1) {
          CPU->PC = (CPU->PC)+1+ offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brnp\n");
        break;
      case 0x6:
        //BRnz
        if (CPU->NZPVal == 4 || CPU->NZPVal  == 2) {
          CPU->PC = (CPU->PC)+1+offset;
        } else {
          CPU->PC = CPU->PC+1;
        }
        //printf("brnz\n");
        break;
      case 0x7:
        //BRnzp
        CPU->PC = CPU->PC+1+offset;
        //printf("brnzp\n");
        break;
    }

    //Every signal should be 0
    ClearSignals(CPU);
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short rd = INSN_11_9(instr);
    unsigned short rs = INSN_8_6(instr);
    unsigned short rt = INSN_2_0(instr);

    //set control signals
    ClearSignals(CPU);
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;

    //mod case
    if (INSN_OP(instr) == 0xA) {
      CPU->memory[rd] = CPU->memory[rs]%CPU->memory[rt];
      CPU->regInputVal = CPU->R[rd];
      SetNZP(CPU, CPU->R[rd]);
      CPU->PC = CPU->PC+1;
      return;
    }

    //not mod
    unsigned short subOp = INSN_5_3(instr);
    switch (subOp){
      case 0x0:
        //add
        CPU->R[rd] = (short)CPU->R[rs] + (short)CPU->R[rt];
        break;
      case 0x1:
        //multiply
        CPU->R[rd] = (short)CPU->R[rs] * (short)CPU->R[rt];
        break;
      case 0x2:
        //subtract
        CPU->R[rd] = (short)CPU->R[rs] - (short)CPU->R[rt];
        break;
      case 0x3:
        //Divide
        if (CPU->R[rt] == 0) {
          CPU->R[rd] = 0;
        } else {
          CPU->R[rd] = (short)CPU->R[rs] / (short)CPU->R[rt];
        }
        break;
      default:
        //add immediate
        if (subOp > 0x3) {
          //reading out 0000 0000 000x xxxx
          rt = (short)INSN_4_0(instr);
          //is x xxxx negative?
          if (rt > 16) {
            rt = rt+ 0xFFE0;
          }
          
          CPU->R[rd] = (short)CPU->R[rs] + (short)rt;
          //printf("rt %02X Reg source %X Reg destination %X \n", rt, (short)CPU->R[rs], CPU->R[rd]);
        } else {
          //error
          printf("Error in arithmetic");
          return;
        }
        break;
    }

    CPU->regInputVal = CPU->R[rd];
    SetNZP(CPU, CPU->R[rd]);

    //increment PC
    CPU->PC = CPU->PC+1;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short rt = INSN_2_0(instr);
    unsigned short rs = INSN_11_9(instr);
    unsigned short uimm;
    short imm;
    short result;
    unsigned short subOp = INSN_8_7(instr);

    ClearSignals(CPU);
    //set control signals
    CPU->NZP_WE = 1;

    switch (subOp) {
      case 0x0:
        //cmp
       
        result = (short)((short)CPU->R[rs] - (short)CPU->R[rt]) * ((short)CPU->R[rs] - (short)CPU->R[rt]);
        //mathamatically, result always positive, sign to negative - BUT why is result sometimes negative?
        //printf("result %d %X\n", result, result);
        /*if (((short)CPU->R[rs] - (short)CPU->R[rt]) < 0) {
          result = result *-1;
        }*/
      
        SetNZP(CPU, result);
        //printf("result %d %X\n", result, result);
        break;
      case 0x1:
        //cmpu
        result = (CPU->R[rs] - CPU->R[rt]) * (CPU->R[rs] - CPU->R[rt]);
        //sign result
        if (((short)CPU->R[rs] - (short)CPU->R[rt]) < 0) {
          result = result *-1;
        }
        SetNZP(CPU, result);
        break;
      case 0x2:
        //cmpi
        imm = (short)INSN_6_0(instr);
        if (imm >= 64) {
          imm = imm + 0xFF80;
        }
        
        result = (short)(CPU->R[rs]) - imm;
        SetNZP(CPU, result);
        break;
      case 0x3:
        //cmpiu
        uimm = INSN_6_0(instr);
        result = (CPU->R[rs]) - uimm;
        
        if (((short)(CPU->R[rs]) - (short)(uimm)) < 0 ) {
          result = result*-1;
        } 
        SetNZP(CPU, result);
        break;
    }
    
    CPU->PC = CPU->PC+1;
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short rd = INSN_11_9(instr);
    unsigned short rs = INSN_8_6(instr);
    unsigned short rt = INSN_2_0(instr);

    ClearSignals(CPU);
    //set control signals
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;

    unsigned short subOp = INSN_5_3(instr);
    switch (subOp) {
      case 0x0:
        //AND
        CPU->R[rd] = (CPU->R[rs]) & (CPU->R[rt]);
        break;
      case 0x1:
        //NOT
        CPU->R[rd] = !(CPU->R[rs]);
        break;
      case 0x2:
        //OR
        CPU->R[rd] = CPU->R[rs] | CPU->R[rt];
        break;
      case 0x3:
        //xor
        CPU-> R[rd] = (CPU->R[rs])^(CPU->R[rt]);
        break;
      default:
        if (subOp > 0x3) {
          rt = INSN_4_0(instr);
          CPU->R[rd] = (CPU->R[rs]) & (rt);
        }
        break;
    }

    CPU -> regInputVal = CPU-> R[rd];
    
      SetNZP(CPU, CPU-> R[rd]);
    
    
    //increment PC
    CPU->PC = CPU->PC+1;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short rs = INSN_8_6(instr);
    short imm;
    unsigned short subOp = INSN_11(instr);

    ClearSignals(CPU);
    CPU->rdMux_CTL = 1;

    if (subOp == 0) {
      //check if trying to access to data memory
      if (CPU->R[rs] >= 0x2000 && CPU->R[rs] < 0x8000 ) {
        printf("Treating data memory as code");
        return;
      }
      CPU->PC = CPU->R[rs];
    } else if (subOp == 1) {
      imm = (short) INSN_10_0(instr);
      if (imm >= 1024) {
        imm = imm + 0xF800;
      }
      CPU->PC = (CPU->PC) +1 + imm;
    }
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short subOp = INSN_11(instr);
    unsigned short rs = INSN_8_6(instr);
    short imm;

    ClearSignals(CPU);
    //set control signals
    CPU->NZP_WE = 1;
    CPU->regFile_WE = 1;
    CPU->rdMux_CTL = 1;

    if (subOp == 0) {
      //JSRR
      //check if trying to access to data memory
      if (CPU->R[rs] >= 0x2000 && CPU->R[rs] < 0x8000 ) {
        printf("Treating data memory as code");
        return;
      }

      //probably shouldn't use R[7] as source, but in case this handles it
      if (rs == 7) {
        imm = CPU->R[rs];
      }
      CPU->R[7] = (CPU->PC)+1;
      CPU->PC = imm;
      CPU->regInputVal = CPU->R[7];
      SetNZP(CPU, CPU->R[7]);
    } else if (subOp == 1) {
      //JSR
      imm = (short) INSN_10_0(instr);
      if (imm >= 1024) {
        imm = imm + 0xF800;
      }
      CPU->R[7] = (CPU->PC) + 1;
      CPU->regInputVal = CPU->R[7];
      CPU->PC = ((CPU->PC) & 0x8000) | (imm << 4);
      SetNZP(CPU, CPU->R[7]);
    }
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output)
{
    unsigned short instr = CPU->memory[CPU->PC];
    unsigned short rs = INSN_8_6(instr);
    unsigned short rd = INSN_11_9(instr);
    unsigned short uimm;

    //set control signals
    ClearSignals(CPU);
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->rdMux_CTL = 1;

    //set umm value
    uimm = INSN_3_0(instr);
    unsigned short subOp = INSN_5_4(instr);

    if (subOp == 0) {
      CPU->R[rd] = CPU->R[rs] << uimm;
    } else if (subOp == 1) {
      CPU->R[rd] = CPU->R[rs] >> uimm;

      //if msb = 1, add all to front
      if ((short)CPU->R[rs] < 0 ) {
       CPU->R[rd] = ((short)CPU->R[rs]) ;
      }
    } else if (subOp == 2) {
      CPU->R[rd] = CPU->R[rs] >> uimm;
    }
    CPU->regInputVal = CPU->R[rd];
    CPU->PC = CPU->PC+1;
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result)
{
    if (CPU->NZP_WE == 1) {
      if (result < 0 ) {
        CPU->NZPVal = 4;
      } else if (result > 0) {
        CPU->NZPVal = 1;
      } else if (result == 0) {
        CPU->NZPVal = 2;
      }
    }
}
