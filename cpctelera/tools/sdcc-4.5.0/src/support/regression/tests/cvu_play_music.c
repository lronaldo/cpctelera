/* cvu_play_music.c
   From the function of the same name in the libcvu library for the ColecoVision
   This file has been included in the tests, since it uses some z80 peephole rules
   not used elsewhere in the regression tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

#include <stdint.h>
#include <stdbool.h>

enum cv_soundchannel {
  CV_SOUNDCHANNEL_0 = 0x0,
  CV_SOUNDCHANNEL_1 = 0x2,
  CV_SOUNDCHANNEL_2 = 0x4,
  CV_SOUNDCHANNEL_NOISE = 0x6
};

struct cvu_music
{
	enum cv_soundchannel channel;
	const uint8_t *volume;
	const uint16_t *tuning;
	uint8_t sixteenth_notes_per_second;
	const uint16_t *notes;
	
	uint16_t note_ticks_remaining;
	uint16_t pause_ticks_remaining;
};

unsigned char cv_get_vint_frequency(void)
{
	return(60);
}

void cv_set_attenuation(enum cv_soundchannel channel, uint8_t dezibel)
{
}

void cv_set_frequency(enum cv_soundchannel channel, uint16_t frequency_divider)
{
}

const uint16_t CVU_TUNING_ISO16_EQUAL[15] = {54719, 51648, 48749, 46013, 43431, 40993, 38692, 36521, 34471, 32536, 30710, 28987, 0, 0, 0};

const uint8_t CVU_VOLUME_DEFAULT[4] = {20, 16, 12, 8};

const uint16_t CVU_EMPTY_MUSIC = 0xffff;

void cvu_init_music(struct cvu_music *music)
{
	music->channel = CV_SOUNDCHANNEL_0;
	music->volume = CVU_VOLUME_DEFAULT;
	music->tuning = CVU_TUNING_ISO16_EQUAL;
	music->sixteenth_notes_per_second = 10;
	music->note_ticks_remaining = 0;
	music->pause_ticks_remaining = 0;
	music->notes = &CVU_EMPTY_MUSIC;
}

#if !defined(__SDCC_pdk14) // Lack of memory
bool cvu_play_music(struct cvu_music *restrict music)
{
	if(music->note_ticks_remaining >= music->sixteenth_notes_per_second)
		music->note_ticks_remaining -= music->sixteenth_notes_per_second;
	else if(music->pause_ticks_remaining >= music->sixteenth_notes_per_second)
	{
		cv_set_attenuation(music->channel, 0);
		music->pause_ticks_remaining -= music->sixteenth_notes_per_second;
	}
	else
	{
		bool pause = false;
		const uint16_t note = *(music->notes);

		cv_set_attenuation(music->channel, 0);

		if(note == 0xffff)
			return(false);

		// Length calculations:
		{
			uint8_t length, rel_length;
			uint16_t leftover_ticks = music->note_ticks_remaining + music->pause_ticks_remaining; // Avoid desynchronization of multi-voice music.

			length = (note >> 4) & 0xf;
			if(!length)
				length = 0x10;
			music->note_ticks_remaining = length * cv_get_vint_frequency();
			music->note_ticks_remaining += leftover_ticks;
			music->note_ticks_remaining -= music->sixteenth_notes_per_second;

			rel_length = (note >> 2) & 0x3;
			switch(rel_length)
			{
			case 0:	// Legato
				break;
			case 1:	// Staccato
				music->pause_ticks_remaining = music->note_ticks_remaining;
				music->note_ticks_remaining = music->note_ticks_remaining >> 2;
				music->pause_ticks_remaining -= music->note_ticks_remaining;
				break;
			case 2:
				music->pause_ticks_remaining = music->note_ticks_remaining >> 1;
				music->note_ticks_remaining -= music->pause_ticks_remaining;
				break;
			default:	// Standard
				music->pause_ticks_remaining = music->note_ticks_remaining >> 2;
				music->note_ticks_remaining -= music->pause_ticks_remaining;
				break;
			}
		}
		
		// Frequency calculations:
		{
			uint8_t octave, halftone;
			uint16_t frequency_divider;
			
			halftone = (note >> 8) & 0xf;
			pause = (halftone == 0xf);
			if(!pause)
			{
				frequency_divider = music->tuning[halftone];

				octave = (note >> 12);
				cv_set_frequency(music->channel, (frequency_divider >> octave) <= 32736 ? frequency_divider >> octave : frequency_divider >> (octave + 1));
			}
		}

		// Loudness calculations:
		{
			cv_set_attenuation(music->channel, pause ? 0 : (music->volume[note & 0x3]));
		}
		music->notes++;
	}
	return(true);
}
#endif

void testBug(void)
{
	struct cvu_music music;
	cvu_init_music(&music);
#if !defined(__SDCC_pdk14) // Lack of memory
	cvu_play_music(&music);
#endif
}

