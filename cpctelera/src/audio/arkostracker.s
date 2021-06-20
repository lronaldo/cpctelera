;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2009 Targhan / Arkos
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU Lesser General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WArraNTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_audio

;;
;; Title: Arkos Player Functions
;;

;;------------------------------------------------------------------------------------------------------
;;--- PLAYER CONFIGURATION CONSTANTS
;;------------------------------------------------------------------------------------------------------

;;
;; Constants: Arkos Player Compilation Constants
;;
;;    Constants used to control which features are enabled / disabled in the Arkos Tracker
;; Player code. Changing them requires recompiling CPCtelera's library to take effect.
;;
;;    PLY_UseSoundEffects - Set to 1 if you want to use Sound Effects in your player. 
;; Both CPU and memory consuming. It is set to 1 by default in CPCtelera.
;;    PLY_UseFades        - Set to 1 to allow fades in / out. A little CPU and memory 
;; consuming. _cpct_akp_setFadeVolume becomes available.
;;    PLY_SystemFriendly  - Set to 1 if you want to save the Registers used by AMSDOS 
;; (AF', BC', IX, IY) which allows you to call this player in BASIC. As this option is 
;; system-friendly, it cuts interruptions, and restores them ONLY IF NECESSARY. It is set
;; to 0 by default. However, IX and IY registers are always saved.
;;    PLY_RetrigValue     - Value used to trigger the Re-trig of Register 13. 0xFE 
;; corresponds to 'CP xx'. Do not change it!
;;
.equ PLY_UseSoundEffects, 1
.equ PLY_UseFades       , 0
.equ PLY_SystemFriendly , 0
.equ PLY_RetrigValue    , #0xFE

;;------------------------------------------------------------------------------------------------------
;;--- PLAYER CODE START
;;------------------------------------------------------------------------------------------------------

;; Digidrum Status
;;    Read here to know if a Digidrum has been played (0=no).
_cpct_akp_digidrumStatus:: .db 0

;; Loop times
;;    Read here to know the number of times a song has looped
_cpct_akp_songLoopTimes:: .db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_akp_musicPlay
;;
;;    Plays next music cycle of the present song with Arkos Tracker Player. Song 
;; has had to be previously established with <cpct_akp_musicInit>.
;;
;; C Definition:
;;    void <cpct_akp_musicPlay> ()
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_akp_musicPlay_asm
;;
;; Known limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function is to be called to start and continue playing the presently 
;; selected song with Arkos Tracker Player. Depending on the frequency at which 
;; the song were created, this function should be called 12, 25, 50, 100, 200 
;; or 300 times per second. 
;;
;;    Each time you call the function, it plays 1/frequency seconds. This means
;; that you have to manually synchronize your calls to this function to have
;; a stable music playing. If you call too fast or too slow you will either 
;; interrupt sound or have sound valleys. Therefore, you are responsible for
;; calling this function with the most accurate timing possible, to get best 
;; sound results.
;;
;; Destroyed Register values: 
;;    AF, AF', BC, BC', DE, DE', HL, HL'
;;
;; Required memory:
;;    1794 bytes 
;;
;;    However, take into account that all of Arkos Tracker Player's
;; functions are linked and included, because they depend on each other. Total
;; memory requirement is around 2089 bytes.
;;
;; Time Measures:
;; (start code)
;;    To be done
;; (end code)
;;
;; Credits:
;;    This is a modification of the original <Arkos Tracker Player at
;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
;; contributed to this source.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_akp_musicPlay::
cpct_akp_musicPlay_asm::   ;; Entry point for assembly calls 
PLY_Play:

;***** Player System Friendly has to restore registers *****
.if PLY_SystemFriendly
   call PLY_DisableInterruptions
   ex   af, af'
   exx
   push af
   push bc
.endif
   push ix
   push iy

   xor  a
   ld   (_cpct_akp_digidrumStatus), a     ;Reset the Digidrum flag.

;Manage Speed. If Speed counter is over, we have to read the Pattern further.
PLY_SpeedCpt:
   ld   a, #1
   dec  a
   jp  nz, PLY_SpeedEnd

;Moving forward in the Pattern. Test if it is not over.
PLY_HeightCpt: 
   ld   a, #1
   dec  a
   jr  nz, PLY_HeightEnd

;Pattern Over. We have to read the Linker.

;Get the Transpositions, if they have changed, or detect the Song Ending !
PLY_Linker_PT: 
   ld  hl, #0
   ld   a, (hl)
   inc hl
   rra
   jr  nc, PLY_SongNotOver

   ;Song over ! 
   
   ;; Increment song loop times
   ld    a, (_cpct_akp_songLoopTimes)
   inc   a
   ld (_cpct_akp_songLoopTimes), a

   ;; We read the address of the Loop point.
   ld   a, (hl)
   inc hl
   ld   h, (hl)
   ld   l, a
   ld   a, (hl)   ;We know the Song won't restart now, so we can skip the first bit.
   inc hl
   rra

PLY_SongNotOver:
   rra
   jr  nc, PLY_NoNewTransposition1
   ld  de, #PLY_Transposition1 + 1
   ldi

PLY_NoNewTransposition1:
   rra
   jr  nc, PLY_NoNewTransposition2
   ld  de, #PLY_Transposition2 + 1
   ldi 

PLY_NoNewTransposition2:
   rra
   jr  nc, PLY_NoNewTransposition3
   ld  de, #PLY_Transposition3 + 1
   ldi

PLY_NoNewTransposition3:
   ;Get the Tracks addresses.
   ld  de, #PLY_Track1_PT + 1
   ldi
   ldi
   ld  de, #PLY_Track2_PT + 1
   ldi
   ldi
   ld  de, #PLY_Track3_PT + 1
   ldi
   ldi

   ;Get the Special Track address, if it has changed.
   rra
   jr  nc, PLY_NoNewHeight
   ld  de, #PLY_Height + 1
   ldi

PLY_NoNewHeight:
   rra
   jr  nc, PLY_NoNewSpecialTrack

PLY_NewSpecialTrack:
   ld   e, (hl)
   inc hl
   ld   d, (hl)
   inc hl
   ld  (PLY_SaveSpecialTrack + 1), de

PLY_NoNewSpecialTrack:
   ld  (PLY_Linker_PT + 1), hl

PLY_SaveSpecialTrack:
   ld  hl, #0
   ld  (PLY_SpecialTrack_PT + 1), hl

   ;Reset the SpecialTrack/Tracks line counter.
   ;We can't rely on the song data, because the Pattern Height is not related to the Tracks Height.
   ld  a, #1
   ld  (PLY_SpecialTrack_WaitCounter + 1), a
   ld  (PLY_Track1_WaitCounter + 1), a
   ld  (PLY_Track2_WaitCounter + 1), a
   ld  (PLY_Track3_WaitCounter + 1), a

PLY_Height:
   ld  a, #1

PLY_HeightEnd:
   ld  (PLY_HeightCpt + 1), a

;Read the Special Track/Tracks.
;------------------------------

;Read the Special Track.
PLY_SpecialTrack_WaitCounter:
   ld  a, #1
   dec a
   jr  nz, PLY_SpecialTrack_Wait

PLY_SpecialTrack_PT:
   ld  hl, #0
   ld   a, (hl) 
   inc hl
   srl  a                              ;Data (1) or Wait (0) ?
   jr  nc, PLY_SpecialTrack_NewWait    ;If Wait, A contains the Wait value. 

   ;Data. Effect Type ?
   srl  a                              ;Speed (0) or Digidrum (1) ?

   ;First, we don't test the Effect Type, but only the Escape Code (=0)
   jr  nz, PLY_SpecialTrack_NoEscapeCode
   ld   a, (hl)
   inc hl 

PLY_SpecialTrack_NoEscapeCode:
   ;Now, we test the Effect type, since the Carry didn't change.
   jr nc,PLY_SpecialTrack_Speed
   ld (_cpct_akp_digidrumStatus), a
   jr PLY_PT_SpecialTrack_EndData

PLY_SpecialTrack_Speed:
   ld (PLY_Speed + 1), a

PLY_PT_SpecialTrack_EndData:
   ld  a, #1

PLY_SpecialTrack_NewWait:
   ld (PLY_SpecialTrack_PT + 1), hl

PLY_SpecialTrack_Wait:
   ld (PLY_SpecialTrack_WaitCounter + 1), a


;Read the Track 1.
;-----------------

;Store the parameters, because the player below is called every frame, but the Read Track isn't.
PLY_Track1_WaitCounter:
   ld   a, #1
   dec  a
   jr  nz, PLY_Track1_NewInstrument_SetWait

PLY_Track1_PT:
   ld   hl, #0
   call PLY_ReadTrack
   ld   (PLY_Track1_PT + 1), hl
   jr   c, PLY_Track1_NewInstrument_SetWait

   ;No Wait command. Can be a Note and/or Effects.
   ;Make a copy of the flags+Volume in a, not to temper with the original.
   ld   a, d

   rra                           ;Volume ? If bit 4 was 1, then volume exists on b3-b0
   jr  nc, PLY_Track1_SameVolume
   and #0b1111
   ld  (PLY_Track1_Volume), a

PLY_Track1_SameVolume:
   rl  d                         ;New Pitch ?
   jr nc, PLY_Track1_NoNewPitch
   ld (PLY_Track1_PitchAdd + 1), ix

PLY_Track1_NoNewPitch:
   rl  d                         ;Note ? If no Note, we don't have to test if a new Instrument is here.
   jr nc, PLY_Track1_NoNoteGiven
   ld  a, e

PLY_Transposition1:
   add a, #0                     ;Transpose Note according to the Transposition in the Linker.
   ld  (PLY_Track1_Note), a

   ld  hl, #0                    ;Reset the TrackPitch.
   ld  (PLY_Track1_Pitch + 1), hl

   rl  d                         ;New Instrument ?
   jr  c, PLY_Track1_NewInstrument

PLY_Track1_SavePTInstrument:
   ld  hl, #0                                    ;Same Instrument. We recover its address to restart it.
   ld   a, (PLY_Track1_InstrumentSpeed + 1)      ;Reset the Instrument Speed Counter. Never seemed useful...
   ld  (PLY_Track1_InstrumentSpeedCpt + 1), a
   jr  PLY_Track1_InstrumentResetPT

PLY_Track1_NewInstrument:        ;New Instrument. We have to get its new address, and Speed.
   ld   l, b                     ;H is already set to 0 before.
   add hl, hl

PLY_Track1_InstrumentsTablePT:
   ld  bc, #0
   add hl, bc
   ld   a, (hl)                  ;Get Instrument address.
   inc hl
   ld   h, (hl)
   ld   l, a
   ld   a, (hl)                  ;Get Instrument speed.
   inc hl
   ld  (PLY_Track1_InstrumentSpeed + 1), a
   ld  (PLY_Track1_InstrumentSpeedCpt + 1), a
   ld   a, (hl)
   or   a                        ;Get IsRetrig?. Code it only if different to 0, else next Instruments are
   jr   z, .+5                   ; going to overwrite it.
   ld  (PLY_PSGReg13_Retrig + 1), a

   inc hl

   ld  (PLY_Track1_SavePTInstrument + 1), hl      ;When using the Instrument again, no need to give the Speed, it is skipped.

PLY_Track1_InstrumentResetPT:
   ld (PLY_Track1_Instrument + 1),hl


PLY_Track1_NoNoteGiven:
   ld  a, #1

PLY_Track1_NewInstrument_SetWait:
   ld  (PLY_Track1_WaitCounter + 1), a

;Read the Track 2.
;-----------------

;Store the parameters, because the player below is called every frame, but the Read Track isn't.
PLY_Track2_WaitCounter:
   ld    a, #1
   dec   a
   jr   nz, PLY_Track2_NewInstrument_SetWait

PLY_Track2_PT:
   ld   hl, #0
   call PLY_ReadTrack
   ld   (PLY_Track2_PT + 1), hl
   jr   c, PLY_Track2_NewInstrument_SetWait

   ;No Wait command. Can be a Note and/or Effects.
   ;Make a copy of the flags+Volume in a, not to temper with the original.
   ld   a, d

   rra                              ;Volume ? If bit 4 was 1, then volume exists on b3-b0
   jr  nc, PLY_Track2_SameVolume
   and #0b1111
   ld  (PLY_Track2_Volume), a

PLY_Track2_SameVolume:
   rl   d                           ;New Pitch ?
   jr  nc, PLY_Track2_NoNewPitch
   ld  (PLY_Track2_PitchAdd + 1), ix

PLY_Track2_NoNewPitch:
   rl   d                           ;Note ? If no Note, we don't have to test if a new Instrument is here.
   jr  nc,PLY_Track2_NoNoteGiven
   ld   a, e

PLY_Transposition2:
   add  a, #0                       ;Transpose Note according to the Transposition in the Linker.
   ld  (PLY_Track2_Note), a

   ld  hl, #0                       ;Reset the TrackPitch.
   ld  (PLY_Track2_Pitch + 1), hl

   rl   d                           ;New Instrument ?
   jr   c, PLY_Track2_NewInstrument

PLY_Track2_SavePTInstrument:
   ld  hl, #0                                ;Same Instrument. We recover its address to restart it.
   ld   a, (PLY_Track2_InstrumentSpeed + 1)  ;Reset the Instrument Speed Counter. Never seemed useful...
   ld  (PLY_Track2_InstrumentSpeedCpt + 1), a
   jr  PLY_Track2_InstrumentResetPT

PLY_Track2_NewInstrument:           ;New Instrument. We have to get its new address, and Speed.
   ld   l, b                        ;H is already set to 0 before.
   add hl, hl

PLY_Track2_InstrumentsTablePT:
   ld  bc, #0
   add hl, bc
   ld   a, (hl)         ;Get Instrument address.
   inc hl
   ld   h, (hl)
   ld   l, a
   ld   a, (hl)         ;Get Instrument speed.
   inc  hl
   ld  (PLY_Track2_InstrumentSpeed + 1), a
   ld  (PLY_Track2_InstrumentSpeedCpt + 1), a
   ld  a, (hl)
   or  a                ;Get IsRetrig?. Code it only if different to 0, else next Instruments are going to overwrite it.
   jr  z, .+5
   ld  (PLY_PSGReg13_Retrig + 1), a
   inc hl

   ld  (PLY_Track2_SavePTInstrument + 1), hl  ;When using the Instrument again, no need to give the Speed, it is skipped.

PLY_Track2_InstrumentResetPT:
   ld  (PLY_Track2_Instrument + 1), hl

PLY_Track2_NoNoteGiven:
   ld   a, #1

PLY_Track2_NewInstrument_SetWait:
   ld  (PLY_Track2_WaitCounter + 1), a

;Read the Track 3.
;-----------------

;Store the parameters, because the player below is called every frame, but the Read Track isn't.
PLY_Track3_WaitCounter:
   ld    a, #1
   dec   a
   jr   nz, PLY_Track3_NewInstrument_SetWait

PLY_Track3_PT:
   ld   hl, #0
   call PLY_ReadTrack
   ld   (PLY_Track3_PT + 1), hl
   jr    c, PLY_Track3_NewInstrument_SetWait

   ;No Wait command. Can be a Note and/or Effects.
   ;Make a copy of the flags+Volume in a, not to temper with the original.
   ld    a, d 

   rra                         ;Volume ? If bit 4 was 1, then volume exists on b3-b0
   jr   nc, PLY_Track3_SameVolume
   and  #0b1111
   ld   (PLY_Track3_Volume), a

PLY_Track3_SameVolume:
   rl   d                      ;New Pitch ?
   jr  nc,PLY_Track3_NoNewPitch
   ld  (PLY_Track3_PitchAdd + 1), ix

PLY_Track3_NoNewPitch:
   rl   d                      ;Note ? If no Note, we don't have to test if a new Instrument is here.
   jr  nc, PLY_Track3_NoNoteGiven
   ld   a, e

PLY_Transposition3:
   add  a, #0                  ;Transpose Note according to the Transposition in the Linker.
   ld  (PLY_Track3_Note), a

   ld  hl, #0                  ;Reset the TrackPitch.
   ld  (PLY_Track3_Pitch + 1), hl

   rl   d                      ;New Instrument ?
   jr   c, PLY_Track3_NewInstrument

PLY_Track3_SavePTInstrument: 
   ld  hl, #0                                   ;Same Instrument. We recover its address to restart it.
   ld   a, (PLY_Track3_InstrumentSpeed + 1)     ;Reset the Instrument Speed Counter. Never seemed useful...
   ld  (PLY_Track3_InstrumentSpeedCpt + 1), a
   jr  PLY_Track3_InstrumentResetPT

PLY_Track3_NewInstrument:                       ;New Instrument. We have to get its new address, and Speed.
   ld   l, b                                    ;H is already set to 0 before.
   add hl, hl

PLY_Track3_InstrumentsTablePT:
   ld  bc, #0
   add hl, bc
   ld   a, (hl)         ;Get Instrument address.
   inc hl
   ld   h, (hl)
   ld   l, a
   ld   a, (hl)         ;Get Instrument speed.
   inc hl
   ld  (PLY_Track3_InstrumentSpeed + 1), a
   ld  (PLY_Track3_InstrumentSpeedCpt + 1), a
   ld   a, (hl)
   or   a               ;Get IsRetrig?. Code it only if different to 0, else next Instruments are going to overwrite it.
   jr   z, .+5
   ld  (PLY_PSGReg13_Retrig + 1), a
   inc hl

   ld  (PLY_Track3_SavePTInstrument + 1), hl      ;When using the Instrument again, no need to give the Speed, it is skipped.

PLY_Track3_InstrumentResetPT:
   ld  (PLY_Track3_Instrument + 1), hl

PLY_Track3_NoNoteGiven:
   ld  a, #1

PLY_Track3_NewInstrument_SetWait:
   ld  (PLY_Track3_WaitCounter + 1), a

PLY_Speed:
   ld  a, #1

PLY_SpeedEnd:
   ld  (PLY_SpeedCpt + 1), a


;Play the Sound on Track 3
;-------------------------
;Plays the sound on each frame, but only save the forwarded Instrument pointer when Instrument Speed is reached.
;This is needed because TrackPitch is involved in the Software Frequency/Hardware Frequency calculation, 
;and is calculated every frame.
   ld  iy, #PLY_PSGRegistersArray + 4

PLY_Track3_Pitch: 
   ld  hl, #0

PLY_Track3_PitchAdd:
   ld  de, #0
   add hl, de
   ld (PLY_Track3_Pitch + 1), hl
   sra  h                        ;Shift the Pitch to slow its speed.
   rr   l
   sra  h
   rr   l
   ex  de, hl
   exx

.equ PLY_Track3_Volume, .+2
.equ PLY_Track3_Note,   .+1

   ld  de, #0                    ;D=Inverted Volume E=Note

PLY_Track3_Instrument:
   ld  hl, #0
   call PLY_PlaySound

PLY_Track3_InstrumentSpeedCpt:
   ld   a, #1
   dec  a
   jr  nz, PLY_Track3_PlayNoForward
   ld  (PLY_Track3_Instrument + 1), hl

PLY_Track3_InstrumentSpeed: 
   ld   a, #6

PLY_Track3_PlayNoForward:
   ld  (PLY_Track3_InstrumentSpeedCpt + 1), a

;***************************************
;Play Sound Effects on Track 3 (If activated)
;***************************************
.if PLY_UseSoundEffects

   PLY_SFX_Track3_Pitch: 
      ld  de, #0
      exx

   .equ PLY_SFX_Track3_Volume, .+2
   .equ PLY_SFX_Track3_Note,   .+1

      ld  de, #0                       ;D=Inverted Volume E=Note

   PLY_SFX_Track3_Instrument:
      ld  hl, #0                       ;If 0, no sound effect.
      ld   a, l
      or   h
      jr   z, PLY_SFX_Track3_End
      ld   a, #1
      ld  (PLY_PS_EndSound_SFX + 1), a
      call PLY_PlaySound
      xor  a
      ld  (PLY_PS_EndSound_SFX + 1), a
      ld   a, l                        ;If the new address is 0, the instrument is over. 
      or   h                           ;Speed is set in the process, we don't care.
      jr   z, PLY_SFX_Track3_Instrument_SetAddress

   PLY_SFX_Track3_InstrumentSpeedCpt:
      ld   a, #1
      dec  a
      jr  nz, PLY_SFX_Track3_PlayNoForward

   PLY_SFX_Track3_Instrument_SetAddress:
      ld  (PLY_SFX_Track3_Instrument + 1), hl

   PLY_SFX_Track3_InstrumentSpeed:
      ld   a, #6

   PLY_SFX_Track3_PlayNoForward:
      ld  (PLY_SFX_Track3_InstrumentSpeedCpt + 1), a

   PLY_SFX_Track3_End:

.endif
;******************************************


   .dw #0x7DDD  ; ld  a, ixl          ;Save the Register 7 of the Track 3.
   ex  af, af'

;Play the Sound on Track 2
;-------------------------
   ld  iy, #PLY_PSGRegistersArray + 2

PLY_Track2_Pitch:
   ld  hl, #0

PLY_Track2_PitchAdd:
   ld  de, #0
   add hl, de
   ld  (PLY_Track2_Pitch + 1), hl
   sra  h                             ;Shift the Pitch to slow its speed.
   rr   l
   sra  h
   rr   l
   ex  de, hl
   exx

.equ PLY_Track2_Volume, .+2
.equ PLY_Track2_Note,   .+1

   ld   de, #0                        ;D=Inverted Volume E=Note

PLY_Track2_Instrument:
   ld   hl, #0
   call PLY_PlaySound

PLY_Track2_InstrumentSpeedCpt:
   ld   a, #1
   dec  a
   jr  nz, PLY_Track2_PlayNoForward
   ld  (PLY_Track2_Instrument + 1), hl

PLY_Track2_InstrumentSpeed:
   ld   a, #6

PLY_Track2_PlayNoForward:
   ld  (PLY_Track2_InstrumentSpeedCpt + 1), a

;***************************************
;Play Sound Effects on Track 2 (If activated)
;***************************************
.if PLY_UseSoundEffects

   PLY_SFX_Track2_Pitch:
      ld  de, #0
      exx

   .equ PLY_SFX_Track2_Volume, .+2
   .equ PLY_SFX_Track2_Note,   .+1

      ld  de, #0                       ;D=Inverted Volume E=Note
   PLY_SFX_Track2_Instrument:
      ld  hl, #0                       ;If 0, no sound effect.
      ld   a, l
      or   h
      jr   z, PLY_SFX_Track2_End
      ld   a, #1
      ld  (PLY_PS_EndSound_SFX + 1), a
      call PLY_PlaySound
      xor  a
      ld  (PLY_PS_EndSound_SFX + 1), a
      ld   a, l                        ;If the new address is 0, the instrument is over. 
      or   h                           ;Speed is set in the process, we don't care.
      jr   z, PLY_SFX_Track2_Instrument_SetAddress

   PLY_SFX_Track2_InstrumentSpeedCpt:
      ld   a, #1
      dec  a
      jr  nz, PLY_SFX_Track2_PlayNoForward

   PLY_SFX_Track2_Instrument_SetAddress:
      ld  (PLY_SFX_Track2_Instrument + 1), hl

   PLY_SFX_Track2_InstrumentSpeed:
      ld   a, #6

   PLY_SFX_Track2_PlayNoForward:
      ld (PLY_SFX_Track2_InstrumentSpeedCpt + 1), a

   PLY_SFX_Track2_End:

.endif
;******************************************

   ex  af, af'
   add  a, a                          ;Mix Reg7 from Track2 with Track3, making room first.
   .dw #0xB5DD  ; or ixl
   rla
   ex  af, af'


;Play the Sound on Track 1
;-------------------------

   ld  iy, #PLY_PSGRegistersArray

PLY_Track1_Pitch:
   ld  hl, #0

PLY_Track1_PitchAdd:
   ld  de, #0
   add hl, de
   ld  (PLY_Track1_Pitch + 1), hl
   sra  h                             ;Shift the Pitch to slow its speed.
   rr   l
   sra  h
   rr   l
   ex  de, hl
   exx

.equ PLY_Track1_Volume, .+2
.equ PLY_Track1_Note,   .+1

   ld   de, #0                        ;D=Inverted Volume E=Note

PLY_Track1_Instrument:
   ld   hl, #0
   call PLY_PlaySound

PLY_Track1_InstrumentSpeedCpt:
   ld   a, #1
   dec  a
   jr  nz, PLY_Track1_PlayNoForward
   ld  (PLY_Track1_Instrument + 1), hl

PLY_Track1_InstrumentSpeed:
   ld   a, #6

PLY_Track1_PlayNoForward:
   ld (PLY_Track1_InstrumentSpeedCpt + 1), a

;***************************************
;Play Sound Effects on Track 1 (If activated)
;***************************************
.if PLY_UseSoundEffects

   PLY_SFX_Track1_Pitch:
      ld  de, #0
      exx
   .equ PLY_SFX_Track1_Volume, .+2
   .equ PLY_SFX_Track1_Note,   .+1

      ld  de, #0                          ;D=Inverted Volume E=Note

   PLY_SFX_Track1_Instrument:
      ld  hl, #0                          ;If 0, no sound effect.
      ld   a, l
      or   h
      jr   z, PLY_SFX_Track1_End
      ld   a, #1
      ld  (PLY_PS_EndSound_SFX + 1), a
      call PLY_PlaySound
      xor  a
      ld  (PLY_PS_EndSound_SFX + 1), a
      ld   a, l                           ;If the new address is 0, the instrument is over. 
      or   h                              ;Speed is set in the process, we don't care.
      jr   z, PLY_SFX_Track1_Instrument_SetAddress

   PLY_SFX_Track1_InstrumentSpeedCpt:
      ld   a, #1
      dec  a
      jr  nz, PLY_SFX_Track1_PlayNoForward

   PLY_SFX_Track1_Instrument_SetAddress:
      ld  (PLY_SFX_Track1_Instrument + 1), hl

   PLY_SFX_Track1_InstrumentSpeed:
      ld   a, #6

   PLY_SFX_Track1_PlayNoForward:
      ld  (PLY_SFX_Track1_InstrumentSpeedCpt + 1), a

   PLY_SFX_Track1_End:

.endif
;***********************************

   ex  af, af'
  .dw #0xB5DD  ; or ixl                   ;Mix Reg7 from Track3 with Track2+1.

;Send the registers to PSG. Various codes according to the machine used.
PLY_SendRegisters:
   ;A=Register 7

   ld  de, #0xC080
   ld   b, #0xF6
   out (c),d   ;#f6c0
   exx
   ld  hl, #PLY_PSGRegistersArray
   ld   e, #0xF6
   ld  bc, #0xF401

;Register 0
   .dw #0x71ED  ;out(c), 0    ; #0xF400+Register
   ld   b, e
   .dw #0x71ED  ;out(c), 0    ; #0xF600
   dec  b
   outi                       ; #0xF400+value
   exx
   out (c), e                 ; #0xF680
   out (c), d                 ; #0xF6C0
   exx

;Register 1
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 2
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 3
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 4
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 5
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 6
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 7
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   dec  b
   out (c), a         ;Read A register instead of the list.
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 8
   out (c), c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec b

   .if PLY_UseFades
         dec  b
         ld   a, (hl)

      PLY_Channel1_FadeValue:
         sub  #0             ;Set a value from 0 (full volume) to 16 or more (volume to 0).
         jr  nc, .+6
         .dw #0x71ED  ;out(c), 0
         jr  .+4
         out (c), a
         inc hl

   .else

         outi

   .endif

   exx
   out (c), e
   out (c), d
   exx
   inc  c
   inc hl            ;Skip unused byte.

;Register 9
   out (c), c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec b

   .if PLY_UseFades         ;If PLY_UseFades is set to 1, we manage the volume fade.
         dec  b
         ld   a, (hl)

      PLY_Channel2_FadeValue:
         sub  #0             ;Set a value from 0 (full volume) to 16 or more (volume to 0).
         jr  nc, .+6
         .dw #0x71ED  ;out(c), 0
         jr  .+4
         out (c), a
         inc hl

   .else

         outi

   .endif

   exx
   out (c), e
   out (c), d
   exx
   inc  c
   inc hl            ;Skip unused byte.

;Register 10
   out (c), c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b

   .if PLY_UseFades
         dec  b
         ld   a, (hl)

         PLY_Channel3_FadeValue:
         sub  #0             ;Set a value from 0 (full volume) to 16 or more (volume to 0).
         jr  nc, .+6
         .dw #0x71ED  ;out(c), 0
         jr  .+4
         out (c), a
         inc hl

   .else

         outi

   .endif

   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 11
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 12
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d
   exx
   inc  c

;Register 13
   .if PLY_SystemFriendly

         call PLY_PSGReg13_Code

   .endif
      PLY_PSGREG13_RecoverSystemRegisters:
         pop iy
         pop ix

   .if PLY_SystemFriendly
         pop bc
         pop af
         exx
         ex  af, af'

         ;Restore Interrupt status
      PLY_RestoreInterruption:
         nop            ;Will be automodified to an DI/EI.
         ret

   .endif


PLY_PSGReg13_Code:
   ld  a, (hl)

PLY_PSGReg13_Retrig:
   cp  #255            ;If IsRetrig?, force the R13 to be triggered.
   ret z

   ld  (PLY_PSGReg13_Retrig + 1), a
   out (c),c
   ld   b, e
   .dw #0x71ED  ;out(c), 0
   dec  b
   outi
   exx
   out (c), e
   out (c), d

   ret

;There are two holes in the list, because the Volume registers are set relatively to 
;the Frequency of the same Channel (+7, always). Also, the Reg7 is passed as a register, so is not kept in the memory.
PLY_PSGRegistersArray:
PLY_PSGReg0:  .db 0
PLY_PSGReg1:  .db 0
PLY_PSGReg2:  .db 0
PLY_PSGReg3:  .db 0
PLY_PSGReg4:  .db 0
PLY_PSGReg5:  .db 0
PLY_PSGReg6:  .db 0
PLY_PSGReg8:  .db 0      ;+7
              .db 0
PLY_PSGReg9:  .db 0      ;+9
              .db 0
PLY_PSGReg10: .db 0      ;+11
PLY_PSGReg11: .db 0
PLY_PSGReg12: .db 0
PLY_PSGReg13: .db 0
PLY_PSGRegistersArray_End:


;Plays a sound stream.
;hl=Pointer on Instrument Data
;IY=Pointer on Register code (volume, frequency).
;E=Note
;D=Inverted Volume
;DE'=TrackPitch

;RET=
;hl=New Instrument pointer.
;IXL=Reg7 mask (x00x)

;Also used inside =
;B,C=read byte/second byte.
;IXH=Save original Note (only used for Independant mode).

PLY_PlaySound:
   ld   b, (hl)
   inc hl
   rr   b
   jp   c, PLY_PS_Hard

;**************
;Software Sound
;**************
   ;Second Byte needed ?
   rr   b
   jr   c, PLY_PS_S_SecondByteNeeded

   ;No second byte needed. We need to check if Volume is null or not.
   ld   a, b
   and  #0b1111
   jr  nz, PLY_PS_S_SoundOn

   ;Null Volume. It means no Sound. We stop the Sound, the Noise, and it's over.
   ;We have to make the volume to 0, because if a bass Hard was activated before, we have to stop it.
   ld  7(iy), a
   .db #0xDD, #0x2E, #0b1001 ; ld ixl,%1001

   ret

PLY_PS_S_SoundOn:
   ;Volume is here, no Second Byte needed. It means we have a simple Software sound (Sound = On, Noise = Off)
   ;We have to test Arpeggio and Pitch, however.
   .db #0xDD, #0x2E, #0b1000 ; ld ixl,%1000

   sub  d                  ;Code Volume.
   jr  nc, .+3
   xor  a
   ld  7(iy), a

   rr   b                  ;Needed for the subroutine to get the good flags.
   call PLY_PS_CalculateFrequency
   ld  0(iy), l            ;Code Frequency.
   ld  1(iy), h
   exx

   ret

PLY_PS_S_SecondByteNeeded:
   .db #0xDD, #0x2E, #0b1000 ; ld ixl,%1000  ;By defaut, No Noise, Sound.

   ;Second Byte needed.
   ld   c, (hl)
   inc hl

   ;Noise ?
   ld   a, c
   and #0b11111
   jr   z, PLY_PS_S_SBN_NoNoise
   ld  (PLY_PSGReg6), a
   .db #0xDD, #0x2E, #0b0000 ; ld ixl,%0000  ;Open Noise Channel.

PLY_PS_S_SBN_NoNoise:
   ;Here we have either Volume and/or Sound. So first we need to read the Volume.
   ld   a, b
   and #0b1111
   sub  d                    ;Code Volume.
   jr  nc, .+3
   xor  a
   ld  7(iy), a

   ;Sound ?
   bit  5, c
   jr  nz, PLY_PS_S_SBN_Sound

   ;No Sound. Stop here.
   .dw #0x2CDD ; inc ixl     ;Set Sound bit to stop the Sound.
   ret

PLY_PS_S_SBN_Sound:
   ;Manual Frequency ?
   rr   b                    ;Needed for the subroutine to get the good flags.
   bit  6, c
   call PLY_PS_CalculateFrequency_TestManualFrequency
   ld  0(iy), l              ;Code Frequency.
   ld  1(iy), h
   exx

   ret

;**********
;Hard Sound
;**********
PLY_PS_Hard:
   ;We don't set the Volume to 16 now because we may have reached the end of the sound !
   rr   b                                        ;Test Retrig here, it is common to every Hard sounds.
   jr  nc, PLY_PS_Hard_NoRetrig
   ld   a, (PLY_Track1_InstrumentSpeedCpt + 1)   ;Retrig only if it is the first step in this line of Instrument !
   ld   c, a
   ld   a, (PLY_Track1_InstrumentSpeed + 1)
   cp   c
   jr  nz, PLY_PS_Hard_NoRetrig
   ld   a, #PLY_RetrigValue
   ld  (PLY_PSGReg13_Retrig + 1), a

PLY_PS_Hard_NoRetrig:
   ;Independant/Loop or Software/Hardware Dependent ?
   bit  1, b                                    ;We don't shift the bits, so that we can use the same code 
   jp  nz, PLY_PS_Hard_LoopOrIndependent        ;(Frequency calculation) several times.

   ;Hardware Sound.
   ld  7(iy), #16                               ;Set Volume
   .db #0xDD, #0x2E, #0b1000 ; ld ixl,%1000     ;Sound is always On here (only Independence mode can switch it off).

   ;This code is common to both Software and Hardware Dependent.
   ld   c, (hl)                                 ;Get Second Byte.
   inc hl
   ld   a, c                                    ;Get the Hardware Envelope waveform.
   and  #0b1111                                 ;We don't care about the bit 7-4, but we have to clear them, 
   ld  (PLY_PSGReg13), a                        ;else the waveform might be reset.

   bit  0, b
   jr   z, PLY_PS_HardwareDependent

;******************
;Software Dependent
;******************

   ;Calculate the Software frequency
   bit  4-2, b                                  ;Manual Frequency ? -2 Because the byte has been shifted previously.
   call PLY_PS_CalculateFrequency_TestManualFrequency
   ld  0(iy), l                                 ;Code Software Frequency.
   ld  1(iy), h
   exx

   ;Shift the Frequency.
   ld   a, c
   rra
   rra                                          ;Shift=Shift*4. The shift is inverted in memory (7 - Editor Shift).
   and #0b11100
   ld  (PLY_PS_SD_Shift + 1), a
   ld   a, b                                    ;Used to get the HardwarePitch flag within the second registers set.
   exx

PLY_PS_SD_Shift:
   jr  .+2
   srl  h
   rr   l
   srl  h
   rr   l
   srl  h
   rr   l
   srl  h
   rr   l
   srl  h
   rr   l
   srl  h
   rr   l
   srl  h
   rr   l
   jr  nc, .+3
   inc hl

   ;Hardware Pitch ?
   bit 7-2, a
   jr   z, PLY_PS_SD_NoHardwarePitch
   exx                                          ;Get Pitch and add it to the just calculated Hardware Frequency.
   ld   a, (hl)
   inc hl
   exx
   add  a, l                                    ;Slow. Can be optimised ? Probably never used anyway.....
   ld   l, a
   exx
   ld   a, (hl)
   inc hl
   exx
   adc  a, h
   ld   h, a

PLY_PS_SD_NoHardwarePitch:
   ld  (PLY_PSGReg11), hl
   exx

;This code is also used by Hardware Dependent.
PLY_PS_SD_Noise:
   ;Noise ?
   bit  7, c
   ret  z

   ld   a, (hl)
   inc hl
   ld  (PLY_PSGReg6), a
   .db #0xDD, #0x2E, #0b0000 ; ld ixl,%0000  
   ret

;******************
;Hardware Dependent
;******************
PLY_PS_HardwareDependent:
   ;Calculate the Hardware frequency
   bit 4-2, b                                   ;Manual Hardware Frequency ? -2 Because the byte has been shifted previously.
   call PLY_PS_CalculateFrequency_TestManualFrequency
   ld  (PLY_PSGReg11),hl                        ;Code Hardware Frequency.
   exx

   ;Shift the Hardware Frequency.
   ld   a, c
   rra
   rra                                          ;Shift=Shift*4. The shift is inverted in memory (7 - Editor Shift).
   and #0b11100
   ld  (PLY_PS_HD_Shift + 1), a
   ld   a, b                                    ;Used to get the Software flag within the second registers set.
   exx

PLY_PS_HD_Shift:
   jr  .+2
   sla  l
   rl   h
   sla  l
   rl   h
   sla  l
   rl   h
   sla  l
   rl   h
   sla  l
   rl   h
   sla  l
   rl   h
   sla  l
   rl   h

   ;Software Pitch ?
   bit 7-2, a
   jr  z, PLY_PS_HD_NoSoftwarePitch
   exx                                          ;Get Pitch and add it to the just calculated Software Frequency.
   ld   a, (hl)
   inc hl
   exx
   add  a, l
   ld   l, a                                    ;Slow. Can be optimised ? Probably never used anyway.....
   exx
   ld   a, (hl)
   inc hl
   exx
   adc  a, h
   ld   h, a

PLY_PS_HD_NoSoftwarePitch:
   ld  0(iy), l                                  ;Code Frequency.
   ld  1(iy), h
   exx

   ;Go to manage Noise, common to Software Dependent.
   jr  PLY_PS_SD_Noise


PLY_PS_Hard_LoopOrIndependent:
   bit  0, b                                    ;We mustn't shift it to get the result in the Carry, as it would be mess 
   jr   z, PLY_PS_Independent                   ;the structure of the flags, making it uncompatible with the common code.

   ;The sound has ended.
   ;If Sound Effects activated, we mark the "end of sound" by returning a 0 as an address.
.if PLY_UseSoundEffects

   PLY_PS_EndSound_SFX: 
      ld  a, #0                                 ; Is the sound played is a SFX (1) or a normal sound (0) ?
      or  a
      jr  z, PLY_PS_EndSound_NotASFX
      ld hl, #0
      ret

   PLY_PS_EndSound_NotASFX:

.endif

   ;The sound has ended. Read the new pointer and restart instrument.
   ld   a, (hl)
   inc hl
   ld   h, (hl)
   ld   l, a
   jp  PLY_PlaySound

;***********
;Independent
;***********
PLY_PS_Independent:
   ld  7(iy), #16                             ;Set Volume

   ;Sound ?
   bit 7-2, b                                  ;-2 Because the byte has been shifted previously.
   jr  nz, PLY_PS_I_SoundOn

   ;No Sound ! It means we don't care about the software frequency (manual frequency, arpeggio, pitch).
   .db #0xDD, #0x2E, #0b1001 ; ld ixl,%1001
   jr  PLY_PS_I_SkipSoftwareFrequencyCalculation

PLY_PS_I_SoundOn:
   .db #0xDD, #0x2E, #0b1000 ; ld ixl, %1000   ;Sound is on.
   .dw #0x63DD               ; ld ixh, e       ;Save the original note for the Hardware frequency, 
                                               ;because a Software Arpeggio will modify it. 
   ;Calculate the Software frequency
   bit 4-2, b                                  ;Manual Frequency ? -2 Because the byte has been shifted previously.
   call PLY_PS_CalculateFrequency_TestManualFrequency
   ld  0(iy), l                                ;Code Software Frequency.
   ld  1(iy), h
   exx

   .dw #0x5CDD               ; ld  e, ixh

PLY_PS_I_SkipSoftwareFrequencyCalculation:
   ld   b, (hl)                                ;Get Second Byte.
   inc hl
   ld   a, b                                   ;Get the Hardware Envelope waveform.
   and #0b1111                                 ;We don't care about the bit 7-4, but we have to clear them, 
   ld  (PLY_PSGReg13), a                       ;else the waveform might be reset.

   ;Calculate the Hardware frequency
   rr   b                                      ;Must shift it to match the expected data of the subroutine.
   rr   b
   bit 4-2, b                                  ;Manual Hardware Frequency ? -2 Because the byte has been shifted previously.
   call PLY_PS_CalculateFrequency_TestManualFrequency
   ld  (PLY_PSGReg11), hl                      ;Code Hardware Frequency.
   exx

   ;Noise ? We can't use the previous common code, because the setting of the Noise is different, 
   ;since Independent can have no Sound.
   bit 7-2, b
   ret z

   ld   a, (hl)
   inc hl
   ld  (PLY_PSGReg6), a
   .dw #0x7DDD  ; ld a, ixl                    ;Set the Noise bit.
   res  3, a
   .dw #0x6FDD  ; ld ixl, a
   ret

;Subroutine that =
;If Manual Frequency? (Flag Z off), read frequency (Word) and adds the TrackPitch (DE').
;Else, Auto Frequency.
;   if Arpeggio? = 1 (bit 3 from B), read it (Byte).
;   if Pitch? = 1 (bit 4 from B), read it (Word).
;   Calculate the frequency according to the Note (E) + Arpeggio + TrackPitch (DE').

;hl = Pointer on Instrument data.
;DE'= TrackPitch.

;RET=
;hl = Pointer on Instrument moved forward.
;hl'= Frequency
;   RETURN IN AUXILIARY REGISTERS
PLY_PS_CalculateFrequency_TestManualFrequency:

   jr   z, PLY_PS_CalculateFrequency

   ;Manual Frequency. We read it, no need to read Pitch and Arpeggio.
   ;However, we add TrackPitch to the read Frequency, and that's all.
   ld   a, (hl)
   inc hl
   exx
   add  a, e                  ;Add TrackPitch LSB.
   ld   l, a
   exx
   ld   a, (hl)
   inc  hl
   exx
   adc  a, d                  ;Add TrackPitch HSB.
   ld   h, a
   ret

PLY_PS_CalculateFrequency:
   ;Pitch ?
   bit 5-1, b
   jr   z, PLY_PS_S_SoundOn_NoPitch
   ld   a, (hl)
   inc hl
   exx
   add  a, e                  ;If Pitch found, add it directly to the TrackPitch.
   ld   e, a
   exx
   ld   a, (hl)
   inc hl
   exx
   adc  a, d
   ld   d, a
   exx

PLY_PS_S_SoundOn_NoPitch:
   ;Arpeggio ?
   ld   a, e
   bit 4-1,b
   jr   z, PLY_PS_S_SoundOn_ArpeggioEnd
   add  a, (hl)               ;Add Arpeggio to Note.
   inc hl
   cp #144
   jr   c, .+4
   ld   a, #143

PLY_PS_S_SoundOn_ArpeggioEnd:

   ;Frequency calculation.
   exx
   ld   l, a
   ld   h, #0
   add hl, hl

   ld  bc, #PLY_FrequencyTable
   add hl, bc

   ld   a, (hl)
   inc hl
   ld   h, (hl)
   ld   l, a
   add hl, de               ;Add TrackPitch + InstrumentPitch (if any).

   ret

;Read one Track.
;hl=Track Pointer.

;Ret =
;hl=New Track Pointer.
;Carry = 1 = Wait A lines. Carry=0=Line not empty.
;A=Wait (0(=256)-127), if Carry.
;D=Parameters + Volume.
;E=Note
;B=Instrument. 0=RST
;IX=PitchAdd. Only used if Pitch? = 1.
PLY_ReadTrack:
   ld   a, (hl)
   inc hl
   srl  a                              ;Full Optimisation ? If yes = Note only, no Pitch, no Volume, Same Instrument.
   jr   c, PLY_ReadTrack_FullOptimisation
   sub #32                             ;0-31 = Wait.
   jr   c, PLY_ReadTrack_Wait
   jr   z, PLY_ReadTrack_NoOptimisation_EscapeCode
   dec  a                              ;0 (32-32) = Escape Code for more Notes (parameters will be read)

   ;Note. Parameters are present. But the note is only present if Note? flag is 1.
   ld   e, a                           ;Save Note.

   ;Read Parameters
PLY_ReadTrack_ReadParameters:
   ld   a, (hl)
   ld   d, a                           ;Save Parameters.
   inc hl

   rla                                 ;Pitch ?
   jr  nc, PLY_ReadTrack_Pitch_End
   ld   b, (hl)                        ;Get PitchAdd
   .dw #0x68DD  ; ld ixl, b
   inc hl
   ld   b, (hl)
   .dw #0x60DD  ; ld ixh, b
   inc hl

PLY_ReadTrack_Pitch_End:
   rla                                 ;Skip IsNote? flag.
   rla                                 ;New Instrument ?
   ret nc
   ld   b, (hl)
   inc hl
   or   a                              ;Remove Carry, as the player interpret it as a Wait command.
   ret

;Escape code, read the Note and returns to read the Parameters.
PLY_ReadTrack_NoOptimisation_EscapeCode:
   ld   e, (hl)
   inc hl
   jr  PLY_ReadTrack_ReadParameters


PLY_ReadTrack_FullOptimisation:
   ;Note only, no Pitch, no Volume, Same Instrument.
   ld   d, #0b01000000                ;Note only.
   sub #1
   ld   e, a
   ret nc
   ld   e, (hl)                       ;Escape Code found (0). Read Note.
   inc hl
   or   a
   ret

PLY_ReadTrack_Wait:
   add  a, #32
   ret

PLY_FrequencyTable:
.dw 3822,3608,3405,3214,3034,2863,2703,2551,2408,2273,2145,2025
.dw 1911,1804,1703,1607,1517,1432,1351,1276,1204,1136,1073,1012
.dw 956,902,851,804,758,716,676,638,602,568,536,506
.dw 478,451,426,402,379,358,338,319,301,284,268,253
.dw 239,225,213,201,190,179,169,159,150,142,134,127
.dw 119,113,106,100,95,89,84,80,75,71,67,63
.dw 60,56,53,50,47,45,42,40,38,36,34,32
.dw 30,28,27,25,24,22,21,20,19,18,17,16
.dw 15,14,13,13,12,11,11,10,9,9,8,8
.dw 7,7,7,6,6,6,5,5,5,4,4,4
.dw 4,4,3,3,3,3,3,2,2,2,2,2
.dw 2,2,2,2,1,1,1,1,1,1,1,1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_akp_musicInit
;;
;;    Sets up a music into Arkos Tracker Player to be played later on with
;; <cpct_akp_musicPlay>.
;;
;; C Definition:
;;    void <cpct_akp_musicInit> (void* *songdata*)
;;
;; Input Parameters (2 bytes):
;;    (2B DE) songdata - Pointer to the start of the array containing song's data in AKS binary format
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_akp_musicInit_asm
;;
;; Parameter Restrictions:
;;    * *songdata* must be an array containing dong's data in AKS binary format.
;; Take into account that AKS binary format enforces a concrete start location in
;; memory. Therefore, song must have been created in Arkos Tracker and exported 
;; to the same memory location that *songdata* points to. If you fail to 
;; locate the song at the same memory location it was exported for in Arkos 
;; Tracker, unexpected results will happen (Typically, noise will be played but,
;; occasionally your program may hang or crash).
;;
;; Known limitations:
;;    * *songdata* must be the same memory address that the one given to Arkos
;; Tracker when exporting song's binary. Arkos Tracker songs are created to
;; be at a concrete memory location, due to optimization constraints. Therefore,
;; this must be taken into account. If you wanted to change the memory location
;; of the song, you should first open the song into Arkos Tracker and export
;; it again with the new desired memory location.
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function should be called fist to initialize the song that is to be 
;; played. The function reads the song header and sets up the player to start 
;; playing it. Once this process is done, <cpct_akp_musicPlay> should be called
;; at the required frequency to continuously play the song.
;;
;; Destroyed Register values: 
;;    AF, AF', BC, DE, HL, IX, IY
;;
;; Required memory:
;;    92 bytes 
;;
;;    However, take into account that all of Arkos Tracker Player's
;; functions are linked and included, because they depend on each other. Total
;; memory requirement is around 2089 bytes.
;;
;; Time Measures:
;; (start code)
;;    To be done
;; (end code)
;;
;; Credits:
;;    This is a modification of the original <Arkos Tracker Player at
;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
;; contributed to this source.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_akp_musicInit::
   ld   hl, #2    ;; [10] Retrieve parameters from stack
   add  hl, sp    ;; [11]
   ld    e, (hl)  ;; [ 7] DE = Pointer to the start of music
   inc  hl        ;; [ 6]
   ld    d, (hl)  ;; [ 7]

cpct_akp_musicInit_asm::   ;; Entry point for assembly calls using registers for parameter passing
   ;; First, set song loop times to 0 when we start
   xor   a                          ;; A = 0
   ld (_cpct_akp_songLoopTimes), a  ;; _cpct_akp_songLoopTimes = 0

PLY_Init:
   ld  hl, #9                          ;Skip Header, SampleChannel, YM Clock (DB*3), and Replay Frequency.
   add hl, de

   ld  de, #PLY_Speed + 1
   ldi                                 ;Copy Speed.
   ld   c, (hl)                        ;Get Instruments chunk size.
   inc hl
   ld   b, (hl)
   inc hl
   ld  (PLY_Track1_InstrumentsTablePT + 1), hl
   ld  (PLY_Track2_InstrumentsTablePT + 1), hl
   ld  (PLY_Track3_InstrumentsTablePT + 1), hl

   add hl, bc                          ;Skip Instruments to go to the Linker address.

   ;Get the pre-Linker information of the first pattern.
   ld  de, #PLY_Height + 1
   ldi
   ld  de, #PLY_Transposition1 + 1
   ldi
   ld  de, #PLY_Transposition2 + 1
   ldi
   ld  de, #PLY_Transposition3 + 1
   ldi
   ld  de, #PLY_SaveSpecialTrack + 1
   ldi
   ldi
   ld  (PLY_Linker_PT + 1), hl        ;Get the Linker address.

   ld  a, #1
   ld  (PLY_SpeedCpt + 1), a
   ld  (PLY_HeightCpt + 1), a

   ld  a, #0xFF
   ld  (PLY_PSGReg13), a

   ;Set the Instruments pointers to Instrument 0 data (Header has to be skipped).
   ld  hl, (PLY_Track1_InstrumentsTablePT + 1)
   ld   e, (hl)
   inc hl
   ld   d, (hl)
   ex  de, hl
   inc hl                             ;Skip Instrument 0 Header.
   inc hl
   ld  (PLY_Track1_Instrument + 1), hl
   ld  (PLY_Track2_Instrument + 1), hl
   ld  (PLY_Track3_Instrument + 1), hl
   ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_akp_stop
;;
;;    Stops playing musing and sound effects on all 3 channels.
;;
;; C Definition:
;;    void <cpct_akp_stop> ()
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_akp_stop_asm
;;
;; Known limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function stops the music and sound effects playing in the 3 channels. 
;; It can be later continued again calling <cpct_akp_musicPlay>. Please, take
;; into account that sound effects cannot be played while music is stopped, as
;; code for sound effects and music play is integrated.
;;
;; Destroyed Register values: 
;;    AF, AF', BC, DE, HL, IX, IY
;;
;; Required memory:
;;    19 bytes 
;;
;;    However, take into account that all of Arkos Tracker Player's
;; functions are linked and included, because they depend on each other. Total
;; memory requirement is around 2089 bytes.
;;
;; Time Measures:
;; (start code)
;;    To be done
;; (end code)
;;
;; Credits:
;;    This is a modification of the original <Arkos Tracker Player at
;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
;; contributed to this source.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Stop the music, cut the channels.
_cpct_akp_stop::
cpct_akp_stop_asm::  ;; Entry point for assembly calls 
PLY_Stop:

   .if PLY_SystemFriendly
      call PLY_DisableInterruptions
      ex  af, af'
      exx
      push af
      push bc
   .endif
   push ix
   push iy

   ld  hl, #PLY_PSGReg8
   ld  bc, #0x0500
   ld  (hl), c
   inc hl
   djnz .-2
   ld   a, #0b00111111
   jp  PLY_SendRegisters

.if PLY_UseSoundEffects
   
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;;
   ;; Function: cpct_akp_SFXGetInstrument
   ;;
   ;;    Returns the number of the instrument which is playing SFX in the selected
   ;; channel (0 = Channel free / not playing SFX).
   ;;
   ;; C Definition:
   ;;    <u16> <cpct_akp_SFXGetInstrument> (<u8> *channel_bitmask*)
   ;;
   ;; Assembly call (Input parameters on registers):
   ;;    > call cpct_akp_SFXGetInstrument_asm
   ;;
   ;; Details:
   ;;    This function returns the number of the instrumeent which is playing in
   ;; the given channel. You may use it to check the status of an FX reproduction. 
   ;; If this function returns a 0 it means that no SFX is playing on the channel 
   ;; (i.e. the channel is free for reproducing new SFXs).
   ;;
   ;; Destroyed Register values: 
   ;;    AF, HL
   ;;
   ;; Required memory:
   ;;    25 bytes 
   ;;
   ;;    However, take into account that all of Arkos Tracker Player's
   ;; functions are linked and included, because they depend on each other. Total
   ;; memory requirement is around 2089 bytes.
   ;;
   ;; Time Measures:
   ;; (start code)
   ;;    Case    | Cycles | microSecs (us)
   ;; --------------------------------------
   ;;   Best     |   68   |  17.00
   ;; --------------------------------------
   ;;   Worst    |   82   |  20.50
   ;; --------------------------------------
   ;; Asm saving |  -28   |  -7.00
   ;; --------------------------------------
   ;; (end code)
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   _cpct_akp_SFXGetInstrument::
      ;; Get Parameter from Stack
      ld   hl, #2               ;; [10] HL points to the start of parameters in the stack
      add  hl, sp               ;; [11]
      ld    a, (hl)             ;; [ 7] A = Selected Channel Bitmask

   cpct_akp_SFXGetInstrument_asm::  ;; Entry point for assembly calls using registers for parameter passing

      ;; Check the selected audio channel depending on bitmask
      rrca                      ;; [ 4] Rotate A right to test bit0 (Check for channel 0 / A)
      jp    c, sfcb_channel_a   ;; [10] if Carry=1, bit0 was 1, Channel A selected
      rrca                      ;; [ 4] Rotate A right to test bit1 (Check for channel 1 / B)
      jp    c, sfcb_channel_b   ;; [10] if Carry=1, bit1 was 1, Channel B selected
   
   sfcb_channel_c:       ;; If it is not channel B or C, is channel 0 / A (bit 0)
      ld   hl, (PLY_SFX_Track3_Instrument + 1) ;; [16] HL = SFX Track 3 (Channel 2 / C) Pitch Status
      ret                                      ;; [10] return
   
   sfcb_channel_b:
      ld   hl, (PLY_SFX_Track2_Instrument + 1) ;; [16] HL = SFX Track 2 (Channel 1 / B) Pitch Status
      ret                                      ;; [10] return
   
   sfcb_channel_a:
      ld   hl, (PLY_SFX_Track1_Instrument + 1) ;; [16] HL = SFX Track 1 (Channel 0 / C) Pitch Status
      ret                                      ;; [10] return


   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;;
   ;; Function: cpct_akp_SFXInit
   ;;
   ;;    Initializes sound effect instruments to be able to play sound effects.
   ;;
   ;; C Definition:
   ;;    void <cpct_akp_SFXInit> (void* *sfx_song_data*)
   ;;
   ;; Input Parameters (2 bytes):
   ;;  (2B DE) sfx_song_data - Pointer to the start of a song file containing instrument data for SFX
   ;;
   ;; Assembly call (Input parameters on registers):
   ;;    > call cpct_akp_SFXInit_asm
   ;;
   ;; Known limitations:
   ;;  * This function *will not work from ROM*, as it uses self-modifying code.
   ;;
   ;; Parameter Restrictions:
   ;;    * *sfx_song_data* must be a song in binary AKS format. However, this song 
   ;; only needs to have instruments defined (there is not need for an actual "song"
   ;; as a list of notes, patterns and others). Instruments will be set up for 
   ;; playing SFX later on with functions like <cpct_akp_SFXPlay>.
   ;;
   ;; Details:
   ;;    This function initializes instruments that will be used later on to play 
   ;; FX sounds at will using <cpct_akp_SFXPlay>. In order for the instruments to
   ;; be initialized, *sfx_song_data* must point to a song defined in AKS format.
   ;; For the purpose of this function, the song only requires to have instruments
   ;; defined in it, as patterns, notes and other information is not used for FX
   ;; sounds. 
   ;;
   ;;    You may use instruments from another song or a specific song containing
   ;; instrument data only. Any song with instruments defined in it is valid to
   ;; set up SFX with Arkos Tracker Player.
   ;;
   ;; Destroyed Register values: 
   ;;    AF, DE, HL
   ;;
   ;; Required memory:
   ;;    14 bytes (+13 bytes from <cpct_akp_SFXStopAll> that comes next to this
   ;; function and is used in initialization)
   ;;
   ;;    However, take into account that all of Arkos Tracker Player's
   ;; functions are linked and included, because they depend on each other. Total
   ;; memory requirement is around 2089 bytes.
   ;;
   ;; Time Measures:
   ;; (start code)
   ;;    Case    | Cycles | microSecs (us)
   ;; --------------------------------
   ;;    Any     |  146   |  36.50
   ;; --------------------------------
   ;; Asm saving |  -41   | -10.25
   ;; -------------------------------
   ;; (end code)
   ;;
   ;; Credits:
   ;;    This is a modification of the original <Arkos Tracker Player at
   ;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
   ;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
   ;; contributed to this source.
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   PLY_SFX_Init:
   _cpct_akp_SFXInit::
      ld  hl, #2                                   ;; [10] Get Parameter from Stack
      add hl, sp                                   ;; [11]
      ld  e, (hl)                                  ;; [ 7]
      inc hl                                       ;; [ 6]
      ld  d, (hl)                                  ;; [ 7] DE = Pointer to the SFX "Song"

   cpct_akp_SFXInit_asm::     ;; Entry point for assembly calls using registers for parameter passing

      ;Find the Instrument Table.
      ld  hl, #12                                  ;; [10]
      add hl, de                                   ;; [11]
      ld  (PLY_SFX_Play_InstrumentTable + 1), hl   ;; [16]

      ;; Initialization continues clearing sound effects from the 3 channels

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;;
   ;; Function: cpct_akp_SFXStopAll
   ;;
   ;;    Stops the reproduction of any sound effect in the 3 channels.
   ;;
   ;; C Definition:
   ;;    void <cpct_akp_SFXStopAll> ()
   ;;
   ;; Assembly call (Input parameters on registers):
   ;;    > call cpct_akp_SFXStopAll_asm
   ;;
   ;; Known limitations:
   ;;  * This function *will not work from ROM*, as it uses self-modifying code.
   ;;
   ;; Details:
   ;;    This function stops all sound FX reproduction on all the 3 sound channels.
   ;;
   ;; Destroyed Register values: 
   ;;    HL
   ;;
   ;; Required memory:
   ;;    13 bytes 
   ;;
   ;;    However, take into account that all of Arkos Tracker Player's
   ;; functions are linked and included, because they depend on each other. Total
   ;; memory requirement is around 2089 bytes.
   ;;
   ;; Time Measures:
   ;; (start code)
   ;;    Case    | Cycles | microSecs (us)
   ;; --------------------------------
   ;;    Any     |   68   |  15.50
   ;; --------------------------------
   ;; Asm saving |    0   |   0
   ;; -------------------------------
   ;; (end code)
   ;;
   ;; Credits:
   ;;    This is a modification of the original <Arkos Tracker Player at
   ;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
   ;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
   ;; contributed to this source.
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   _cpct_akp_SFXStopAll::
   cpct_akp_SFXStopAll_asm::  ;; Entry point for assembly calls 
   PLY_SFX_StopAll:
      ;Clear the three channels of any sound effect.
      ld  hl, #0                                   ;; [10]
      ld  (PLY_SFX_Track1_Instrument + 1), hl      ;; [16]
      ld  (PLY_SFX_Track2_Instrument + 1), hl      ;; [16]
      ld  (PLY_SFX_Track3_Instrument + 1), hl      ;; [16]
      ret                                          ;; [10]

   .equ PLY_SFX_OffsetPitch,        0
   .equ PLY_SFX_OffsetVolume,       PLY_SFX_Track1_Volume - PLY_SFX_Track1_Pitch
   .equ PLY_SFX_OffsetNote,         PLY_SFX_Track1_Note - PLY_SFX_Track1_Pitch
   .equ PLY_SFX_OffsetInstrument,   PLY_SFX_Track1_Instrument - PLY_SFX_Track1_Pitch
   .equ PLY_SFX_OffsetSpeed,        PLY_SFX_Track1_InstrumentSpeed - PLY_SFX_Track1_Pitch
   .equ PLY_SFX_OffsetSpeedCpt,     PLY_SFX_Track1_InstrumentSpeedCpt - PLY_SFX_Track1_Pitch

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;;
   ;; Function: cpct_akp_SFXPlay
   ;;
   ;;    Plays a concrete sound effect, using the instruments of the "SFX song" 
   ;; given to <cpct_akp_SFXInit> when initializing sound effects.
   ;;
   ;; C Definition:
   ;;    void <cpct_akp_SFXPlay> (<u8> *sfx_num*, <u8> *volume*, <u8> *note*, 
   ;; <u8> *speed*, <u16> *inverted_pitch*, <u8> *channel_bitmask*)
   ;;
   ;; Input Parameters (7 bytes):
   ;;  (1B L ) sfx_num         - Number of the instrument in the SFX Song (>0), same as the number given to the 
   ;; instrument in Arkos Tracker.
   ;;  (1B H ) volume          - Volume [0-15], 0 = off, 15 = maximum volume.
   ;;  (1B E ) note            - Note to be played with the given instrument [0-143]
   ;;  (1B D ) speed           - Speed (0 = As original, [1-255] = new Speed (1 is fastest))
   ;;  (2B BC) inverted_pitch  - Inverted Pitch (-0xFFFF -> 0xFFFF). 0 is no pitch. The higher the pitch, the lower the sound.
   ;;  (1B A ) channel_bitmask - Bitmask representing channels to use for reproducing the sound (Ch.A = 001 (1), Ch.B = 010 (2), Ch.C = 100 (4))
   ;;
   ;; Assembly call (Input parameters on registers):
   ;;    > call cpct_akp_SFXPlay_asm
   ;;
   ;; Known limitations:
   ;;  * This function *will not work from ROM*, as it uses self-modifying code.
   ;;
   ;; Details:
   ;;    Plays a given sound effect, along with the music, in a concrete channel 
   ;; and with some parameters (Volume, Note, Speed, Inverted Pitch). This lets
   ;; you create lots of different (and potentially complex) sound effects 
   ;; from a set of instruments. In fact, you could play a song made of sound
   ;; effect calls. 
   ;;
   ;; Destroyed Register values: 
   ;;    AF, BC, DE, HL,
   ;;
   ;; Required memory:
   ;;    95 bytes 
   ;;
   ;;    However, take into account that all of Arkos Tracker Player's
   ;; functions are linked and included, because they depend on each other. Total
   ;; memory requirement is around 2089 bytes.
   ;;
   ;; Time Measures:
   ;; (start code)
   ;;    Case    | Cycles  | microSecs (us)
   ;; --------------------------------------
   ;;    Any     |     To be done
   ;; --------------------------------------
   ;; Asm saving |  -141   |  -45.25
   ;; --------------------------------------
   ;; (end code)
   ;;
   ;; Credits:
   ;;    This is a modification of the original <Arkos Tracker Player at
   ;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
   ;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
   ;; contributed to this source.
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   _cpct_akp_SFXPlay::
   PLY_SFX_Play:
      ld  (PLY_SFX_Recover_IX+2), ix   ;; [20] Save IX value (cannot use push as parameters are on the stack)
      pop  af                          ;; [10]
      pop  hl                          ;; [10]
      pop  de                          ;; [10]
      pop  bc                          ;; [10]
      pop  ix                          ;; [14]
      push ix                          ;; [15]
      push bc                          ;; [11]
      push de                          ;; [11]
      push hl                          ;; [11]
      push af                          ;; [11]

      .dw #0x7DDD  ; ld a, ixl         ;; [ 8] A = Channel number

      cpct_akp_SFXPlay_asm::     ;; Entry point for assembly calls using registers for parameter passing

      ;; Pick up the selected audio channel using bitmasks
      ;; to reproduce the sound effect
      rrca                             ;; [ 4] Check for channel 0 / A (bit 0)
      jp    c, sfp_channel_a           ;; [10] 
      rrca                             ;; [ 4] Check for channel 1 / B (bit 1)
      jp    c, sfp_channel_b           ;; [10]
      
      sfp_channel_c:                   ;; If it is not channel A or B, is channel 2 / C (bit 2)
      ld   ix, #PLY_SFX_Track3_Pitch   ;; [14] IX = Track 3 (channel 2 / C)
      jp  PLY_SFX_Play_Selected        ;; [10] 
      
      sfp_channel_b:
      ld   ix, #PLY_SFX_Track2_Pitch   ;; [14] IX = Track 2 (channel 1 / B)
      jp  PLY_SFX_Play_Selected        ;; [10]
      
      sfp_channel_a:
      ld   ix, #PLY_SFX_Track1_Pitch   ;; [14] IX = Track 1 (Channel 0 / A)

   PLY_SFX_Play_Selected:
      ld  PLY_SFX_OffsetPitch + 1(ix), c        ;Set Pitch
      ld  PLY_SFX_OffsetPitch + 2(ix), b
      ld   a, e                                 ;Set Note
      ld  PLY_SFX_OffsetNote (ix), a
      ld   a, #15                               ;Set Volume
      sub  h
      ld  PLY_SFX_OffsetVolume (ix), a
      ld   h, #0                                ;Set Instrument Address
      add hl, hl

   PLY_SFX_Play_InstrumentTable: 
      ld  bc, #0
      add hl, bc
      ld   a, (hl)
      inc hl
      ld   h, (hl)
      ld   l, a
      ld   a, d                                 ;Read Speed or use the user's one ?
      or   a
      jr  nz, PLY_SFX_Play_UserSpeed
      ld   a, (hl)                              ;Get Speed

   PLY_SFX_Play_UserSpeed:
      ld  PLY_SFX_OffsetSpeed + 1 (ix), a
      ld  PLY_SFX_OffsetSpeedCpt + 1 (ix), a
      inc hl                                    ;Skip Re-trig
      inc hl
      ld  PLY_SFX_OffsetInstrument + 1 (ix), l
      ld  PLY_SFX_OffsetInstrument + 2 (ix), h

   PLY_SFX_Recover_IX:
      ld  ix, #0                                ; [14] 0 is a placeholder to save the previous value of IX

      ret

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   ;;
   ;; Function: cpct_akp_SFXStop
   ;;
   ;;    Stops the reproduction sound FX on given channels
   ;;
   ;; C Definition:
   ;;    void <cpct_akp_SFXStop> (<u8> *stop_bitmask*)
   ;;
   ;; Input Parameters (1 byte):
   ;;  (1B A) stop_bitmask - A value where the 3 Least Significant Bits represent which channels to stop 
   ;; (bits enabled = channels to stop)
   ;;
   ;; Assembly call (Input parameters on registers):
   ;;    > call cpct_akp_SFXStop_asm
   ;;
   ;; Known limitations:
   ;;  * This function *will not work from ROM*, as it uses self-modifying code.
   ;;
   ;; Parameter Restrictions:
   ;;    * *stop_bitmask* must be a value that operates as a set of enabled / 
   ;; disabled bits (a bitmask). Concretely, the 3 Least Significant bits, 2, 1 
   ;; and 0 (xxxxx210) refer to Channels C, B and A respectively. Rest of the 
   ;; bits are ignored. Bits set to 1 mean that those channels will be stopped. 
   ;; Bits set to 0 mean that those channels are to be left as they are.
   ;;
   ;; Details:
   ;;    This function lets you selectively stop sound FX reproduction on one, two
   ;; or the 3 available channels. A *stop_bitmask* is given as parameter containing
   ;; the information about what channels shall be stopped.
   ;;
   ;; Destroyed Register values: 
   ;;    AF, HL
   ;;
   ;; Required memory:
   ;;    30 bytes 
   ;;
   ;;    However, take into account that all of Arkos Tracker Player's
   ;; functions are linked and included, because they depend on each other. Total
   ;; memory requirement is around 2082 bytes.
   ;;
   ;; Time Measures:
   ;; (start code)
   ;; Case       | Cycles | microSecs (us)
   ;; --------------------------------
   ;; Best (0)   |  101   |  25.25 
   ;; --------------------------------
   ;; Best (1)   |  149   |  37.25 
   ;; --------------------------------
   ;; Asm saving |  -28   |  -7.00
   ;; --------------------------------
   ;; (end code)
   ;;
   ;; Credits:
   ;;    This is a modification of the original <Arkos Tracker Player at
   ;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
   ;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
   ;; contributed to this source.
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   _cpct_akp_SFXStop::
   PLY_SFX_Stop:
      ld   hl, #2                             ;; [10] Get Parameter from Stack
      add  hl, sp                             ;; [11]
      ld    a, (hl)                           ;; [ 7] A = Channel number to be stopped

   cpct_akp_SFXStop_asm::     ;; Entry point for assembly calls using registers for parameter passing

      ld   hl, #0                             ;; [10] Value 0 to stop SFX in a channel

      rrca                                    ;; [ 4] Test bit 0 (00000001) to know if channel 0 has to be stopped
      jp   nc, PLY_SFSStop_no1                ;; [10] If Carry=0, channel 2 is left as is.
      ld (PLY_SFX_Track1_Instrument + 1), hl  ;; [16] Stop Channel 0

   PLY_SFSStop_no1:
      rrca                                    ;; [ 4] Test bit 1 (00000010) to know if channel 1 has to be stopped
      jp   nc, PLY_SFSStop_no2                ;; [10] If Carry=0, channel 1 is left as is.
      ld (PLY_SFX_Track2_Instrument + 1), hl  ;; [16] Stop Channel 1

   PLY_SFSStop_no2:
      rrca                                    ;; [ 4] Test bit 2 (00000100) to know if channel 0 has to be stopped
      jp  z, PLY_SFSStop_no3                  ;; [10] If Carry=0, channel 0 is left as is.
      ld (PLY_SFX_Track3_Instrument + 1), hl  ;; [16] Stop Channel 2

   PLY_SFSStop_no3:
      ret                                     ;; [10] Return

.endif

.if PLY_UseFades
   
   ;E = Fade value (0 = full volume, 16 or more = no volume).

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_akp_setFadeVolume
;;
;;    Sets global volumes for creating fade in / out effects.
;;
;; C Definition:
;;    void <cpct_akp_setFadeVolume> (<u8> *volume*)
;;
;; Input Parameters (1 byte):
;;  (1B A) volume - Global volume for all channels, [0-15]. 0 = max volume, 16 or more = no volume
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_akp_setFadeVolume_asm
;;
;; Known limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Parameter Restrictions:
;;    * *volume* must be a value in the range [0-15] for some meaning. Values 
;; greater than 15 mean no volume at all. 0 is the maximum volume, 15 the minimum.
;;
;; Details:
;;    This function controls the global reproduction volume for all channels. This
;; lets you do Fade in / out effects, by making the volume progressively go up 
;; or down.
;;
;; Destroyed Register values: 
;;    AF, HL
;;
;; Required memory:
;;    15 bytes 
;;
;;    However, take into account that all of Arkos Tracker Player's
;; functions are linked and included, because they depend on each other. Total
;; memory requirement is around 2082 bytes.
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; -------------------------------------
;;    Any     |   51   |  12.75
;; -------------------------------------
;; Asm saving |  -28   |   7.00
;; -------------------------------------
;; (end code)
;;
;; Credits:
;;    This is a modification of the original <Arkos Tracker Player at
;; http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start> 
;; code from Targhan / Arkos. Madram / Overlander and Grim / Arkos have also 
;; contributed to this source.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
_cpct_akp_setFadeVolume::
   ;; Get parameter from the stack
   ld    hl, #2      ;; [10]
   add   hl, sp      ;; [11]
   ld     a, (hl)    ;; [ 7] A = Fade Volume 

cpct_akp_setFadeVolume_asm::     ;; ASM Entry point for setFadeVolume
   PLY_SetFadeValue:
      ld  (PLY_Channel1_FadeValue + 1), a   ;; [ 7] Set fade volume for channel A (0)
      ld  (PLY_Channel2_FadeValue + 1), a   ;; [ 7] Set fade volume for channel B (1)
      ld  (PLY_Channel3_FadeValue + 1), a   ;; [ 7] Set fade volume for channel C (2)
      ret                                   ;; [10]
.endif

.if PLY_SystemFriendly
   ;Save Interrupt status and Disable Interruptions
   PLY_DisableInterruptions:
      ld   a, i
      di

      ;IFF in P/V flag.
      ;Prepare opcode for DI.
      ld   a, #0xF3
      jp  po, PLY_DisableInterruptions_Set_Opcode

      ;Opcode for EI.
      ld   a, #0xFB

   PLY_DisableInterruptions_Set_Opcode:
      ld  (PLY_RestoreInterruption), a
      ret
.endif
