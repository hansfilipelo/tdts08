#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "globals.h"
#include "basic_block.h"
#include "loader.h"
#include "deps.h"
#include "graph.h"
#include "exit_codes.h"
#include "utils.h"

#define PERCENTAGE 5
#define MAX_LINE 1024

#define alu_cost_u 2.0
#define mul_cost_u 16.0
#define fpu_cost_u 32.0
#define mmu_cost_u 100.0

/* whenever we refer to the "program" we mean the program to be compiled */

/* the address of the first instruction in the program */
/* it is used in the basic block determination procedure */
/* you won't need it, as the basic blocks are already delimited for you */
unsigned int entry_point;

/* the text segment of the program */
/* it is loaded from the binary supplied as an argument to the VLIW compiler */
/* each instruction is 8 bytes long */
unsigned char *text_segment;

/* the address of the first instruction in the text segment */
/* it is not necessarily equal to the entry point */
unsigned int text_start;

/* the text segment size in bytes (not in instructions) */
/* the number of instructions is text_size / 8 */
unsigned int text_size;

/* = 1 if the host is big endian, 0 if it is little endian */
/* don't bother */
/* not used at all, because the compiler runs anyway only on hosts of the
   same endianess as the programs to be compiled */
unsigned int big_endian;

/* the array of basic blocks */
/* look at `globals.h' for an explanation how the basic blocks are encoded
   in this data structure */
unsigned int *basic_blocks;

/* the number of basic blocks */
unsigned int no_of_bb;
/* the current basic block */
unsigned int bb = (unsigned int)-1;
unsigned int sa, ea, nstr;

/* the program name given as an argument to the compiler in the command line */
static char *input, *graph_file, *vliw_file;
unsigned int get_away = 1;

/* don't bother */
static void usage(char *s) {
  fprintf(stderr, "Usage mode 1:\n");
  fprintf(stderr, "%s input_file\n", s);
  fprintf(stderr, "\tIn this mode, a table is output.\n");
  fprintf(stderr, "\tThe table has B rows and four columns.\n");
  fprintf(stderr, "\tB is the number of basic blocks in the program.\n");
  fprintf(stderr, "\tThe 1st column indicates the basic block number (0 based)\n");
  fprintf(stderr, "\tThe 2nd column indicates the start address of the basic block\n");
  fprintf(stderr, "\tThe 3rd column indicates the end address of the basic block\n");
  fprintf(stderr, "\tThe 4th column indicates the number of instructions in the basic block\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage mode 2:\n");
  fprintf(stderr, "%s -b basic_block -d dependency_graph -v vliw_packing\n", s);
  fprintf(stderr, "\tIn this mode, the program checks if the given dependency graph and VLIW\n");
  fprintf(stderr, "\tinstructions corresponding to the given basic block are correct\n");
}

/* don't bother */
static void parse_cmd_line(int argc, char *argv[]) {
  int c;
  extern int optind;
  char *endp;
  
  while ((c = getopt(argc, argv, "hb:d:v:")) != EOF) {
    switch (c) {
    case 'b':
      if (optarg == NULL) {
	fprintf(stderr,
		"-%c requires to be followed by a basic block number\n",
		c);
	exit(INTERF_ERR);
      }
      errno = 0;
      bb = (unsigned int)strtoul(optarg, &endp, 0);
      if (errno != 0 || *endp != '\0') {
	fprintf(stderr, "%s is not a valid basic block number\n", optarg);
	exit(INTERF_ERR);
      }
      break;
    case 'd':
      if (optarg == NULL) {
	fprintf(stderr, "-d has to be followed by the dependency graph file name\n");
	exit(INTERF_ERR);
      }
      graph_file = optarg;
      break;
    case 'v':
      if (optarg == NULL) {
	fprintf(stderr, "-v has to be followed by the vliw packing file name\n");
	exit(INTERF_ERR);
      }
      vliw_file = optarg;
      break;
    case '?':
      usage(argv[0]);
      exit(INTERF_ERR);
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
    }
  }
  if (graph_file != NULL || bb != (unsigned int)-1 || vliw_file != NULL) {
    if (graph_file == NULL) {
      fprintf(stderr, "Missing dependency graph file\n");
      exit(INTERF_ERR);
    }
    if (bb == (unsigned int)-1) {
      fprintf(stderr, "Missing basic block number\n");
      exit(INTERF_ERR);
    }
    if (vliw_file == NULL) {
      fprintf(stderr, "Missing vliw file\n");
      exit(INTERF_ERR);
    }
  }
  if (optind < argc)
    input = argv[optind];
  else {
    fprintf(stderr, "Missing input file\n");
    usage(argv[0]);
    exit(INTERF_ERR);
  }
}

void print_node(FILE *f, unsigned int k) {
  fprintf(f, "%#010x", k);
}

char *_fgets(FILE *f) {
  static char line[MAX_LINE];
  unsigned int l;

  if (fgets(line, MAX_LINE, f) != NULL) {
    l = strlen(line);
    if (l == 0) {
      fprintf(stderr, "Line too short\n");
      exit(INTERF_ERR);
    }
    if (line[l - 1] != '\n') {
      if (!feof(f)) {
	fprintf(stderr, "Line too long\n");
	exit(INTERF_ERR);
      }
    } else
      line[l - 1] = '\0';
    if (l > 1 && line[l - 2] == '\r')
      line[l - 2] = '\0';
    return line;
  } else if (!feof(f)) {
    fprintf(stderr, "I/O error while reading from the stream\n");
    exit(INTERF_ERR);
  }
  return NULL; 
}

graph_t *extract_graph(const char *s, unsigned int sa, unsigned int ea) {
  FILE *f;
  graph_t *g = NULL, *n1, *n2;
  char *line, *tok, *endp;
  unsigned int k1, k2;

  if ((f = fopen(s, "rb")) == NULL)
    fatal("fopen", SYS_CALL_ERR);
  while ((line = _fgets(f)) != NULL) {
    tok = strtok(line, " \t");
    if (tok != NULL) {
      errno = 0;
      k1 = (unsigned int)strtoul(tok, &endp, 0);
      if (errno != 0 || *endp != '\0') {
	fprintf(stderr, "%s is not a valid instruction address\n", tok);
	exit(INTERF_ERR);
      }
      if (k1 < sa || k1 >= ea) {
	fprintf(stderr,
		"Address %#010x does not belong to the chosen basic block.\n",
		k1);
	exit(INTERF_ERR);
      }
      n1 = add_node(&g, (void *)k1);
      if ((tok = strtok(NULL, " \t\n")) != NULL) {
	errno = 0;
	k2 = (unsigned int)strtoul(tok, &endp, 0);
	if (errno != 0 || *endp != '\0') {
	  fprintf(stderr, "%s is not a valid instruction address\n", tok);
	  exit(INTERF_ERR);
	}
	if (k2 < sa || k2 >= ea) {
	  fprintf(stderr,
		  "Address %#010x does not belong to the chosen basic block.\n",
		  k2);
	  exit(INTERF_ERR);
	}
	n2 = add_node(&g, (void *)k2);
	_add_arc(n1, n2, NULL);
      }
    }
  }
  if (fclose(f) != 0)
    fatal("fclose", SYS_CALL_ERR);
  return g;
}

unsigned int sub_graph(graph_t *g1, graph_t *g2) {
  graph_t *q, *n1, *n2;
  adj_t *t;

  for (q = g1; q != NULL; q = q->next_node) {
    if ((n1 = contains(g2, q->data)) == NULL)
      return 0;
    for (t = q->to; t != NULL; t = t->next_arc) {
      if ((n2 = contains(g2, t->node->data)) == NULL)
	return 0;
      if (!_are_connected(n1, n2, NULL))
	return 0;
    }
  }
  return 1;
}

unsigned int are_equal(graph_t *g1, graph_t *g2) {
  return sub_graph(g1, g2) && sub_graph(g2, g1);
}

unsigned int handle_token(graph_node_t *g,
			  unsigned int sa, unsigned int ea,
			  unsigned int *addrs,
			  unsigned int n, unsigned int op_type, char *tok) {
  char *endp;
  unsigned int k;

  errno = 0;
  *addrs = (unsigned int)strtoul(tok, &endp, 0);
  if (errno != 0 || *endp != '\0') {
    fprintf(stderr, "%s is not a valid instruction address\n", tok);
    return 0;
  }
  if (*addrs < sa || *addrs >= ea) {
    fprintf(stderr, "Address %#010x does not belong to the chosen basic block.\n",
	    *addrs);
    return 0;
  }
  k = (*addrs - sa) / 8;
  if ((g[k].resources.fu & ~op_type) != 0) {
    fprintf(stderr, "Incorrect VLIW instruction encoding!\n");
    fprintf(stderr,
	    "Instruction %#010x appears in an atom dedicated to the wrong operation\n", *addrs);
    return 0;
  }
  if (g[k].node->from != NULL) {
    fprintf(stderr, "Incorrect VLIW instruction packing!\n");
    fprintf(stderr,
	    "Instruction %#010x needs a not yet produced result\n", *addrs);
    return 0;
  }
  return 1;
}

unsigned int handle_instructions(graph_node_t *g,
				 unsigned int sa, unsigned int ea,
				 unsigned int *addrs, unsigned int n,
				 unsigned int op_type, char **tok) {
  unsigned int i;

  i = 0;
  if (n == 0)
    return 1;
  if (*tok == NULL) {
    fprintf(stderr, "Incomplete molecule\n");
    return 0;
  }
  if (strcmp(*tok, "nop") != 0)
    if (!handle_token(g, sa, ea, addrs + i, n, op_type, *tok))
      return 0;
  for (i = 1; i < n; i++) {
    *tok = strtok(NULL, " \t");
    if (*tok == NULL) {
      fprintf(stderr, "Incomplete molecule\n");
      return 0;
    }
    if (strcmp(*tok, "nop") != 0)
      if (!handle_token(g, sa, ea, addrs + i, n, op_type, *tok))
	return 0;
  }
  *tok = strtok(NULL, " \t");
  return 1;
}

unsigned int get_fpu_alu_mp(FILE *f, unsigned int *alus, unsigned int *muls,
			    unsigned int *fpus, unsigned int *mmus) {
  char *tok, *endp, *line;

  line = _fgets(f);
  if (line == NULL)
    return 0;
  tok = strtok(line, " \t");
  if (tok == NULL)
    return 0;
  errno = 0;
  *alus = (unsigned int)strtoul(tok, &endp, 10);
  if (errno != 0 || *endp != '\0') {
    fprintf(stderr, "%s is not a valid number of ALUs\n", tok);
    return 0;
  }
  tok = strtok(NULL, " \t");
  if (tok == NULL)
    return 0;
  errno = 0;
  *muls = (unsigned int)strtoul(tok, &endp, 10);
  if (errno != 0 || *endp != '\0') {
    fprintf(stderr, "%s is not a valid number of MULs\n", tok);
    return 0;
  }
  tok = strtok(NULL, " \t");
  if (tok == NULL)
    return 0;
  errno = 0;
  *fpus = (unsigned int)strtoul(tok, &endp, 10);
  if (errno != 0 || *endp != '\0') {
    fprintf(stderr, "%s is not a valid number of FPUs\n", tok);
    return 0;
  }
  tok = strtok(NULL, " \t");
  if (tok == NULL)
    return 0;
  errno = 0;
  *mmus = (unsigned int)strtoul(tok, &endp, 10);
  if (errno != 0 || *endp != '\0') {
    fprintf(stderr, "%s is not a valid number of ALUs\n", tok);
    return 0;
  }  
  return 1;
}

unsigned int analyse_vliw(graph_node_t *g, char *s,
			  unsigned int sa, unsigned int ea,
			  graph_t *grph) {
  FILE *f;
  unsigned int *addrs, alus = 0, muls = 0, fpus = 0, mmus = 0, i;
  char *line, *tok;
  unsigned int k = 0;
  double alu_cost, mul_cost, fpu_cost, mmu_cost, hw_cost;

  if ((f = fopen(s, "rb")) == NULL)
    fatal("fopen", SYS_CALL_ERR);
  if (!get_fpu_alu_mp(f, &alus, &muls, &fpus, &mmus)) {
    fprintf(stderr,
	    "Unable to read the number of ALUs, MULs, FPUs and Bus Access Units\n");
    return 0;
  }
  if ((addrs = (unsigned int *)
       calloc(alus + fpus + muls + mmus, sizeof(unsigned int))) == NULL)
    fatal("calloc", SYS_CALL_ERR);
  while ((line = _fgets(f)) != NULL) {
    tok = strtok(line, " \t");
    if (tok == NULL)
      continue;
    k++;
    if (!handle_instructions(g, sa, ea, addrs, alus,
			     ADD_SUB, &tok) ||
	!handle_instructions(g, sa, ea, addrs + alus, muls,
			     MUL_DIV, &tok) ||
	!handle_instructions(g, sa, ea, addrs + alus + muls, fpus,
			     FLOAT_OP, &tok) ||
	!handle_instructions(g, sa, ea, addrs + alus + muls + fpus, mmus,
			     MEM_ACC, &tok))
      return 0;
    for (i = 0; i < alus + muls + fpus + mmus; i++)
      remove_node(&grph, (void *)addrs[i]);
  }
  if (fclose(f) != 0)
    fatal("fopen", SYS_CALL_ERR);
  if (grph != NULL) {
    fprintf(stderr,
	    "There were some instructions which were not packed in VLIW instructions\n");
    return 0;
  }
  alu_cost = alus * alu_cost_u;
  mul_cost = muls * mul_cost_u;
  fpu_cost = fpus * fpu_cost_u;
  mmu_cost = mmus * mmu_cost_u;
  hw_cost = alu_cost + mul_cost + fpu_cost + mmu_cost;
  fprintf(stderr, "ALU cost:\t\t%u x %f = %f\n", alus, alu_cost_u, alu_cost);
  fprintf(stderr, "MUL cost:\t\t%u x %f = %f\n", muls, mul_cost_u, mul_cost);
  fprintf(stderr, "FPU cost:\t\t%u x %f = %f\n", fpus, fpu_cost_u, fpu_cost);
  fprintf(stderr, "Bus access cost:\t%u x %f = %f\n",
	  mmus, mmu_cost_u, mmu_cost);
  fprintf(stderr, "Total hardware cost:\t%f\n", hw_cost);
  fprintf(stderr, "Performance:\t\t%u cycles\n", k);
  fprintf(stderr, "Cost-performance ratio:\t%f\n", hw_cost * k);
  return 1;
}

int main(int argc, char *argv[]) {
  graph_node_t *gn;
  graph_t *g1, *g2;
  unsigned int i;

  /* don't bother */
  /* initializes the_bb, input */
  parse_cmd_line(argc, argv);

  /* initializes
     `text_segment', `text_start', `text_size', `entry_point', `big_endian' */
  /* uncommented code in loader.c if you're interested */
  /* don't bother */
  load(input);


  /* for each basic block look for dependencies */
  /* for each basic block remove false dependencies
     (with constraints on the number of registers or not?) */
  /* for each basic block sort the dependency graph in topological order */
  /* pack the parallel instructions (with memory access constraints?) */
  /* backend */

  /* finds the basic blocks */
  /* initializes `basic_blocks', `no_of_bb', `jump', `no_of_jumps' */
  /* uncommented code is provided in `basic_block.c' */
  /* look only if you're interested, but in general don't bother */

  /* a basic block begins at address A if:
     1. if A is the program entry point, or
     2. one reaches A as a result of a jump/branch, or
     3. the instruction at address A - 8 was a jump/branch
  */
  /* a basic block ends with the instruction J found at address B if:
     1. J is a jump/branch instruction, or
     2. J is a syscall instruction, or
     3. J is a break instruction, or
     4. J is the last instruction in the program
  */
  find_basic_blocks();

  if (bb == (unsigned int)-1) {
    printf("BB_no\t[Start_addr\tEnd_addr)\tNo_of_instructions\n");
    for (i = 0; i < 79; i++)
      printf("=");
    printf("\n");
    for (i = 0; i < no_of_bb; i++)
      printf("%u\t%#010x\t%#010x\t%u\n", i, text_start + basic_blocks[i],
	     text_start + basic_blocks[i + 1],
	     (basic_blocks[i + 1] - basic_blocks[i]) / 8);
  } else {
    if (bb >= no_of_bb) {
      fprintf(stderr, "basic block %u does not exist\n", bb);
      exit(INTERF_ERR);
    }
    sa = basic_blocks[bb] + text_start;
    ea = basic_blocks[bb + 1] + text_start;
    nstr = (ea - sa) / 8;
    gn = construct_dependency(basic_blocks[bb], basic_blocks[bb + 1]);
    g1 = gn[(basic_blocks[bb + 1] - basic_blocks[bb]) / 8 - 1].node;
    g2 = extract_graph(graph_file, sa, ea);
    if (!are_equal(g1, g2)) {
      fprintf(stderr, "WARNING: The proposed graph dependency is different from the one\n");
      fprintf(stderr, "         proposed by the tool.\n");
      fprintf(stderr, "         However, if the partial order relation defined by the tool's graph\n");
      fprintf(stderr, "         is a subset of the partial order relation defined by your graph\n");
      fprintf(stderr, "         then your solution still might be correct.\n");
      fprintf(stderr, "         Therefore, this was just a warning.\n");
    } else
      fprintf(stderr, "The dependency graph is correct!\n");
    print_graph(stdout, g2, NULL, NULL, NULL,
		(void (*)(FILE *, void *))print_node);
    flush(&g2);
    if (analyse_vliw(gn, vliw_file, sa, ea, g1))
      fprintf(stderr, "The VLIW packing which you propose is correct!\n");
  }

  return 0;
}
