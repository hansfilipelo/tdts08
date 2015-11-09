#ifndef _GLOBALS_H_
#define _GLOBALS_H_

extern unsigned int big_endian;

extern unsigned int text_start;
extern unsigned int text_size;
extern unsigned int entry_point;

extern unsigned char *text_segment;

/* the basic blocks are numbered from 0 to no_of_bb - 1 */
/* basic_blocks[i] = v means the v-th location in text_segment is
   the first location in the i-th basic block, 0 <= i < no_of_bb,
   0 <= v < text_size */
/* basic_block[no_of_bb] = text_size */
/* the i-th basic block contains the locations
   basic_block[i] -- basic_block[i+1] - 1, 0 <= i < no_of_bb */

extern unsigned int *basic_blocks;
extern unsigned int no_of_bb;
extern unsigned int bb;

#endif
