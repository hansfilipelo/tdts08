#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "exit_codes.h"
#include "decode.h"
#include "basic_block.h"

#define ASCENDING 1
#define DESCENDING (-1)

#define INCR 1024
#define MAX_ALLOC 1048576

static unsigned int allocated;

static unsigned int *forwards;
static unsigned int f_alloc;
static unsigned int no_of_fwds;

static void enlarge(void) {
  allocated += INCR;
  if (allocated > MAX_ALLOC) {
    fprintf(stderr,
	    "Allocation of basic blocks (%d) exceeds maximum allowed (%d)\n",
	    allocated,
	    MAX_ALLOC);
    exit(LARGENESS_ERR);
  }
  if ((basic_blocks = (unsigned int*)realloc(basic_blocks,
					     allocated * sizeof(unsigned int)))
      == NULL)
    fatal("basic_block, enlarge, realloc", SYS_CALL_ERR);
}

static unsigned int m_bsearch(unsigned int *array,
			      unsigned int length,
			      unsigned int val,
			      int direction) {
  unsigned int l, r, m;

  l = 0;
  r = length;
  m = (l + r) / 2;

  while (l < r) {
    if (array[m] == val)
      return m;
    if ((direction == ASCENDING && val > array[m]) ||
	(direction == DESCENDING && val < array[m]))
      l = m + 1;
    else if ((direction == DESCENDING && val > array[m]) ||
	     (direction == ASCENDING && val < array[m]))
      r = m;
    m = (l + r) / 2;
  }
  return l;
}

static void enlarge_fwds(void) {
  f_alloc += INCR;
  if (f_alloc > MAX_ALLOC) {
    fprintf(stderr,
	    "Allocation of forward jump addresses (%d) exceeds maximum allowed (%d)\n",
	    f_alloc,
	    MAX_ALLOC);
    exit(LARGENESS_ERR);
  }
  if ((forwards = (unsigned int*)realloc(forwards,
					  f_alloc * sizeof(unsigned int)))
      == NULL)
    fatal("forwards, enlarge, realloc", SYS_CALL_ERR);
}

static void insert_in_forwards(unsigned int addr) {
  unsigned int where;

  where = m_bsearch(forwards, no_of_fwds, addr, DESCENDING);
  if (where < no_of_fwds && forwards[where] == addr)
    return;

  if (no_of_fwds >= f_alloc)
    enlarge_fwds();

  memmove(forwards + where + 1,
	  forwards + where,
	  (no_of_fwds - where) * sizeof(unsigned int));

  forwards[where] = addr;
  no_of_fwds++;
}

static int lookup_in_forwards(unsigned int addr) {
  if (no_of_fwds == 0)
    return 0;
  if (addr > forwards[no_of_fwds - 1]) {
    fprintf(stderr, "Skipped some forwards\n");
    fprintf(stderr, "Algorithm error\n");
    exit(ALG_ERR);
  }
  if (addr == forwards[no_of_fwds - 1]) {
    no_of_fwds--;
    return 1;
  }
  return 0;
}

static void append_bb(unsigned int k) {
  if (no_of_bb >= allocated)
    enlarge();

  basic_blocks[no_of_bb++] = k;
}

static void split_bb(unsigned int k) {
  unsigned int where;

  where = m_bsearch(basic_blocks, no_of_bb, k, ASCENDING);
  if (where < no_of_bb && basic_blocks[where] == k)
    /* jump to a previously defined basic block */
    return;
  if (no_of_bb >= allocated)
    enlarge();
  memmove(basic_blocks + where + 1,
	  basic_blocks + where,
	  (no_of_bb - where) * sizeof(unsigned int));
  no_of_bb++;
  basic_blocks[where] = k;
}

void find_basic_blocks(void) {
  unsigned int i;
  unsigned int new_bb = 1;
  unsigned int target_address;

  /* the entry point is a forward address */
  insert_in_forwards(entry_point);

  for (i = 0; i < text_size; i += 8) {
    if (lookup_in_forwards(text_start + i) || new_bb) {
      append_bb(i);
      new_bb = 0;
    }
    /* if we reach a jump instruction then
          1. the current basic block ends
          2. a basic block starts at the target address */
    /* if the target address is < current address then
          if it's not at the beginning of a basic block then
	     we split the basic block
       else
          remember the target address
    */
    if (jump_instruction(text_segment + i, &target_address)) {
      if (target_address != (unsigned int)-1) {
	/*	insert_jump_instruction(text_start + i, target_address); */
	if (target_address >= text_start + text_size)
	  fprintf(stderr,
		  "Weird target address in the instruction %#010x%08x at address %#010x\n",
		  *(unsigned int*)(text_segment + i),
		  *(unsigned int*)(text_segment + i + 4),
		  text_start + i);
	if (target_address <= text_start + i)
	  split_bb(target_address - text_start);
	else
	  insert_in_forwards(target_address);
      }
      new_bb = 1;
    }
    /* a sys_call or a break instruction delimit a new basic block */
    if (get_opcode(text_segment + i) == SYSCALL ||
	get_opcode(text_segment + i) == BREAK)
      new_bb = 1;
  }
  append_bb(text_size);
  no_of_bb--;
  if (forwards != NULL)
    free(forwards);
}
