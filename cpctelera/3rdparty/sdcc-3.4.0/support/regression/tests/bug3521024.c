#include <testfwk.h>

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
	unsigned char current;	/* Currently processed node */
};

const unsigned char HUFFMAN_ROOT = 124;
const unsigned char HUFFMAN_LS = 125, HUFFMAN_BS = 127, HUFFMAN_RS = 253;
const struct cvu_huffman_node huffman_tree[255] = {

/* Node 0 */ {128, 129},
/* Node 1 */ {130, 131},
/* Node 2 */ {132, 133},
/* Node 3 */ {134, 135},
/* Node 4 */ {136, 137},
/* Node 5 */ {138, 139},
/* Node 6 */ {140, 141},
/* Node 7 */ {142, 143},
/* Node 8 */ {144, 145},
/* Node 9 */ {146, 147},
/* Node 10 */ {148, 149},
/* Node 11 */ {150, 151},
/* Node 12 */ {152, 153},
/* Node 13 */ {154, 155},
/* Node 14 */ {156, 157},
/* Node 15 */ {158, 159},
/* Node 16 */ {160, 161},
/* Node 17 */ {162, 163},
/* Node 18 */ {164, 165},
/* Node 19 */ {166, 167},
/* Node 20 */ {168, 169},
/* Node 21 */ {170, 171},
/* Node 22 */ {172, 173},
/* Node 23 */ {174, 175},
/* Node 24 */ {176, 177},
/* Node 25 */ {178, 179},
/* Node 26 */ {180, 181},
/* Node 27 */ {182, 183},
/* Node 28 */ {184, 185},
/* Node 29 */ {186, 187},
/* Node 30 */ {188, 189},
/* Node 31 */ {190, 191},
/* Node 32 */ {192, 193},
/* Node 33 */ {194, 195},
/* Node 34 */ {196, 197},
/* Node 35 */ {198, 199},
/* Node 36 */ {200, 201},
/* Node 37 */ {202, 203},
/* Node 38 */ {204, 205},
/* Node 39 */ {206, 207},
/* Node 40 */ {208, 209},
/* Node 41 */ {210, 211},
/* Node 42 */ {212, 213},
/* Node 43 */ {214, 215},
/* Node 44 */ {216, 217},
/* Node 45 */ {218, 219},
/* Node 46 */ {220, 221},
/* Node 47 */ {222, 223},
/* Node 48 */ {224, 225},
/* Node 49 */ {226, 227},
/* Node 50 */ {228, 229},
/* Node 51 */ {230, 231},
/* Node 52 */ {232, 233},
/* Node 53 */ {234, 235},
/* Node 54 */ {236, 237},
/* Node 55 */ {238, 239},
/* Node 56 */ {240, 241},
/* Node 57 */ {242, 243},
/* Node 58 */ {244, 245},
/* Node 59 */ {246, 247},
/* Node 60 */ {248, 249},
/* Node 61 */ {125, 0},
/* Node 62 */ {1, 2},
/* Node 63 */ {3, 4},
/* Node 64 */ {5, 6},
/* Node 65 */ {7, 8},
/* Node 66 */ {9, 10},
/* Node 67 */ {11, 12},
/* Node 68 */ {13, 14},
/* Node 69 */ {15, 16},
/* Node 70 */ {17, 18},
/* Node 71 */ {19, 20},
/* Node 72 */ {21, 22},
/* Node 73 */ {23, 24},
/* Node 74 */ {25, 26},
/* Node 75 */ {27, 28},
/* Node 76 */ {29, 30},
/* Node 77 */ {31, 32},
/* Node 78 */ {33, 34},
/* Node 79 */ {35, 36},
/* Node 80 */ {37, 38},
/* Node 81 */ {39, 40},
/* Node 82 */ {41, 42},
/* Node 83 */ {43, 44},
/* Node 84 */ {45, 46},
/* Node 85 */ {47, 48},
/* Node 86 */ {49, 50},
/* Node 87 */ {51, 52},
/* Node 88 */ {53, 54},
/* Node 89 */ {55, 56},
/* Node 90 */ {57, 58},
/* Node 91 */ {59, 60},
/* Node 92 */ {61, 62},
/* Node 93 */ {63, 64},
/* Node 94 */ {65, 66},
/* Node 95 */ {67, 68},
/* Node 96 */ {69, 70},
/* Node 97 */ {71, 72},
/* Node 98 */ {73, 74},
/* Node 99 */ {75, 76},
/* Node 100 */ {77, 78},
/* Node 101 */ {79, 80},
/* Node 102 */ {81, 82},
/* Node 103 */ {83, 84},
/* Node 104 */ {85, 86},
/* Node 105 */ {87, 88},
/* Node 106 */ {89, 90},
/* Node 107 */ {91, 92},
/* Node 108 */ {93, 94},
/* Node 109 */ {95, 96},
/* Node 110 */ {97, 98},
/* Node 111 */ {99, 100},
/* Node 112 */ {101, 102},
/* Node 113 */ {103, 104},
/* Node 114 */ {105, 106},
/* Node 115 */ {107, 108},
/* Node 116 */ {109, 110},
/* Node 117 */ {111, 112},
/* Node 118 */ {113, 114},
/* Node 119 */ {115, 116},
/* Node 120 */ {117, 118},
/* Node 121 */ {119, 120},
/* Node 122 */ {253, 250},
/* Node 123 */ {251, 252},
/* Node 124 */ {254, 126},
/* Node 125 */ {255, 127},
/* Node 126 */ {170, 123},
/* Node 127 */ {0, 5},
/* Node 128 */ {6, 7},
/* Node 129 */ {8, 9},
/* Node 130 */ {10, 11},
/* Node 131 */ {12, 16},
/* Node 132 */ {17, 18},
/* Node 133 */ {19, 20},
/* Node 134 */ {21, 22},
/* Node 135 */ {23, 24},
/* Node 136 */ {25, 26},
/* Node 137 */ {27, 28},
/* Node 138 */ {29, 30},
/* Node 139 */ {31, 32},
/* Node 140 */ {33, 34},
/* Node 141 */ {35, 36},
/* Node 142 */ {37, 38},
/* Node 143 */ {39, 40},
/* Node 144 */ {41, 42},
/* Node 145 */ {43, 44},
/* Node 146 */ {45, 46},
/* Node 147 */ {47, 48},
/* Node 148 */ {49, 50},
/* Node 149 */ {51, 52},
/* Node 150 */ {53, 54},
/* Node 151 */ {55, 56},
/* Node 152 */ {57, 58},
/* Node 153 */ {59, 60},
/* Node 154 */ {61, 62},
/* Node 155 */ {63, 64},
/* Node 156 */ {65, 66},
/* Node 157 */ {67, 68},
/* Node 158 */ {69, 70},
/* Node 159 */ {71, 72},
/* Node 160 */ {73, 74},
/* Node 161 */ {75, 76},
/* Node 162 */ {77, 78},
/* Node 163 */ {79, 80},
/* Node 164 */ {81, 82},
/* Node 165 */ {83, 84},
/* Node 166 */ {86, 87},
/* Node 167 */ {88, 89},
/* Node 168 */ {90, 91},
/* Node 169 */ {92, 93},
/* Node 170 */ {94, 95},
/* Node 171 */ {96, 97},
/* Node 172 */ {98, 99},
/* Node 173 */ {100, 101},
/* Node 174 */ {102, 103},
/* Node 175 */ {104, 105},
/* Node 176 */ {106, 107},
/* Node 177 */ {108, 109},
/* Node 178 */ {110, 111},
/* Node 179 */ {112, 113},
/* Node 180 */ {114, 115},
/* Node 181 */ {116, 117},
/* Node 182 */ {118, 119},
/* Node 183 */ {120, 121},
/* Node 184 */ {122, 123},
/* Node 185 */ {124, 125},
/* Node 186 */ {126, 127},
/* Node 187 */ {128, 129},
/* Node 188 */ {130, 131},
/* Node 189 */ {132, 133},
/* Node 190 */ {134, 135},
/* Node 191 */ {136, 137},
/* Node 192 */ {138, 139},
/* Node 193 */ {140, 141},
/* Node 194 */ {142, 143},
/* Node 195 */ {144, 145},
/* Node 196 */ {146, 147},
/* Node 197 */ {148, 149},
/* Node 198 */ {150, 151},
/* Node 199 */ {152, 153},
/* Node 200 */ {154, 155},
/* Node 201 */ {156, 157},
/* Node 202 */ {158, 159},
/* Node 203 */ {160, 161},
/* Node 204 */ {162, 163},
/* Node 205 */ {164, 165},
/* Node 206 */ {166, 167},
/* Node 207 */ {168, 169},
/* Node 208 */ {171, 172},
/* Node 209 */ {173, 174},
/* Node 210 */ {175, 176},
/* Node 211 */ {177, 178},
/* Node 212 */ {179, 180},
/* Node 213 */ {181, 182},
/* Node 214 */ {183, 184},
/* Node 215 */ {185, 186},
/* Node 216 */ {187, 188},
/* Node 217 */ {189, 190},
/* Node 218 */ {191, 192},
/* Node 219 */ {193, 194},
/* Node 220 */ {195, 196},
/* Node 221 */ {197, 198},
/* Node 222 */ {199, 200},
/* Node 223 */ {201, 202},
/* Node 224 */ {203, 204},
/* Node 225 */ {205, 206},
/* Node 226 */ {207, 208},
/* Node 227 */ {209, 210},
/* Node 228 */ {211, 212},
/* Node 229 */ {213, 214},
/* Node 230 */ {215, 216},
/* Node 231 */ {217, 218},
/* Node 232 */ {219, 220},
/* Node 233 */ {221, 222},
/* Node 234 */ {223, 224},
/* Node 235 */ {225, 226},
/* Node 236 */ {227, 228},
/* Node 237 */ {229, 230},
/* Node 238 */ {231, 232},
/* Node 239 */ {233, 234},
/* Node 240 */ {235, 236},
/* Node 241 */ {237, 238},
/* Node 242 */ {239, 240},
/* Node 243 */ {241, 242},
/* Node 244 */ {243, 244},
/* Node 245 */ {245, 246},
/* Node 246 */ {247, 248},
/* Node 247 */ {249, 250},
/* Node 248 */ {251, 252},
/* Node 249 */ {253, 254},
/* Node 250 */ {2, 3},
/* Node 251 */ {4, 13},
/* Node 252 */ {14, 15},
/* Node 253 */ {121, 1},
/* Node 254 */ {122, 85}};

const unsigned char data[] = {72, 60, 102, 102, 123, 15};
const unsigned char udata[] = {0x01, 0x02, 0x03, 0x04, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x0d, 0x0e, 0x0f};

unsigned char get_data(void)
{
  static int i;
  return(data[i++]);
}

unsigned char get_data2(void)
{
  static int i;
  return(data[i++]);
}

unsigned char huffman_recursive(struct cvu_huffman_state *state)
{
	unsigned char direction;
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
			ret = huffman_recursive(state);
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
			ret = huffman_recursive(state);
		}
	}

	return(ret);
}

unsigned char huffman_iterative(struct cvu_huffman_state *state)
{
	unsigned char current;
	unsigned char ret;

	current = state->root;

	for(;;)
	{
		state->buffer >>= 1;
		if(state->bit == 8)
		{
			state->buffer = (state->input)();
			state->bit = 0;
		}
		state->bit++;

		if(!(state->buffer & 0x01))	/* Left */
		{
			if(current >= state->ls && current < state->rs)
			{
				ret = state->nodes[current].left;
				break;
			}

			current = state->nodes[current].left;
		}
		else	/* Right */
		{
			if(current >= state->bs)
			{
				ret = state->nodes[current].right;
				break;
			}

			current = state->nodes[current].right;
		}
	}
	
	return(ret);
}

void init_huffman(struct cvu_huffman_state *state, unsigned char (*input)(void), const struct cvu_huffman_node *tree, unsigned char root, unsigned char ls, unsigned char bs, unsigned char rs)
{
	state->input = input;
	state->nodes = tree;
	state->root = root;
	state->ls = ls;
	state->bs = bs;
	state->rs = rs;
	state->bit = 8;
	state->current = state->root;
}

void testBug(void)
{
  unsigned char i;
  struct cvu_huffman_state state;

  init_huffman(&state, &get_data, huffman_tree, HUFFMAN_ROOT, HUFFMAN_LS, HUFFMAN_BS, HUFFMAN_RS);

  for(i = 0; i < 15; i++)
    ASSERT(huffman_recursive(&state) == udata[i]);

  init_huffman(&state, &get_data2, huffman_tree, HUFFMAN_ROOT, HUFFMAN_LS, HUFFMAN_BS, HUFFMAN_RS);

  for(i = 0; i < 15; i++)
    ASSERT(huffman_iterative(&state) == udata[i]);
}

