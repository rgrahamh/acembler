#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//The size of arguments (area between delimitors)
#define DELIM_SIZE 32

//A char* for handlers to play around with; would be local, but this is far more efficient than constantly malloc-ing
char val[32];

//Struct for R-Type instructions
typedef struct r_type_struct{
    unsigned int opcode:6;
    unsigned int rs:5;
    unsigned int rt:5;
    unsigned int rd:5;
    unsigned int shamt:5;
    unsigned int funct:6;
} r_instr;

//Struct for I-Type instructions
typedef struct i_type_struct{
    unsigned int opcode:6;
    unsigned int rs:5;
    unsigned int rt:5;
    unsigned int immediate:16;
} i_instr;

//Struct for J-Type instructions
typedef struct j_type_struct{
    unsigned int opcode:6;
    unsigned int addr:26;
} j_instr;

// INSTRUCTION KEY:
// ---------------
// R-Type Instructions:
//  0: add (add)
//    0x20
//  1: addu (add unsigned)
//    0x21
//  2: and (AND)
//    0x24
//  3: jr (jump register)
//    0x08
//  4: nor (NOR)
//    0x27
//  5: or (OR)
//    0x25
//  6: slt (set less than)
//    0x2a
//  7: sltu (set less than unsigned)
//    0x2b
//  8: sll (shift left logical)
//    0x00
//  9: srl (shift right logical)
//    0x02
// 10: sub (subtract)
//    0x22
// 11: subu (subtract unsigned)
//    0x23
char* R_TYPE_INSTR[] = {"add", "addu", "and", "jr", "nor", "or", "slt", "sltu", "sll", "slr", "sub", "subu"};
char R_TYPE_FCODE[] = {0x20, 0x21, 0x24, 0x08, 0x27, 0x25, 0x2a, 0x2b, 0x00, 0x02, 0x22, 0x23};

// I-Type Instructions:
//  0: addi (add immediate)
//    0x08
//  1: addiu (add immediate unsigned)
//    0x09
//  2: andi (AND immediate)
//    0x0c
//  3: beq (branch equal)
//    0x04
//  4: bne (branch not equal)
//    0x05
//  5: lbu (load byte unsigned)
//    0x24
//  6: lhu (load halfword unsigned)
//    0x25
//  7: ll (load linked)
//    0x30
//  8: lui (load upper immediate)
//    0x0f
//  9: lw (load word)
//    0x23
// 10: sltiu (set less than immediate unsigned)
//    0x0b
// 11: sb (store byte)
//    0x28
// 12: sc (store conditional)
//    0x38
// 13: sh (store halfword)
//    0x29
// 14: sw (store word)
//    0x2b
// 15: ori (OR immediate)
//    0x0d
// 16: slti (set less than immediate)
//    0x0a
char* I_TYPE_INSTR[] = {"addi", "addiu", "andi", "beq", "bne", "lbu", "lhu", "ll", "lui", "lw", "sltiu", "sb", "sc", "sh", "sw", "ori", "slti"};
char I_TYPE_OPCODE[] = {0x08, 0x09, 0x0c, 0x04, 0x05, 0x24, 0x25, 0x30, 0x0f, 0x24, 0x0b, 0x28, 0x38, 0x29, 0x2b, 0x0d, 0x0a};

// J-Type Instructions:
//  0: j (jump)
//    0x02
//  1: jal (jump and link)
//    0x03
char* J_TYPE_INSTR[] = {"j", "jal"};
char J_TYPE_OPCODE[] = {0x02, 0x03};

// Register aliases:
//     0: $zero (zero)
//     1: $at (assembler temporary)
//   2,3: $v0,$v1 (return values)
//   4-7: $a0,$a3 (arguments)
//  8-15: $t0-$t7 (temporaries)
// 16-23: $s0-$s7 (saved temporaries)
// 24,25: $t8,$t9 (temporaries)
// 26,27: $k0,$k1 (reserved for kernel)
//    28: $gp (global pointer)
//    29: $sp (stack pointer)
//    30: $fp (frame pointer)
//    31: $ra (return address)
char* REG[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "fa"};


//Helper functions
char* nextArg(char* str, char delim);
int compareString(char* main, char* substr);
int substr(char* str, char until, char* cpy, int cpySize);
int parseReg(char* reg);

//Comparison functions; returns 1 if it's an X-Type instr, 0 otherwise
int compRType(char* instr);
int compIType(char* instr);
int compJType(char* instr);
int compReg(char* reg);

//Instruction handlers
void handleRType(char* instr, int instrIdx, r_instr* new_instr);
void handleIType(char* instr, int instrIdx, i_instr* new_instr);
void handleJType(char* instr, int instrIdx, j_instr* new_instr);

int VERBOSE = 0;
