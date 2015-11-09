#include <stdlib.h>
#include "deps.h"
#include "graph.h"
#include "decode.h"
#include "utils.h"
#include "exit_codes.h"
#include "globals.h"

int ascendent(graph_t *parent, graph_t *child) {
  adj_t *w;

  for (w = child->from; w != NULL; w = w->next_arc)
    if (w->node == parent)
      return 1;
  for (w = child->from; w != NULL; w = w->next_arc)
    if (ascendent(parent, w->node))
      return 1;
  return 0;
}

void connect_to_graph(graph_node_t *g,
		      unsigned int k) {
  unsigned int i;
  unsigned char mem_not_done;
  /* we may have at most four in-operands (dsw rt, rs, rd) */
  /* therefore we may have at most four RAW dependencies */
  /* because we may have at most two out-operands (dlw rt, rs, rd)
     we may have at most two RAW dependencies between any two instructions */
  /* if the instruction is a load instruction then it needs to
     follow any previous (in program order sense) store instruction */
  /* if the instruction is a store instruction it needs to follow any
     other previous (in program order sense) load or store instruction */

  mem_not_done = g[k].resources.memory;

  for (i = k; i > 0 && (uses_resources(&g[k].resources) || mem_not_done);) {
    i--;
    if (raw(&g[i].resources, &g[k].resources))
      /* k depends on i (i becomes an ascendent of k) */
      /* the arc is added only if k does not already depend
	 on a descendant of i */
      if (!ascendent(g[i].node, g[k].node))
	_add_arc(g[i].node, g[k].node, NULL);
    if (mem_not_done && mem_dep(&g[i].resources, &g[k].resources)) {
      mem_not_done = 0;
      if (!ascendent(g[i].node, g[k].node))
	_add_arc(g[i].node, g[k].node, NULL);
    }
  }
}

/* for each instruction in the basic block we construct a graph_node_t */
/* this graph node contains the list of resources the instruction uses */
graph_node_t *construct_dependency(unsigned int first,
				   unsigned int last) {
  graph_node_t *g = NULL;
  graph_t *graph = NULL;
  unsigned int i, k;

  if ((g = (graph_node_t*)calloc((last - first) / 8,
				 sizeof(graph_node_t))) == NULL) {
    fprintf(stderr, "last = %d, first = %d\n", last, first);
    fatal("construct dependency, calloc", SYS_CALL_ERR);
  }

  for (i = first; i < last; i += 8) {
    k = (i - first) / 8;
    /* determine the used resources */
    used_resources(text_segment + i, &g[k].resources);
    g[k].node = add_node(&graph, (void *)(text_start + i));
    /* determine its dependencies */
    /* and remove the artificial ones */
    connect_to_graph(g, k);
  }
  return g;
}
