#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdio.h>
#include <stdarg.h>

struct _s1;

/*
  for each arc added to the graph, 2 * sizeof(adj_t) = 40 bytes 
  are consumed
*/

typedef struct _s2 {
  struct _s2 *next_arc;
  struct _s2 *prev_arc;
  struct _s2 *conjug;
  struct _s1 *node;
  void *label;
} adj_t;

/*
  for each node added to the graph, sizeof(graph_t) = 20 bytes
  are consumed
*/

/*
  a totally connected graph consumes
  N * sizeof(graph_t) + 2 * N * N * sizeof(adj_t) = 20 * N + 40 * N * N bytes
  N = 10^6 => graph_size = 40000 * 10^9 bytes ~= 40 Tera :-)
  for A = 3 * N => 3 * N * 40 + N * 20 = 140 * N
  for N = 10^6 => graph_size = 140 MB
*/

typedef struct _s1 {
  struct _s1 *next_node;
  struct _s1 *prev_node;
  struct _s2 *to;
  struct _s2 *from;
  void* data;
} graph_t;

/* the first argument is the address of the first node in the graph */
/* it returns the address of the node containing the second argument */
/* or NULL if there is no such node */
/* the comparison is done at address level, and NOT at contents level */
extern graph_t *contains(graph_t*, void*);
/* the first argument is the address of the first node in the graph */
/* it returns the address of the node containing the second argument */
/* or NULL if there is no such node */
/* the comparison is done at contents level, and NOT at address level */
/* the comparison is done by means of the third argument */
/* the function passed as the third argument should return 0 if its two
   arguments are equal */
extern graph_t *contains_c(graph_t*, void*, int (*)(void*, void*));

/* the second argument is the address of the first node in the graph */
/* the function returns the address of the node containing the data
   that satisfies a condition specified by the function sent as the
   first argument */
extern graph_t *first_that(int (*)(void*, va_list), graph_t*, ...);

/* the first argument is the address of the first node in the graph */
/* if the nodes exist and are connected, then it returns the arc
   (in outgoing version) */
/* the last argument gives the arc label */
/* the comparison is done at address level */
/* if not, it returns NULL */
extern adj_t *are_connected(graph_t*, void*, void*, void*);

/* the arguments are pointers to the two nodes that may be connected */
/* they must exist in the graph */
/* the last argument gives the arc label */
/* the comparison is done at address level */
/* used mainly internally */
/* if they are connected, then return the arc (in outgoing version) */
/* if not, return NULL */
extern adj_t *_are_connected(graph_t*, graph_t*, void*);

/* the first argument is the address of the graph
   the node should be added to */
/* if the node is already present, the function returns it */
/* the comparison is done at address level */
/* if not, then it creates it, adds it and returns it */
extern graph_t *add_node(graph_t**, void*);

/* the first argument is the address of the graph
   the node should be added to */
/* the node must not exist already in the graph */
/* used mainly internally */
/* it returns the added node, i.e. the second argument */
extern graph_t *_add_node(graph_t**, graph_t*);

/* the first argument is the address of the graph
   the node should be added to */
/* the last argument is the label of the arc to be added */
/* if any of the nodes is not yet present, it is created */
/* the label and the nodes are compared at address level */
/* if the arc already exists, then it is returned (in outgoing version) */
/* if not, then it creates it, adds it and returns it (in outgoing version) */
extern adj_t *add_arc(graph_t**, void*, void*, void*);

/* adds an arc between the two nodes contained in the two arguments */
/* the two nodes have to exist */
/* used mainly internally */
/* if the arc already exists, then it is returned (in outgoing version) */
/* if not, then it creates it, adds it and returns it (in outgoing version) */
extern adj_t *_add_arc(graph_t*, graph_t*, void*);

/* the first argument is the address of the graph
   the node should be added to */
/* if the node to be removed is not present in the graph,
   then don't do anything */
/* the comparison is performed at address level */
extern void remove_node(graph_t**, void*);

/* the first argument is the address of the graph
   the node should be added to */
/* used mainly internally */
/* the node must exist */
extern void _remove_node(graph_t**, graph_t*);

/* the first argument is the address of the first node in the graph */
/* the last argument is the label of the arc */
/* if the nodes do not exist or they are not connected, don't do anything */
extern void remove_arc(graph_t*, void*, void*, void*);

/* the arguments are pointers to the two nodes that may be connected */
/* they must exist in the graph */
/* the last argument gives the label of the arc */
/* used mainly internally */
/* if they are not connected, don't do anything */
extern void _remove_arc(graph_t*, graph_t*, void*);

/* the arc must exist */
/* it is the outgoing version */
/* used mainly internally */
/* it will remove also its conjugate */
extern void __remove_arc(adj_t*);

extern void flush(graph_t**);

/* the first argument specifies the output stream
   the graph should be printed to */
/* the second argument gives the address of the first node in the graph */
/* the first function argument prints attributes of the entire graph;
   it may be NULL*/
/* the second function argument prints attributes of the nodes
   like shape or colour; it may be NULL */
/* the third function argument prints attributes of the edges;
   it may be NULL */
/* the last function argument prints the actual node; it should NOT be NULL */
/* currently the format of the output is compatible
   with the xvcg graph layout tool */
extern void print_graph(FILE*, graph_t*,
			void (*)(FILE*),
			void (*)(FILE*, void*),
			void (*)(FILE*, adj_t*),
			void (*)(FILE*, void*));

extern char *graph_colours[];

#endif
