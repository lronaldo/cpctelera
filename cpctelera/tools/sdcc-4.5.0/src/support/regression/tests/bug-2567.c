/* bug-2567.c
   A bug resulting in overlapping spill locations in temporary variables.
 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t cv_vmemp;

#define TILE_SITE0 224

enum tools
{
	TOOL_PATH_LR,
	TOOL_PATH_UD,
	TOOL_PATH_LU,
	TOOL_PATH_UR,
	TOOL_PATH_RD,
	TOOL_PATH_DL,
	TOOL_RAIL_LR,
	TOOL_RAIL_UD,
	TOOL_RAIL_LU,
	TOOL_RAIL_UR,
	TOOL_RAIL_RD,
	TOOL_RAIL_DL,
	TOOL_NODE,
	TOOL_UPGRADE,
	TOOL_DESTROY,
	TOOL_INVALID
};

struct site
{
	uint_fast8_t pos[2];
	uint_fast8_t built;
	enum tools tool;
};

struct site sites[1];

inline cv_vmemp tile_at(uint_fast8_t x, uint_fast8_t y)
{
	return(x + y * 32);
}

uint_fast8_t free_workers;
uint_fast8_t workers;

void cvu_voutb(const uint8_t value, const cv_vmemp dest)
{
	(void)value;
	ASSERT(dest == tile_at(0, 1) || dest == tile_at(1, 1) || dest == tile_at(0, 2) || dest == tile_at(1, 2));
}

void build(void)
{
	free_workers = workers;
	for(struct site *s = sites; s < sites + 1; s++)
	{
		uint_fast8_t x;
		uint_fast8_t y;

		if(!free_workers)
			return;

		if(s->tool == UINT8_MAX)
			continue;

		x = s->pos[0];
		y = s->pos[1];

		free_workers--;

		{
			uint_fast8_t progress = s->built / 32;
			bool big = (s->tool >= TOOL_RAIL_LU);

			cvu_voutb(TILE_SITE0 + progress, tile_at(x, y));
			if(big)
			{
				cvu_voutb(TILE_SITE0 + progress, tile_at(x + 1, y));
				cvu_voutb(TILE_SITE0 + progress, tile_at(x, y + 1));
				cvu_voutb(TILE_SITE0 + progress, tile_at(x + 1, y + 1));
			}
		}
	}
}

void testBug(void)
{
	workers = 1;

	sites[0].pos[0] = 0;
	sites[0].pos[1] = 1;
	sites[0].tool = TOOL_NODE;
	sites[0].built = 0;

	build();
}

extern cv_vmemp tile_at(uint_fast8_t x, uint_fast8_t y);

