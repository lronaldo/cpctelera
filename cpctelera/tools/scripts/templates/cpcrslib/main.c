//
// Copyright (c) 2008-2015 Raúl Simarro <artaburu@hotmail.com>
// Permission is hereby granted, free of charge, to any person obtaining a copy of this 
// software and associated documentation files (the "Software"), to deal in the Software 
// without restriction, including without limitation the rights to use, copy, modify, 
// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <cpcrslib.h>

char main (void) {
   unsigned char z=0;

   cpc_DisableFirmware();     //Now, I don't gonna use any firmware routine so I modify interrupts jump to nothing
   cpc_ClrScr();           //fills scr with ink 0
   cpc_SetMode(1);            //hardware call to set mode 1

   cpc_SetColour(0,20);        //set background = black
   cpc_SetColour(16,20);       //set border = black

   cpc_PrintGphStrStd(1,"THIS IS A SMALL DEMO", (int)0xc050); //parameters: pen, text, adress
   cpc_PrintGphStrStd(2,"OF MODE 1 TEXT WITH", (int)0xc0a0);
   cpc_PrintGphStrStd(3,"8x8 CHARS WITHOUT FIRMWARE", (int)0xc0f0);
   cpc_PrintGphStrStdXY(3,"AND A SMALL SOFT SCROLL DEMO",8,70);
   cpc_PrintGphStrStdXY(2,"CPCRSLIB (C) 2015",19,80);
   cpc_PrintGphStrStdXY(1, "-- FONT BY ANJUEL  2009  --",2,160);
   cpc_PrintGphStrStdXY(1,"ABCDEFGHIJKLMNOPQRSTUVWXYZ",2,174);

    //while (cpc_AnyKeyPressed()==0){}
   while (cpc_AnyKeyPressed()==0) {       //Small scrolling effect
      z = !z;
      if (z) {
         cpc_RRI (0xe000, 40, 79);
         cpc_RRI (0xe4b0, 32, 79);
      }
      //cpc_RRI (0xe5f0, 12, 79);
      cpc_RLI (0xe5f0+0x50+0x50+79, 12, 79);
   }

   while (cpc_AnyKeyPressed()==0){}
   cpc_EnableFirmware();   //before exit, firmware jump is restored
   return 0;
}
