#include "graph.h"
#include "utils.h"
#include "exit_codes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char *graph_colours[] = {
  "black",
  "red",
  "green",
  "yellow",
  "cyan",
  "blue",
  "orange",


  "lightred",
  "lightgreen",
  "lightblue",
  "lightyellow",
  "lightgrey",
  "lightcyan",

  "darkred",
  "darkgreen",
  "darkblue",
  "darkyellow",
  "darkgrey",
  "darkcyan",

  "magenta",
  "gold",
  "khaki",
  "lilac",

  "purple",
  "yellowgreen",
  "turquoise",

  "darkmagenta",
  "orchid",
  "lightmagenta",
  "pink"
};

static graph_t *alloc_node(void* d) {
  graph_t *t;

  if ((t = (graph_t*)malloc(sizeof(graph_t))) == NULL)
    fatal("alloc_node, malloc", SYS_CALL_ERR);

  t->to = NULL;
  t->from = NULL;
  t->prev_node = NULL;
  t->next_node = NULL;
  t->data = d;

  return t;
}

static adj_t *alloc_arc(graph_t *n, void *l) {
  adj_t *q;

  if ((q = (adj_t*)malloc(sizeof(adj_t))) == NULL)
    fatal("alloc_arc, malloc", SYS_CALL_ERR);

  q->next_arc = NULL;
  q->prev_arc = NULL;
  q->conjug = NULL;
  q->node = n;
  q->label = l;

  return q;
}

graph_t *contains(graph_t *g, void* d) {
  graph_t *q;

  for (q = g; q != NULL; q = q->next_node)
    if (q->data == d)
      return q;
  return NULL;
}

graph_t *contains_c(graph_t *g, void* d, int (*cmp_func)(void*, void*)) {
  graph_t *q;

  for (q = g; q != NULL; q = q->next_node)
    if (cmp_func(q->data, d) == 0)
      return q;
  return NULL;
}

graph_t *first_that(int (*cmp_func)(void*, va_list), graph_t *g, ...) {
  graph_t *q;
  va_list v;

  for (q = g; q != NULL; q = q->next_node) {
    va_start(v, g);
    if (cmp_func(q->data, v) == 0)
      return q;
    va_end(v);
  }
  return NULL;
}

adj_t *are_connected(graph_t *g, void* d1, void* d2, void *l) {
  graph_t *n1, *n2;

  if ((n1 = contains(g, d1)) == NULL)
    return NULL;
  if ((n2 = contains(g, d2)) == NULL)
    return NULL;

  return _are_connected(n1, n2, l);
}

adj_t *_are_connected(graph_t *n1, graph_t *n2, void *l) {
  adj_t *q;

  for (q = n1->to; q != NULL; q = q->next_arc)
    if (q->node == n2 && q->label == l)
      return q;
  return NULL;
}

graph_t *add_node(graph_t **g, void* d) {
  graph_t *t;

  if ((t = contains(*g, d)) != NULL)
    return t;

  t = alloc_node(d);
  return _add_node(g, t);
}

graph_t *_add_node(graph_t **g, graph_t *n) {
  if (*g == NULL) {
    *g = n;
    return n;
  }

  n->next_node = *g;
  (*g)->prev_node = n;
  *g = n;
  return n;
}

adj_t *add_arc(graph_t **g, void* d1, void* d2, void *l) {
  graph_t *t1, *t2;

  t1 = add_node(g, d1);
  t2 = add_node(g, d2);

  return _add_arc(t1, t2, l);  
}

adj_t *_add_arc(graph_t *n1, graph_t *n2, void *l) {
  adj_t *q1, *q2;

  if ((q1 = _are_connected(n1, n2, l)) != NULL)
    return q1;

  q1 = alloc_arc(n2, l);
  q2 = alloc_arc(n1, l);
  q1->conjug = q2;
  q2->conjug = q1;

  if (n1->to == NULL)
    n1->to = q1;
  else {
    q1->next_arc = n1->to;
    n1->to->prev_arc = q1;
    n1->to = q1;
  }

  if (n2->from == NULL)
    n2->from = q2;
  else {
    q2->next_arc = n2->from;
    n2->from->prev_arc = q2;
    n2->from = q2;
  }

  return q1;
}

void remove_node(graph_t **g, void* d) {
  graph_t *n;

  if ((n = contains(*g, d)) == NULL)
    return;

  _remove_node(g, n);
}

void _remove_node(graph_t **g, graph_t *n) {
  adj_t *q, *aux;

  for (q = n->to; q != NULL;) {
    aux = q;
    q = q->next_arc;
    __remove_arc(aux);
  }

  for (q = n->from; q != NULL;) {
    aux = q;
    q = q->next_arc;
    __remove_arc(aux->conjug);
  }

  if (n->next_node != NULL)
    n->next_node->prev_node = n->prev_node;
  if (n->prev_node != NULL)
    n->prev_node->next_node = n->next_node;
  else
    *g = n->next_node;
  free(n);
}

void flush(graph_t **g){
  graph_t *q, *aux;

  for (q = *g; q != NULL;) {
    aux = q->next_node;
    _remove_node(g, q);
    q = aux;
  }
}

void remove_arc(graph_t *g, void* d1, void* d2, void *l) {
  graph_t *n1, *n2;

  if ((n1 = contains(g, d1)) == NULL)
    return;
  if ((n2 = contains(g, d2)) == NULL)
    return;
  _remove_arc(n1, n2, l);
}

void _remove_arc(graph_t *n1, graph_t *n2, void *l) {
  adj_t *q;

  if ((q = _are_connected(n1, n2, l)) == NULL)
    return;

  __remove_arc(q);
}

void __remove_arc(adj_t *q) {
  adj_t *conjug;

  conjug = q->conjug;

  if (conjug->next_arc != NULL)
    conjug->next_arc->prev_arc = conjug->prev_arc;
  if (conjug->prev_arc != NULL)
    conjug->prev_arc->next_arc = conjug->next_arc;
  else
    q->node->from = conjug->next_arc;
  
  if (q->next_arc != NULL)
    q->next_arc->prev_arc = q->prev_arc;
  if (q->prev_arc != NULL)
    q->prev_arc->next_arc = q->next_arc;
  else
    conjug->node->to = q->next_arc;

  free(conjug);
  free(q);
}

void print_graph(FILE *f, graph_t *g,
		 void (*graph_attrib)(FILE*),
		 void (*node_attrib)(FILE*, void*),
		 void (*edge_attrib)(FILE*, adj_t*),
		 void (*print_node)(FILE*, void*)) {
  graph_t *q;
  adj_t *w;

  fprintf(f, "graph: {\n");
  fprintf(f, "\tport_sharing: no\n");
  if (graph_attrib != NULL)
    graph_attrib(f);
  for (q = g; q != NULL; q = q->next_node) {
    fprintf(f, "\tnode: {\n");
    fprintf(f, "\t\ttitle: \"");
    print_node(f, q->data);
    fprintf(f, "\"\n");
    if (node_attrib != NULL)
      node_attrib(f, q->data);
    fprintf(f, "\t}\n");
  }
  for (q = g; q != NULL; q = q->next_node) {
    for (w = q->to; w != NULL; w = w->next_arc) {
      fprintf(f, "\tedge: {\n");
      fprintf(f, "\t\tsourcename: \"");
      print_node(f, q->data);
      fprintf(f, "\"\n");
      fprintf(f, "\t\ttargetname: \"");
      print_node(f, w->node->data);
      fprintf(f, "\"\n");
      if (edge_attrib != NULL)
	edge_attrib(f, w);
      fprintf(f, "\t}\n");
    }
  }
  fprintf(f, "}");
}
