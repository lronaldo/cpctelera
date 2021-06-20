/*
   bug3565757.c
*/

#include <testfwk.h>

#pragma std_c99
#pragma disable_warning 85

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef uint_least16_t edge_index_t;

struct edge_t
{
	uint_fast8_t u[2];
	uint_fast8_t v[2];
	edge_index_t prev;
	edge_index_t next;
	bool in_stix;
};

struct edge_t edges[4];

void init_edges(void)
{
}

void get_edge(struct edge_t *const e, const edge_index_t e_i)
{
}

void set_edge(const edge_index_t e_i, const struct edge_t *const e)
{
	memcpy(edges + e_i, e, sizeof(struct edge_t));
}

const uint_fast8_t vertices[4][2] = {{7, 188}, {248, 188}, {248, 9}, {7, 9}};

struct edge_t edge_cache;

void init_geometry(void)
{
	edge_index_t i;
	init_edges();

	for(i = 0; i < 4; i++)
	{
		get_edge(&edge_cache, i);
		edge_cache.u[0] = vertices[(i + 0) % 4][0];
		edge_cache.u[1] = vertices[(i + 0) % 4][1];
		edge_cache.v[0] = vertices[(i + 1) % 4][0];
		edge_cache.v[1] = vertices[(i + 1) % 4][1];
		edge_cache.prev = (i + 3) % 4;
		edge_cache.next = (i + 1) % 4;
		set_edge(i, &edge_cache);
	}
}

void testBug(void)
{
	init_geometry();

	ASSERT(edges[0].prev = 3);
	ASSERT(edges[3].prev = 2);
}

