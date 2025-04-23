/*
   bug-2931.c - segfault in codegen for stack results in shift for pdk.
 */

#include <testfwk.h>

#include <stdbool.h>

struct cvu_huffman_node
{
        unsigned char left;   /* Position of left node in tree or character. */
        unsigned char right;  /* Position of right node in tree or character. */
};

struct cvu_huffman_state
{
	unsigned char (*input)(void);
	const struct cvu_huffman_node *nodes;	/* Array of nodes */
	unsigned char root;	/* Position of root node among nodes */
	unsigned char ls, bs, rs;
	unsigned char bit;	/* Position of currently processed bit */
	unsigned char buffer;	/* Currently processed input byte */
	unsigned char current;	/* Currently processed node for recursive algorithm */
};

unsigned char cvu_get_huffman_recursive(struct cvu_huffman_state *state) __reentrant
{
	bool direction;
	unsigned char ret;

	state->buffer >>= 1;
	if(state->bit == 8)
	{
		state->buffer = (state->input)();
		state->bit = 0;
	}
	direction = state->buffer & 0x01;
	state->bit++;

	if(!direction)	/* Left */
	{
		if(state->current >= state->ls && state->current < state->rs)
		{
			ret = state->nodes[state->current].left;
			state->current = state->root;
		}
		else
		{
			state->current = state->nodes[state->current].left;
			ret = cvu_get_huffman_recursive(state);
		}
	}
	else	/* Right */
	{
		if(state->current >= state->bs)
		{
			ret = state->nodes[state->current].right;
			state->current = state->root;
		}
		else
		{
			state->current = state->nodes[state->current].right;
			ret = cvu_get_huffman_recursive(state);
		}
	}

	return(ret);
}

void testBug(void)
{
}

