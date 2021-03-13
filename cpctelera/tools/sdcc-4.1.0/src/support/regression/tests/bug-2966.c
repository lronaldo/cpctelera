/* bug-2966.c
   An error in instruction size estimation for z80 resulted in an out-of-range relative jump.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

#define PORT_A_KEY_UP           0x0001
#define PORT_A_KEY_DOWN         0x0002
#define PORT_A_KEY_LEFT         0x0004
#define PORT_A_KEY_RIGHT        0x0008
#define PORT_A_KEY_1            0x0010
#define PORT_A_KEY_2            0x0020

#define PORT_B_KEY_UP           0x0040
#define PORT_B_KEY_DOWN         0x0080
#define PORT_B_KEY_LEFT         0x0100
#define PORT_B_KEY_RIGHT        0x0200
#define PORT_B_KEY_1            0x0400
#define PORT_B_KEY_2            0x0800

void displayOn (void) {
  return;
}

void SMS_waitForVBlank (void) {
  return;
}

void PSGFrame (void) {
  return;
}

void PSGStop (void) {
  return;
}

unsigned int filter_paddle (unsigned int i) {
  return i;
}

unsigned int SMS_getKeysPressed (void) {
  return PORT_B_KEY_2;
}

void PSGPlay (void *p) {
  return;
}

void *CH0_psgc, *CH1_psgc, *CH2_psgc, *CH3_psgc, *VolumeTest_psgc;

unsigned int kp;

void testBug (void) {
  displayOn();

  for(;;) {
    SMS_waitForVBlank();
    PSGFrame();
    kp=filter_paddle(SMS_getKeysPressed());
    if (kp & (PORT_A_KEY_UP|PORT_B_KEY_UP))
      PSGPlay(CH0_psgc);
    if (kp & (PORT_A_KEY_RIGHT|PORT_B_KEY_RIGHT))
      PSGPlay(CH1_psgc);
    if (kp & (PORT_A_KEY_DOWN|PORT_B_KEY_DOWN))
      PSGPlay(CH2_psgc);
    if (kp & (PORT_A_KEY_LEFT|PORT_B_KEY_LEFT))
      PSGPlay(CH3_psgc);
    if (kp & (PORT_A_KEY_1|PORT_B_KEY_1))
      PSGPlay(VolumeTest_psgc);
    if (kp & (PORT_A_KEY_2|PORT_B_KEY_2))
      break;
  }
  PSGStop();
}

