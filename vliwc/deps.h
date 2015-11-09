#ifndef _DEPS_H_
#define _DEPS_H_

#include "graph.h"
#include "decode.h"

/* one such graph_node_t corresponds to each instruction in the program */
/* `construct_dependency' constructs actually an array of such
   nodes */
/* the actual dependencies between the instructions is captured in a graph
   structure of type `graph_t', more about it in `graph.h' */
typedef struct {
  /* the resources needed by this instruction */
  /* look in `decode.h' for a detailed description of the `resources_t'
     structure */
  resources_t resources;
  /* a pointer to the graph node of type `graph_t' */
  graph_t *node;
} graph_node_t;

graph_node_t *construct_dependency(unsigned int, unsigned int);

#endif
