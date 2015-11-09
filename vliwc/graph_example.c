#include <stdio.h>
#include <string.h>
/*
  Compile this example like in the following:
  gcc -Wall -g -o graph_example graph_example.c graph.c utils.c
  where you put the three *.c files in the same directory
*/
/*
  In order to use the graph library you have to include graph.h.
*/
#include "graph.h"

/*
  The data field in the graph node is void * such that it can point
  to any type of data.
*/
/*
   Let us assume that the data of the node contains strings, i.e. char *.
*/

/*
  This is the pointer to the graph root.
  The graph root is the last node that is added to the graph.
  Initially, the graph is empty.
*/
graph_t *g = NULL;

/*
  As the graph "knows" that the data it stores in its nodes are void *, in
  order to print a node, the print_graph procedure expects a function that
  takes two arguments: the FILE stream to print to and the void * representing
  the data of one node.
  print_graph is designed to work in general, regardless of the particular
  type of the data of a node.
  However, in each graph instance, we know exactly the type of the data of a
  node. Therefore, in every particular graph instance, the function used to
  print a node won't take a FILE stream and a void * but rather a FILE stream
  and an argument of the type of the data of a node of that particular graph
  instance. In our case, instead of a void *, the data of a node of our graph
  is of type char *. Therefore, the print_node function will look like in the
  following:
*/
void print_node(FILE *f, char *s) {
  fprintf(f, "%s", s);
}

int main(int argc, char *argv[]) {
  char s1[] = "first node",
    s2[] = "second node",
    s3[] = "third node";
  graph_t *first, *second, *third;

  /*
    Let us add a node to the graph.
    The data of the node is the string s1 in this example.
  */
  /*
    We want to keep a pointer to the first added node.
    add_node will return the pointer to the added node.
    It is not mandatory to keep track of the added nodes, it is sufficient
    to keep track only of the root node. However, in this example, we keep
    track of all the nodes. Therefore, we store the return values of add_node
    instead of ignoring them.
  */
  first = add_node(&g, s1);
  /*
    Let's add a second and a third node.
  */
  second = add_node(&g, s2);
  third = add_node(&g, s3);

  /*
    Let us place a directed arc between the first and the second node. The
    arc won't have any label.
  */
  add_arc(&g, s1, s2, NULL);
  /*
    Let us place a directed arc between the first and the third node too. This
    arc won't have any label either.
  */
  add_arc(&g, s1, s3, NULL);

  /*
    Let us have a first surprize: We look for the string "first node" in the
    graph, and we will be surprized not to find it.
  */
  if (contains(g, "first node") == NULL)
    fprintf(stderr, "Surprize! `first node' is not in the graph\n");
  /*
    Why has this happened? Because the graph stores the address of s1 and we
    interrogated whether it stores the address of the string `first node'
    that was passed as an argument. The `contains' function tests the
    _addresses_ and not their _contents_
  */
  /*
    Let us try it again, this time we use `contains_c'.
    `contains_c' demands a function to compare the contents of the address
    stored in a graph node and the contents of the address passed as a second
    argument to contains_c.
    As we are comparing strings, we send `strcmp'.
    However, `strcmp' is a function that expects two const char *, while
    the function that `contains_c' wants is a function that expects two void *.
    Therefore, strcmp has to be cast to the type desired by contains_c.
  */
  if (contains_c(g, "first node", (int (*)(void *, void *))strcmp) != NULL)
    fprintf(stderr, "`contains_c' has found it though\n");

  /*
    Let us check if there exists a directed arc with a given label between
    the first and the third node a given label. Note that the comparison
    is made again at address levels. That is to say that
    are_connected(g, s1, s3, NULL) will return something different from NULL
    but that
    are_connected(g, "first node", "third node", NULL) or
    are_connected(g, "first node", s3, NULL) or
    are_connected(g, s1, "third node", NULL) would return NULL.
  */
  if (are_connected(g, s1, s3, NULL) != NULL)
    fprintf(stderr,
	    "There is a directed arc between the first and the third node\n");

  /*
    Let us see if there exists a directed arc with a given label between the
    third node and the first node.
  */
  if (are_connected(g, s3, s1, NULL) == NULL)
    fprintf(stderr,
	    "There is no directed arc between the third and the first node\n");

  /*
    Let us print the graph on stdout. Don't bother about the first three
    arguments of function type, i.e. the third, fourth and fifth arguments
    passed to print_graph. The last argument is the function that prints
    the data stored in the node. We pass print_node and look also at the
    commentary preceding this function.
    Again, like in the case of strcmp, we cast it.
  */
  print_graph(stdout, g, NULL, NULL, NULL,
	      (void (*)(FILE *, void *))print_node);

  /*
    Once you have the graph in textual form, as resulting from print_graph,
    you can visualize it by means of the xvcg tool.
    Run this program like
    ./graph_example | xvcg -
    or
    ./graph_example > graph.xvcg ; xvcg graph.xvcg
  */
  /*
    Observe that the nodes appear in the reverse order they were added to the
    graph.
  */
  if (third == g)
    fprintf(stderr, "Surprize: `g' equals `third'\n");
  if (third->next_node == second)
    fprintf(stderr, "And the second node appears in the list of nodes _after_ the third node\n");
  if (second->next_node == first)
    fprintf(stderr, "And the first node appears in the list of nodes _after_ the second node\n");

  /*
    Free the memory occupied by the graph.
    flush will NOT attempt to free the memory occupied by the data of the nodes
    Only the graph structure is freed.
  */
  flush(&g);

  return 0;
}
