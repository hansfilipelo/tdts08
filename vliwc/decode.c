#include "decode.h"
#include "exit_codes.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

unsigned short get_opcode(unsigned char *instruction) {
  return *(unsigned short*)(instruction + 2);
}

unsigned int get_target(unsigned char *instruction) {
  return *(unsigned int*)(instruction + 4) & 0x00ffffff;
}

unsigned char get_rs(unsigned char *instruction) {
  return instruction[4];
}

unsigned char get_rd(unsigned char *instruction) {
  return instruction[6];
}

unsigned char get_rt(unsigned char *instruction) {
  return instruction[5];
}

short get_imm(unsigned char *instruction) {
  return *(unsigned int*)(instruction + 6);
}

int jump_instruction(unsigned char *instruction, unsigned int *target) {
  unsigned short opcode;
  unsigned int PC;

  opcode = get_opcode(instruction);

  /* will be removed */

  /*
  if (opcode == JR && get_rs(instruction) != RA || opcode == JALR) {
    fprintf(stderr,
	    "The program contains jumps whose addresses cannot be determined at compile\n");
    fprintf(stderr, "time. This happens most likely because of function variables. In such cases\n");
    fprintf(stderr, "one cannot determine the basic blocks and hence detect the data dependencies.\n");
    fprintf(stderr, "The static optimization is impossible. The compiler cowardly aborts.\n");
    exit(INTERF_ERR);
  }
  */

  /* until here */
  if (opcode == JR || opcode == JALR) {
    /* will be removed */
    /*
    if ((opcode == JR && get_rs(instruction) != RA) ||
	(opcode == JALR && get_rd(instruction) != RA))
      fprintf(stderr,
	      "Suspicious JR or JALR %#010x%08x at address %#010x\n",
	      *(unsigned int*)instruction,
	      *(unsigned int*)(instruction + 4),
	      text_start + instruction - text_segment);
    */
    /* until here */
    *target = (unsigned int)-1;
    return 1;
  }

  if (opcode == NOP || opcode > BC1T)
    return 0;

  PC = text_start + instruction - text_segment;
  if (opcode < JR)
    *target = (PC & 0xf0000000) | (get_target(instruction) << 2);
  else
    *target = PC + 8 + (get_imm(instruction) << 2);
  return 1;
}

void used_resources(unsigned char *instruction, resources_t *r) {
  unsigned short opcode;

  opcode = get_opcode(instruction);

  switch (opcode) {

  case JALR:
    r->written[0] = get_rd(instruction);
  case JR:
    r->read[0] = get_rs(instruction);
    break;

  case BLTZ:
  case BLEZ:
  case BGTZ:
  case BGEZ:
    r->fu = ADD_SUB;
    r->read[0] = get_rs(instruction);
    break;

  case BEQ:
  case BNE:
    r->fu = ADD_SUB;
    r->read[0] = get_rs(instruction);
    r->read[1] = get_rt(instruction);
    break;

  case BC1F:
  case BC1T:
    r->read[0] = FCC;
    break;

  case nDLW:
    r->written[1] = get_rt(instruction) + 1;
  case nLB:
  case nLBU:
  case nLH:
  case nLHU:
  case nLW:
    r->read[1] = get_rd(instruction);
  case DLW:
  case LB:
  case LBU:
  case LH:
  case LHU:
  case LW:
  case LWL:
  case LWR:
    r->fu = MEM_ACC;
    r->read[0] = get_rs(instruction);
    r->written[0] = get_rt(instruction);
    r->memory = READ_MEMORY;
    if (opcode == DLW)
      r->written[1] = r->written[0] + 1;
    if (opcode == LWL || opcode == LWR)
      r->read[1] = r->written[0];
    break;

  case nL_D:
    r->written[1] = get_rt(instruction) + FLOAT_REG_BASE + 1;
  case nL_S:
    r->read[1] = get_rd(instruction);
  case L_D:
  case L_S:
    r->fu = MEM_ACC;
    r->read[0] = get_rs(instruction);
    r->written[0] = get_rt(instruction) + FLOAT_REG_BASE;
    r->memory = READ_MEMORY;
    if (opcode == L_D)
      r->written[1] = r->written[0] + 1;
    break;

  case nDSW:
    r->read[3] = get_rt(instruction) + 1;
  case nSB:
  case nSH:
  case nSW:
    r->read[2] = get_rd(instruction);
  case SB:
  case SH:
  case SW:
  case DSW:
  case SWL:
  case SWR:
    r->fu = MEM_ACC;
    r->read[1] = get_rs(instruction);
    r->read[0] = get_rt(instruction);
    r->memory = WRITE_MEMORY;
    if (opcode == DSW)
      r->read[3] = r->read[0] + 1;
    break;

  case nDSZ:
    r->read[1] = get_rd(instruction);
  case DSZ:
    r->fu = MEM_ACC;
    r->read[0] = get_rs(instruction);
    r->memory = WRITE_MEMORY;
    break;

  case nS_D:
    r->read[3] = get_rt(instruction) + FLOAT_REG_BASE + 1;
  case nS_S:
    r->read[2] = get_rd(instruction);
  case S_D:
  case S_S:
    r->fu = MEM_ACC;
    r->read[0] = get_rs(instruction);
    r->read[1] = get_rt(instruction) + FLOAT_REG_BASE;
    r->memory = WRITE_MEMORY;
    if (opcode == S_D)
      r->read[2] = r->read[1] + 1;
    break;    

  case ADD:
  case ADDU:
  case SUB:
  case SUBU:
  case AND:
  case OR:
  case XOR:
  case NOR:
  case SLLV:
  case SRLV:
  case SLT:
  case SLTU:
    r->fu = ADD_SUB;
    r->read[0] = get_rs(instruction);
    r->read[1] = get_rt(instruction);
    r->written[0] = get_rd(instruction);
    break;
    
  case ADDI:
  case ADDIU:
  case ANDI:
  case ORI:
  case XORI:
    r->fu = ADD_SUB;
    r->read[0] = get_rs(instruction);
    r->written[0] = get_rt(instruction);
    break;
  case SLTI:
  case SLTIU:
    r->fu = ADD_SUB;
    r->read[0] = get_rs(instruction);
    r->written[0] = get_rd(instruction);
    break;
    
  case SRAV:
    r->read[1] = get_rs(instruction);
  case SLL:
  case SRL:
  case SRA:
    r->fu = ADD_SUB;
    r->read[0] = get_rt(instruction);
    r->written[0] = get_rd(instruction);
    break;

  case MULT:
  case MULTU:
  case DIV:
  case DIVU:
    r->fu = MUL_DIV;
    r->read[0] = get_rs(instruction);
    r->read[1] = get_rt(instruction);
    r->written[0] = HI;
    r->written[1] = LO;
    break;

  case MFHI:
    r->read[0] = HI;
    r->written[0] = get_rd(instruction);
    break;

  case MFLO:
    r->read[0] = LO;
    r->written[0] = get_rd(instruction);
    break;

  case MTHI:
    r->written[0] = HI;
    r->read[0] = get_rs(instruction);
    break;

  case MTLO:
    r->written[0] = HI;
    r->read[0] = get_rs(instruction);
    break;

  case ADD_S:
  case ADD_D:
  case SUB_S:
  case SUB_D:
  case MUL_S:
  case MUL_D:
  case DIV_S:
  case DIV_D:
    r->read[1] = get_rt(instruction) + FLOAT_REG_BASE;
  case ABS_S:
  case ABS_D:
  case MOV_S:
  case MOV_D:
  case NEG_S:
  case NEG_D:
  case CVT_S_D:
  case CVT_S_W:
  case CVT_D_S:
  case CVT_D_W:
  case CVT_W_S:
  case CVT_W_D:
  case SQRT_S:
  case SQRT_D:
    r->fu = FLOAT_OP;
    r->read[0] = get_rs(instruction) + FLOAT_REG_BASE;
    r->written[0] = get_rd(instruction) + FLOAT_REG_BASE;
    break;

  case C_EQ_S:
  case C_EQ_D:
  case C_LT_S:
  case C_LT_D:
  case C_LE_S:
  case C_LE_D:
    r->fu = FLOAT_OP;
    r->read[0] = get_rs(instruction) + FLOAT_REG_BASE;
    r->read[1] = get_rt(instruction) + FLOAT_REG_BASE;
    r->written[0] = FCC;
    break;

  case LUI:
    r->written[0] = get_rt(instruction);
    break;

  case MFC1:
    r->read[0] = get_rs(instruction) + FLOAT_REG_BASE;
    r->written[0] = get_rt(instruction);
    break;

  case MTC1:
    r->written[0] = get_rs(instruction) + FLOAT_REG_BASE;
    r->read[0] = get_rt(instruction);
    break;

  case SYSCALL:
    r->read[0] = V0;
    break;
  }
}

int is_moveable(unsigned char *instruction) {
  unsigned short opcode;

  opcode = get_opcode(instruction);
  if ((opcode > NOP && opcode <= BC1T) ||
      opcode == SYSCALL ||
      opcode == BREAK)
    return 0;

  return 1;
}

int uses_resources(resources_t *r) {
  int i;

  for (i = 0; i < 4; i++)
    if (r->read[i] != 0)
      return 1;
  return 0;
}

int raw(resources_t *parent, resources_t *child) {
  /* Your code here */
  /* will be removed */
  int i, j;
  int ret_val = 0;

  for (i = 0; i < 4; i++)
    if (child->read[i] != 0)
      for (j = 0; j < 2; j++)
	if (parent->written[j] == child->read[i]) {
	  child->read[i] = 0;
	  ret_val = 1;
	  break;
	}
  return ret_val;
  /* until here */
  /* End of your code */
}

int mem_dep(resources_t *parent, resources_t *child) {
  /* Your code here */
  /* will be removed */
  return 
    (child->memory == READ_MEMORY && parent->memory == WRITE_MEMORY) ||
    (child->memory == WRITE_MEMORY && parent->memory != 0);
  /* End of your code */
}
