/*-------------------------------------------------------------------------

  graph.h - header file for graph.c
  
   Written By -  Raphael Neider <rneider AT web.de> (2005)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/

/* $Id: graph.h 4781 2007-04-29 20:33:44Z borutr $ */

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "../common.h"

typedef unsigned int hash_t;

struct GraphNode;

typedef struct GraphEdge {
  struct GraphNode *src;        // starting node of this edge
  struct GraphNode *node;       // other end of this edge
  unsigned int weight;          // weight assigned to this edge
  struct GraphEdge *prev;       // link to previous edge
  struct GraphEdge *next;       // link to next edge
} GraphEdge;

typedef struct GraphNode {
  void *data;                   // data stored in this node
  hash_t hash;                  // hash value for "data"
  
  GraphEdge *edge;              // first edge leaving this node
  struct GraphNode *prev;       // link to previous node
  struct GraphNode *next;       // link to next edge
} GraphNode;

// compare function, returns 0 for different items and 1 for equal items
typedef int Graph_compareData(void *item1, void *item2);

typedef struct {
  GraphNode *node;              // first node in this graph
  Graph_compareData *compare;   // function used to compare two data items
} Graph;

/* Create a new edge from src to dest.
 * Returns a pointer to the new edge. */
GraphEdge *newGEdge (GraphNode *src, GraphNode *dest, unsigned int weight);

/* Delete an edge and remove it from the containing list.
 * Returns a pointer to the previous edge or (if there is NULL) to its successor. */
GraphEdge *deleteGEdge (GraphEdge *edge);



/* Create a new node. */
GraphNode *newGNode (void *data, hash_t hash);

/* Delete a node and all its edges. this also removes the node
 * from its containing list.
 * Returns the previous node in the list or (if there is NULL)
 * its successor. */
GraphNode *deleteGNode (GraphNode *node);

/* Adds an edge with the given weight. If the edge already exists,
 * its weight its increased instead! */
GraphEdge *addGEdge (GraphNode *from, GraphNode *to, unsigned int weight);

/* Adds the edges (from,to) and (to,from) with the specified weights. */
void addGEdge2 (GraphNode *from, GraphNode *to, unsigned int weight, unsigned int weight_back);

/* Remove an edge from the node. This deletes the edge and updates the 
 * list of edges attached to the "from" node. */
void remGEdge (GraphNode *from, GraphNode *to);

/* Returns the edge (from,to) or NULL if no such edge exists. */
GraphEdge *getGEdge (GraphNode *from, GraphNode *to);



/* Create a new graph which uses the given compare function to test
 * its nodes' data for equality. */
Graph *newGraph (Graph_compareData *compare);

/* Delete a graph, all its contained nodes and their edges. */
void deleteGraph (Graph *graph);

/* Add a node to the graph. */
GraphNode *addGNode (Graph *graph, void *data, hash_t hash);

/* Remove a node from the graph. This also deletes the node and all
 * its associated (outbound) edges. */
void remGNode (Graph *graph, void *data, hash_t hash);

/* Returns the specified node or NULL if no such node exists. */
GraphNode *getGNode (Graph *graph, void *data, hash_t hash);

/* Returns the specified node (after creating it if neccessary). */
GraphNode *getOrAddGNode (Graph *graph, void *data, hash_t hash);

#endif
