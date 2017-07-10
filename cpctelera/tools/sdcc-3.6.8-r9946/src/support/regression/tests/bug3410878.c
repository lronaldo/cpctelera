/*
   bug3410878.c

   Dead code elimination fails here, resulting in unconnected CFG and live-ranges.
   Those, in turn did mess up register allocation, which, then resulted in code-generation
   failing. Fixed by workarounds in register allcoation.

*/

#include <testfwk.h>

#pragma std_sdcc99

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct cv_controller_state
{
	uint8_t keypad;
	uint8_t joystick;
	int16_t spinner;
};
#define CV_LEFT 0x08
#define CV_DOWN 0x04
#define CV_RIGHT 0x02
#define CV_UP 0x01

typedef uint16_t cv_vmemp;

#pragma disable_warning 85

extern void cvu_memtovmemcpy(cv_vmemp dest, const void * src, size_t n)
{
}

extern void cvu_vmemtomemcpy(void *dest, cv_vmemp src, size_t n)
{
}

uint8_t *get_tile_data(uint16_t x, uint16_t y)
{
}

#define IMAGE ((const cv_vmemp)0x1c00)
volatile bool step;
volatile struct cv_controller_state cs;

void mainx(void)
{
	uint16_t x, y;
	uint16_t mapx, mapy;
	unsigned char i, j, k;
	uint8_t *b;

	x = 16;
	y = 12;
	mapx = 0;
	mapy = 0;
	for(;;)
	{
		uint8_t buffer[32];

		while(!step);
		step = false;

		if(cs.joystick & CV_RIGHT)
			x++;
		if(cs.joystick & CV_DOWN)
			y++;
		if(cs.joystick & CV_LEFT && x > 8)
			x--;
		if(cs.joystick & CV_UP && y > 8)
			y--;
		// Scroll map		
		if(x > mapx + 24)
		{
			for(i = 0; i < 24; i++)
			{
				cvu_vmemtomemcpy(buffer, IMAGE + i * 32 + 8, 24);
				cvu_memtovmemcpy(IMAGE + i * 32, buffer, 24);
			}
			mapx += 8;
			b = get_tile_data(mapx + 24, mapy);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32 + 24, b + i * 8, 8);
			}
			b = get_tile_data(mapx + 24, mapy + 8);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32 + 32 * 8 + 24, b + i * 8, 8);
			}
			b = get_tile_data(mapx + 24, mapy + 16);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32 + 32 * 16 + 24, b + i * 8, 8);
			}
		}
		if(x < mapx + 8)
		{
			for(i = 0; i < 24; i++)
			{
				cvu_vmemtomemcpy(buffer, IMAGE + i * 32, 24);
				cvu_memtovmemcpy(IMAGE + i * 32 + 8, buffer, 24);
			}
			mapx -= 8;
			b = get_tile_data(mapx, mapy);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32, b + i * 8, 8);
			}
			b = get_tile_data(mapx, mapy + 8);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32 + 32 * 8, b + i * 8, 8);
			}
			b = get_tile_data(mapx, mapy + 16);
			for(i = 0; i < 8; i++)
			{
				cvu_memtovmemcpy(IMAGE + i * 32 + 32 * 16, b + i * 8, 8);
			}
		}
		if(y > mapy + 16)
		{
			for(i = 0; i < 16; i++)
			{
				cvu_vmemtomemcpy(buffer, IMAGE + i * 32 + 8 * 32, 32);
				cvu_memtovmemcpy(IMAGE + i * 32, buffer, 32);
			}
			mapy += 8;
			for(j = 0; j < 32; j += 8)
			{
				b = get_tile_data(mapx + j, mapy + 16);
				for(i = 0; i < 8; i++)
				{
					cvu_memtovmemcpy(IMAGE + i * 32 + 16 * 32 + j, b + i * 8, 8);
				}
			}

		}
		if(y < mapy + 8)
		{
			for(i = 23; i >= 8; i--)
			{
				cvu_vmemtomemcpy(buffer, IMAGE + i * 32 - 8 * 32, 32);
				cvu_memtovmemcpy(IMAGE + i * 32, buffer, 32);
			}
			mapy -= 8;
			for(j = 0; j < 32; j += 8)
			{
				b = get_tile_data(mapx + j, mapy);
				for(i = 0; i < 8; i++)
				{
					cvu_memtovmemcpy(IMAGE + i * 32 + j, b + i * 8, 8);
				}
			}

		}
	}
}

void testBug(void)
{
}

