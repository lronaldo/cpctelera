/*-------------------------------------------------------------------------

  graph.c - implementation of general graphs
  
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

/* $Id: graph.c 9490 2016-01-31 11:44:32Z molnarkaroly $ */

#include "graph.h"

/* === helpers ====================================================== */

int default_compare (void *data1, void *data2)
{
  return (data1 == data2);
}

/* === GraphEdge ==================================================== */

GraphEdge *newGEdge (GraphNode *src, GraphNode *dest, unsigned int weight) {
  GraphEdge *edge = (GraphEdge *)Safe_alloc(sizeof(GraphEdge));
  edge->src = src;
  edge->node = dest;
  edge->weight = weight;
  return edge;
}

GraphEdge *deleteGEdge (GraphEdge *edge) {
  GraphEdge *head;
  // remove edge from list
  if (edge->next) edge->next->prev = edge->prev;
  if (edge->prev) edge->prev->next = edge->next;

  if (edge->prev) head = edge->prev; else head = edge->next;
  Safe_free (edge);
  return head;
}

/* === GraphNode ==================================================== */

GraphNode *newGNode (void *data, hash_t hash) {
  GraphNode *node = (GraphNode*)Safe_alloc(sizeof(GraphNode));
  node->data = data;
  node->hash = hash;
  return node;
}

GraphNode * deleteGNode (GraphNode *node) {
  GraphNode *head;

  if (!node) return NULL;

  // delete all edges
  while (node->edge) {
    node->edge = deleteGEdge (node->edge);
  } // while

  // remove node from list
  if (node->next) node->next->prev = node->prev;
  if (node->prev) node->prev->next = node->next;

  if (node->prev) head = node->prev; else head = node->next;
  Safe_free (node);
  return head;
}

GraphEdge *addGEdge (GraphNode *from, GraphNode *to, unsigned int weight) {
  GraphEdge *edge = getGEdge (from, to);
  if (edge == NULL) {
    edge = newGEdge (from, to, weight);
    // insert edge into list
    if (from->edge) from->edge->prev = edge;
    edge->next = from->edge;
    from->edge = edge;
  } else
    edge->weight += weight;

  assert (edge->src == from && edge->node == to);
  return edge;
}

void addGEdge2 (GraphNode *from, GraphNode *to, unsigned int weight, unsigned int weight_back) {
  addGEdge (from, to, weight);
  addGEdge (to, from, weight_back);
}

void remGEdge (GraphNode *from, GraphNode *to) {
  GraphEdge *curr = from->edge;
  while (curr && curr->node != to) curr = curr->next;

  if (!curr) return;
  
  if (from->edge == curr)
    from->edge = deleteGEdge (curr);
  else
    deleteGEdge (curr);
}

GraphEdge *getGEdge (GraphNode *from, GraphNode *to) {
  GraphEdge *curr = from->edge;
  while (curr && curr->node != to) {
    assert (curr->src == from);
    curr = curr->next;
  }
  return curr;
}

/* === Graph ======================================================== */

Graph *newGraph (Graph_compareData *compare) {
  Graph *graph = (Graph*)Safe_alloc(sizeof(Graph));
  graph->compare = compare;
  if (!compare) graph->compare = default_compare;
  
  return graph;
}

void deleteGraph (Graph *graph) {
  // remove all nodes
  while (graph->node) {
    graph->node = deleteGNode (graph->node);
  } // while

  Safe_free (graph);
}

GraphNode *addGNode (Graph *graph, void *data, hash_t hash) {
  GraphNode *node = newGNode (data, hash);
  if (graph->node) graph->node->prev = node;
  node->next = graph->node;
  graph->node = node;
  return node;
}

void remGNode (Graph *graph, void *data, hash_t hash) {
  GraphNode *curr = graph->node;
  while (curr && ((curr->hash != hash) || (!graph->compare(curr->data, data)))) {
    curr = curr->next;
  } // while

  if (!curr) return;

  if (graph->node == curr)
    graph->node = deleteGNode (curr);
  else
    deleteGNode (curr);
}

GraphNode *getGNode (Graph *graph, void *data, hash_t hash) {
  GraphNode *curr = graph->node;
  while (curr && ((curr->hash != hash) || (!graph->compare(curr->data, data)))) {
    curr = curr->next;
  } // while

  return curr;
}

GraphNode *getOrAddGNode (Graph *graph, void *data, hash_t hash) {
  GraphNode *curr = getGNode (graph, data, hash);
  if (!curr)
    curr = addGNode (graph, data, hash);

  assert (curr != NULL);
  return curr;
}
