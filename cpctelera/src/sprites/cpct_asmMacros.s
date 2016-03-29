;; Useful macros for assembly programming
;;
;; Check all TODOs in this file
;; TODO: Check if it's possible to modify absolutely locating macros to prevent them from inserting an aditional byte
;; TODO: Try to genterate macros to add translation tables at fixed memory locations
;;

;; macro: _reverse_bits_in_A
;;    Reverts bits of A register ([01234567] => [76543210])
;;    
;; >> Input: A = TReg = Byte to be reversed
;; >> Modifies: AF, TReg
;; >> 16 microseconds
;; >> 16 bytes
;; TODO: Parameterize selection masks to reuse macro for different inversions
;;
.macro _reverse_bits_of_A  TReg
   rlca            ;; [1] | Rotate left twice so that...
   rlca            ;; [1] | ... A=[23456701]

   ;; Mix bits of TReg and A so that all bits are in correct relative order
   ;; but displaced from their final desired location
   xor  TReg       ;; [1] TReg = [01234567] (original value)
   and #0b01010101 ;; [2]    A = [23456701] (bits rotated twice left)
   xor  TReg       ;; [1]   A2 = [03254761] (TReg mixed with A to get bits in order)
   
   ;; Now get bits 54 and 10 in their right location and save them into TReg
   rlca            ;; [1]    A = [ 32 54 76 10 ] (54 and 10 are in their desired place)
   ld TReg, a      ;; [1] TReg = A (Save this bit location into TReg)
   
   ;; Now get bits 76 and 32 in their right location in A
   rrca            ;; [1] | Rotate A right 4 times to...
   rrca            ;; [1] | ... get bits 76 and 32 located at their ...
   rrca            ;; [1] | ... desired location :
   rrca            ;; [1] | ... A = [ 76 10 32 54 ] (76 and 32 are in their desired place)
   
   ;; Finally, mix bits from TReg and A to get all bits reversed into A
   xor  TReg       ;; [1] TReg = [32547610] (Mixed bits with 54 & 10 in their right place)
   and #0b11001100 ;; [2]    A = [76103254] (Mixed bits with 76 & 32 in their right place)
   xor  TReg       ;; [1]   A2 = [76543210] final value: bits of A reversed
.endm

;; macro: _revertpixels_m1_a
;;    Inverts order of pixels in A, according to mode 1 format 
;; ([01234567] => [32107654])
;;    
;; >> Input: A = TReg = Pixels to be reverted (mode 1 format)
;; >> Modifies: AF, TReg
;; >> 16 microseconds
;; >> 16 bytes
;; TODO: review if modified register (C) can be passed as parameter
;; TODO: Make it possible to configure the initial copy from A to C
;;
.macro _reverse_mode_1_pixels_of_A  TReg
   rlca            ;; [1] | Rotate left twice so that...
   rlca            ;; [1] | ... A=[23456701]

   ;; Mix bits of TReg and A so that all bits are in correct relative order
   ;; but displaced from their final desired location
   xor TReg        ;; [1] TReg = [01234567] (original value)
   and #0b01010101 ;; [2]    A = [23456701] (bits rotated twice left)
   xor TReg        ;; [1]   A2 = [03254761] (TReg mixed with A to get bits in order)
   
   ;; Now get bits 32 & 76 in their right location and save them into C
   rlca            ;; [1]    A = [ 32 54 76 10 ] (32 & 76 are in their desired place)
   ld TReg, a      ;; [1] TReg = A (Save this bit location into TReg)
   
   ;; Now get bits 10 and 54 in their right location in A
   rrca            ;; [1] | Rotate A right 4 times to...
   rrca            ;; [1] | ... get bits 10 and 54 located at their ...
   rrca            ;; [1] | ... desired location :
   rrca            ;; [1] | ... A = [ 76 10 32 54 ] (10 and 54 are in their desired place)
   
   ;; Finally, mix bits from C and A to get all bits reversed into A
   xor TReg        ;; [1] TReg = [32547610] (Mixed bits with 32 & 76 in their right place)
   and #0b00110011 ;; [2]    A = [76103254] (Mixed bits with 10 & 54 in their right place)
   xor TReg        ;; [1]   A2 = [32107654] final value: mode 1 pixels of A reversed
.endm