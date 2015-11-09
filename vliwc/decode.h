#ifndef _DECODE_H_
#define _DECODE_H_

#define READ_MEMORY 0x01
#define WRITE_MEMORY 0x02

#define ADD_SUB 0x01
#define MUL_DIV 0x02
#define MEM_ACC 0x04
#define FLOAT_OP 0x08

/* check "The SimpleScalar Tool Set, Version 2.0", Doug Burger, Todd M. Austin
   for the instruction format */


/* the resources an instruction uses */
/* written[i] = r means that the instruction writes a result
   in the register r, 0 <= i < 2 */
/* read[i] = r means that the instruction reads an operand
   from the register r, 0 <= i < 4 */
/* for example if
   read[0] = 1;
   read[1] = 0;
   read[2] = 5;
   read[3] = 0;
   written[0] = 0;
   written[1] = 2;
   then it means that the instruction uses registers 1 and 5 in read mode
   and register 2 for writing in it */
/* memory = READ_MEMORY means that the instruction accesses the memory in
   read mode */
/* memory = WRITE_MEMORY means that the instruction accesses the memory in
   write mode */
/* memory = 0 means that the instruction does not access the memory */
typedef struct {
  unsigned char written[2];
  unsigned char read[4];
  unsigned char fu;
  unsigned char memory;
} resources_t;

#define ZERO	0x00
#define AT	0x01
#define V0	0x02
#define V1	0x03
#define A0	0x04
#define A1	0x05
#define A2	0x06
#define A3	0x07
#define T0	0x08
#define T1	0x09
#define T2	0x0a
#define T3	0x0b
#define T4	0x0c
#define T5	0x0d
#define T6	0x0e
#define T7	0x0f
#define S0	0x10
#define S1	0x11
#define S2	0x12
#define S3	0x13
#define S4	0x14
#define S5	0x15
#define S6	0x16
#define S7	0x17
#define T8	0x18
#define T9	0x19
#define K0	0x1a
#define K1	0x1b
#define GP	0x1c
#define SP	0x1d
#define S8	0x1e
#define RA	0x1f
#define HI	0x20
#define LO	0x21

#define FLOAT_REG_BASE 0x22

#define F00	0x22
#define F01	0x23
#define F02	0x24
#define F03	0x25
#define F04	0x26
#define F05	0x27
#define F06	0x28
#define F07	0x29
#define F08	0x2a
#define F09	0x2b
#define F0a	0x2c
#define F0b	0x2d
#define F0c	0x2e
#define F0d	0x2f
#define F0e	0x30
#define F0f	0x31
#define F10	0x32
#define F11	0x33
#define F12	0x34
#define F13	0x35
#define F14	0x36
#define F15	0x37
#define F16	0x38
#define F17	0x39
#define F18	0x3a
#define F19	0x3b
#define F1a	0x3c
#define F1b	0x3d
#define F1c	0x3e
#define F1d	0x3f
#define F1e	0x40
#define F1f	0x41

#define FCC	0x42


#define NOP	0x00

#define J	0x01
#define JAL	0x02
#define JR	0x03
#define JALR	0x04
#define BEQ	0x05
#define BNE	0x06
#define BLEZ	0x07
#define BGTZ	0x08
#define BLTZ	0x09
#define BGEZ	0x0a
#define BC1F	0x0b
#define BC1T	0x0c

#define LB	0x20
#define nLB	0xc0
#define LBU	0x22
#define nLBU	0xc1
#define LH	0x24
#define nLH	0xc2
#define LHU	0x26
#define nLHU	0xc3
#define LW	0x28
#define nLW	0xc4
#define DLW	0x29
#define nDLW	0xce
#define L_S	0x2a
#define nL_S	0xc5
#define L_D	0x2b
#define nL_D	0xcf
#define LWL	0x2c
#define LWR	0x2d
#define SB	0x30
#define nSB	0xc6
#define SH	0x32
#define nSH	0xc7
#define SW	0x34
#define nSW	0xc8
#define DSW	0x35
#define nDSW	0xd0
#define DSZ	0x38
#define nDSZ	0xd1
#define S_S	0x36
#define nS_S	0xc9
#define S_D	0x37
#define nS_D	0xd2
#define SWL	0x39
#define SWR	0x3a

#define ADD	0x40
#define ADDI	0x41
#define ADDU	0x42
#define ADDIU	0x43
#define SUB	0x44
#define SUBU	0x45
#define MULT	0x46
#define MULTU	0x47
#define DIV	0x48
#define DIVU	0x49
#define MFHI	0x4a
#define MTHI	0x4b
#define MFLO	0x4c
#define MTLO	0x4d
#define AND	0x4e
#define ANDI	0x4f
#define OR	0x50
#define ORI	0x51
#define XOR	0x52
#define XORI	0x53
#define NOR	0x54
#define SLL	0x55
#define SLLV	0x56
#define SRL	0x57
#define SRLV	0x58
#define SRA	0x59
#define SRAV	0x5a
#define SLT	0x5b
#define SLTI	0x5c
#define SLTU	0x5d
#define SLTIU	0x5e

#define ADD_S	0x70
#define ADD_D	0x71
#define SUB_S	0x72
#define SUB_D	0x73
#define MUL_S	0x74
#define MUL_D	0x75
#define DIV_S	0x76
#define DIV_D	0x77
#define ABS_S	0x78
#define ABS_D	0x79
#define MOV_S	0x7a
#define MOV_D	0x7b
#define NEG_S	0x7c
#define NEG_D	0x7d
#define CVT_S_D	0x80
#define CVT_S_W	0x81
#define CVT_D_S	0x82
#define CVT_D_W	0x83
#define CVT_W_S	0x84
#define CVT_W_D	0x85
#define C_EQ_S	0x90
#define C_EQ_D	0x91
#define C_LT_S	0x92
#define C_LT_D	0x93
#define C_LE_S	0x94
#define C_LE_D	0x95
#define SQRT_S	0x96
#define SQRT_D	0x97

#define SYSCALL	0xa0
#define BREAK	0xa1
#define LUI	0xa2
#define MFC1	0xa3
#define MTC1	0xa5

/* don't bother */
/* the first argument is an instruction word */
/* the second argument is an out argument */
/* if the instruction is a jump/branch instruction where the target address
   can be statically determined then the function returns true and the
   target address is returned in the second argument */
/* if the instruction is a jump instruction where the address can not be
   statically determined, the program exits with an error message */
/* if the instruction is not a jump/branch instruction, then the function
   returns false */
extern int jump_instruction(unsigned char *, unsigned int *);

/* given an instruction (an array of 8 bytes), return the opcode */
extern unsigned short get_opcode(unsigned char *);
/* given an instruction (an array of 8 bytes), return the `rs' field from it */
extern unsigned char get_rs(unsigned char *);
/* given an instruction (an array of 8 bytes), return the `rd' field from it */
extern unsigned char get_rd(unsigned char *);
/* given an instruction (an array of 8 bytes), return the `rt' field from it */
extern unsigned char get_rt(unsigned char *);
/* given an instruction (an array of 8 bytes),
   return the `target' field from it */
extern unsigned int get_target(unsigned char *);
/* given an instruction (an array of 8 bytes),
   return the `immediate' field from it */
extern short get_imm(unsigned char *);

/* return true if the instruction uses some resources */
/* it returns false if all the elements in the `read' and `written' arrays
   in the argument are 0 */
extern int uses_resources(resources_t *);
/* return true if the second instruction */
extern int raw(resources_t *, resources_t *);
extern int mem_dep(resources_t *, resources_t *);

/* given an instruction (an array of 8 bytes), it initializes the
   structure of resources the instruction uses */
extern void used_resources(unsigned char *, resources_t *);

/* `syscall', `break', jumps and branches appear always as the
   last instruction in the basic blocks */
/* they are not allowed to be packed with some other instructions */
/* `is_moveable' returns true if the instruction given as an argument
   (an array of 8 bytes) is such an instruction */
extern int is_moveable(unsigned char *);

#endif
