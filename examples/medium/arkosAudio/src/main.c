//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>
#include "molusk.h" // This file is geneated on compilation from music/molusk.aks

//
// Defined type to know the status of a Key 
//    Key is either Pressed / Released, and K_NOEVENT is used to
//    report that a key is in the same status as in previous checks
//    (Continues pressed or continues released)
//
typedef enum { K_NOEVENT, K_RELEASED, K_PRESSED } TKeyStatus;

////////////////////////////////////////////////////////////////////////
// Checks if a Key has changed from pressed to released or viceversa
// If it has changed, that is considered and event, the status of 
// the key is changed and the concrete event is returned. If it is
// in its previous status, nothing is done and K_NOEVENT is returned
//
TKeyStatus checkKeyEvent(cpct_keyID key, TKeyStatus *keystatus) {
   TKeyStatus newstatus;   // Hold the new status of the key (pressed / released)

   // Check the new status of the key and save it into newstatus
   if ( cpct_isKeyPressed(key) )
      newstatus = K_PRESSED;   // Key is now pressed
   else
      newstatus = K_RELEASED;  // Key is now released

   // Check if newstatus is same or different than previous one
   // If it is different, change key status and report the event
   if (newstatus == *keystatus)
      return K_NOEVENT;       // Same key status, report NO EVENT
   else {
      *keystatus = newstatus; // Status has changed, save it...
      return newstatus;       // And return the new status
   }
}

////////////////////////////////////////////////////////////////////////
// MAIN: Arkos Tracker Music Example
//    Keys:
//       * SPACE - Start / Stop Music
//       *   1   - Play a sound effect on Channel A
//       *   2   - Play a sound effect on Channel C
//
void main(void) {
   TKeyStatus k_space, k_0, k_1;    // Status of the 3 Keys for this example (Space, 1, 2)
   u8  playing   = 1;               // Flag to know if music is playing or not
   u8  color     = 1;               // Color to draw charactes (normal / inverse)
   u8* pvideomem = CPCT_VMEM_START; // Pointer to video memory where next character will be drawn

   // All 3 keys are considered to be released at the start of the program
   k_space = k_0 = k_1 = K_RELEASED;

   // Initialize CPC
   cpct_disableFirmware();    // Disable firmware to prevent interaction
   cpct_setVideoMode(2);      // Set Mode 2 (640x200, 2 colours)
   cpct_setDrawCharM2(1, 0);  // Set Initial colours for drawCharM2 (Foreground/Background)

   // Initialize the song to be played
   cpct_akp_musicInit(molusk_song);    // Initialize the music
   cpct_akp_SFXInit(molusk_song);      // Initialize instruments to be used for SFX (Same as music song)

   while (1) {
      // We have to call the play function 50 times per second (because the song is 
      // designed at 50Hz). We only have to wait for VSYNC and call the play function
      // when the song is not stopped (still playing)
      cpct_waitVSYNC();

      // Check if the music is playing. When it is, do all the things the music
      // requires to be done every 1/50 secs.
      if (playing) {
         cpct_akp_musicPlay();   // Play next music 1/50 step.

         // Write a new number to the screen to see something while playing. 
         // The number will be 0 when music is playing, and 1 when it finishes.
         //  -> If some SFX is playing write the channel where it is playing
         
         // Check if there is an instrument plaing on channel A
         if (cpct_akp_SFXGetInstrument(AY_CHANNEL_A))
            cpct_drawCharM2(pvideomem, 'A'); // Write an 'A' because channel A is playing
         
         // Check if there is an instrument plaing on channel C
         else if (cpct_akp_SFXGetInstrument(AY_CHANNEL_C))
            cpct_drawCharM2(pvideomem, 'C'); // Write an 'C' because channel A is playing 
         
         // No SFX is playing on Channels A or C, write the number of times
         // this song has looped.
         else
            cpct_drawCharM2(pvideomem, '0' + cpct_akp_songLoopTimes);

         // Point to the start of the next character in video memory
         if (++pvideomem >= (u8*)0xC7D0) {
            pvideomem = CPCT_VMEM_START; // When we reach the end of the screen, we return..
            color ^= 1;                  // .. to the start, and change the colour
            cpct_setDrawCharM2(color, color^1); // Set new colour pair for drawCharM2 (inverted from previous one)
         }

         // Check if music has already ended (when looptimes is > 0)
         if (cpct_akp_songLoopTimes > 0)
            cpct_akp_musicInit(molusk_song); // Song has ended, start it again and set loop to 0
      }

      // Check keyboard to let the user play/stop the song with de Space Bar
      // and reproduce some sound effects with keys 1 and 0
      cpct_scanKeyboard_f();

      // When Space is released, stop / continue music
      if ( checkKeyEvent(Key_Space, &k_space) == K_RELEASED ) {
         // Only stop it when it was playing previously
         // No need to call "play" again when continuing, as the
         // change in "playing" status will make the program call "play"
         // again from the next cycle on
         if (playing)
            cpct_akp_stop();
         
         // Change it from playing to not playing and viceversa (0 to 1, 1 to 0)
         playing ^= 1;

      // Check if Key 0 has been released to reproduce a Sound effect on channel A
      } else if ( checkKeyEvent(Key_0, &k_0) == K_RELEASED ) {
         cpct_akp_SFXPlay(13, 15, 36, 20, 0, AY_CHANNEL_A);

      // Check if Key 1 has been released to reproduce a Sound effect on channel C
      } else if ( checkKeyEvent(Key_1, &k_1) == K_RELEASED ) 
         cpct_akp_SFXPlay(3, 15, 60, 0, 40, AY_CHANNEL_C);
   }
}
