/*--------------------------------------------------------------------------------------

 dmd_test.cpp 
   Demo and example project for the Freetronics DMD, a 512 LED matrix display
   panel arranged in a 32 x 16 layout.

 Copyright (C) 2011 Marc Alexander (info <at> freetronics <dot> com)

 See http://www.freetronics.com/dmd for resources and a getting started guide.

 Note that the DMD library uses the SPI port for the fastest, low overhead writing to the
 display. Keep an eye on conflicts if there are any other devices running from the same
 SPI port, and that the chip select on those devices is correctly set to be inactive
 when the DMD is being written to.

 USAGE NOTES
 -----------

 - Place the DMD library folder into the "arduino/libraries/" folder of your Arduino installation.
 - Get the TimerOne library from here: http://code.google.com/p/arduino-timerone/downloads/list
   or download the local copy from the DMD library page (which may be older but was used for this creation)
   and place the TimerOne library folder into the "arduino/libraries/" folder of your Arduino installation.
 - Restart the IDE.
 - In the Arduino IDE, you can open File > Examples > DMD > dmd_demo, or dmd_clock_readout, and get it
   running straight away!

 * The DMD comes with a pre-made data cable and DMDCON connector board so you can plug-and-play straight
   into any regular size Arduino Board (Uno, Freetronics Eleven, EtherTen, USBDroid, etc)
  
 * Please note that the Mega boards have SPI on different pins, so this library does not currently support
   the DMDCON connector board for direct connection to Mega's, please jumper the DMDCON pins to the
   matching SPI pins on the other header on the Mega boards.

 This example code is in the public domain.
 The DMD library is open source (GPL), for more see DMD.cpp and DMD.h

--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD2.h>        //
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial_black_16.h>
#include <fonts/Arial_Black_16_ISO_8859_1.h>
#include <fonts/Arial14.h>
#include <fonts/Arial_Black_16_Utf_8.h>
#include <fonts/My32x15Font.h>
#include <fonts/Font_1.h>

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 2
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 5, 4, 12, 15);  // DMD controls the entire display

long timeTimer = 400000L; // 250us
long t = 0;
void setup(void)
{

  Serial.begin(9600);
  dmd.setBrightness(1);
  dmd.selectFont(Arial14);
  dmd.begin();
  //  clear/init the DMD pixels held in RAM
  dmd.clearScreen();   //true is normal (all pixels off), false is negative (all pixels on)
  // testHang();
  
  delay(1000);
}

/*--------------------------------------------------------------------------------------
  loop
  Arduino architecture main loop
--------------------------------------------------------------------------------------*/
bool flag ;
void loop(void)
{
  // if (flag) {
  //   dmd.setBrightness(255);
  // } else {
  //   dmd.setBrightness(127);
  // }
  // flag = !flag;
  // clock();
  // TestGraph();
  // testDrawChar();
  // testScroll(); // Not working

  // halfPexel(); // Khong hoat dong
   
  // HienThi2Hang();
   
  // draw a border rectangle around the outside of the display
  // testBorder();

  // testDrawImage();

  //  // stripe chaser
  // testStripeChaser(); // Khong hoat dong
  //  testWritePixel();
  // testDrawLine();
  testChar();
  // testCharUnicode();
  // testString();
}
void print(String s) {
  Serial.println(s);
}
void testChar() {
  dmd.clearScreen();
  // dmd.selectFont(Arial_Black_16);
  // dmd.selectFont(Arial_14);
  dmd.selectFont(Font_1);
  for (int i = 32; i < 262; i++) {
    dmd.clearScreen();
    print(String(i));
    dmd.drawChar(  0,  0, (i > 255 ? i - 255 : i), GRAPHICS_ON, (i > 255 ? 1 : 0));
    delay(1000);
  }
}
void testCharUnicode() {
  dmd.clearScreen();
  // dmd.selectFont(Arial_Black_16);
  // dmd.selectFont(Arial_14);
  dmd.selectFont(Arial_Black_16_Utf_8);
  print(String(128));
  dmd.drawChar(  4,  0, 128 , GRAPHICS_ON);
  long start=millis();
  long timer=start;
  boolean ret=false;
  while(1){
    if ((timer+100) < millis()) {
      dmd.marqueeScrollX(1);
      timer=millis();
    }
  }
}
void testString() {
  // dmd.clearScreen();
  // dmd.selectFont(Arial_Black_16_Utf_8);
  //  const char *MSG = "M""\x81""nh!";
  //  dmd.drawMarquee(MSG,strlen(MSG),(32*DISPLAYS_ACROSS)-1,0);
  //  dmd.clearScreen();
   dmd.selectFont(My32x15Font);
   // Français, Österreich, Magyarország
  //  const char *MSG = "M""\x81""nh.123456789";
  const char *MSG = "!";
  //  dmd.drawMarquee(MSG,strlen(MSG),(32*DISPLAYS_ACROSS)-1,0);

  dmd.fillScreen(false);
  dmd.drawString(0,0, MSG, GRAPHICS_ON); 
  // const char *MSG1 = "abc12345";
  // //  dmd.drawMarquee(MSG,strlen(MSG),(32*DISPLAYS_ACROSS)-1,0);
  // // dmd.fillScreen(false);
  //  dmd.drawString(0,16, MSG1, GRAPHICS_ON); 
   // /////////////////////////////////////////////////////////////////////////////////////
  //  Lỗi không dịch chuyển dc 
   ////////////////////////////////////////////////////////////////////////////////////////
   long start=millis();
   long timer=start;
   while(1){
     if ((timer+1000) < millis()) {
       dmd.marqueeScrollX(1);
       timer=millis();
     }
   }
}
void clock() {
  // 10 x 14 font clock, including demo of OR and NOR modes for pixels so that the flashing colon can be overlayed
   dmd.clearScreen();
   dmd.selectFont(Arial_Black_16);
   dmd.drawChar(  0,  3, '2', GRAPHICS_ON );
   dmd.drawChar(  7,  3, '3', GRAPHICS_ON );
   dmd.drawChar( 17,  3, '4', GRAPHICS_ON );
   dmd.drawChar( 25,  3, '5', GRAPHICS_ON );
   dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );   // clock colon overlay on
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_NOR    );   // clock colon overlay off
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );   // clock colon overlay on
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_NOR    );   // clock colon overlay off
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );   // clock colon overlay on
   delay( 1000 );
}
// Khong hoat dong
void TestGraph() {
   dmd.clearScreen();
   dmd.selectFont(Arial_Black_16);
   dmd.drawChar( 15,  3, ':', GRAPHICS_NOR    );   // clock colon overlay off
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );   // clock colon overlay on
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_NOR    );   // clock colon overlay off
   delay( 1000 );
   dmd.drawChar( 15,  3, ':', GRAPHICS_OR     );   // clock colon overlay on
   delay( 1000 );
}
void testDrawChar() {
  int cot = 10;
  int hang = 0;
  dmd.clearScreen();
  dmd.selectFont(Arial_Black_16);
  dmd.drawChar(  cot,  hang, '3', GRAPHICS_OFF );
  delay(5000);
}

// void testScroll() {
//   dmd.clearScreen();
//   dmd.selectFont(Arial_Black_16);
//   dmd.drawMarquee("Scrolling Text",14,(32*DISPLAYS_ACROSS)-1,0);
//   long start=millis();
//   long timer=start;
//   boolean ret=false;
//   while(!ret){
//     if ((timer+100) < millis()) {
//       ret=dmd.marqueeScrollX( 1 );
//       timer=millis();
//     }
//   }
// }
// ////////////////////////////////////////////////////////////////////////

void halfPexel() {
  dmd.clearScreen();
  dmd.selectFont(Arial_Black_16);
  // half the pixels on
   dmd.drawTestPattern( PATTERN_ALT_0 ); //  Không hoạt động drawTestPattern
   delay( 1000 );

   // the other half on
   dmd.drawTestPattern( PATTERN_ALT_1 );
   delay( 1000 );

  dmd.drawTestPattern( PATTERN_STRIPE_0 );
   delay( 1000 );

  dmd.drawTestPattern( PATTERN_STRIPE_1 );
   delay( 1000 );
}

void HienThi2Hang() {
  // display some text
  //  dmd.clearScreen();  // Làm nháy màn màn hình
   dmd.selectFont(System5x7);
   for (byte x=0;x<DISPLAYS_ACROSS;x++) {
     for (byte y=0;y<DISPLAYS_DOWN;y++) {
       dmd.drawString(  (32*x),  (16*y), "freet", GRAPHICS_ON );
       dmd.drawString(  (32*x),  8 +(16*y), "ronic", GRAPHICS_ON );
     }
   }
  //  while(1);
  delay( 500 );
}

void testBorder() {
   // draw a border rectangle around the outside of the display
   dmd.clearScreen();
   dmd.drawBox(  0,  0, (32*DISPLAYS_ACROSS)-1, (16*DISPLAYS_DOWN)-1, GRAPHICS_ON );
   delay( 1000 );
   
}

void testDrawImage() {
  for (byte y=0;y<DISPLAYS_DOWN;y++) {
    for (byte x=0;x<DISPLAYS_ACROSS;x++) {
       // draw an X
       int ix=32*x;
       int iy=16*y;
       dmd.drawLine(  0+ix,  0+iy, 11+ix, 15+iy, GRAPHICS_ON );
       dmd.drawLine(  0+ix, 15+iy, 11+ix,  0+iy, GRAPHICS_ON );
       delay( 1000 );
   
       // draw a circle
       dmd.drawCircle( 16+ix,  8+iy,  5, GRAPHICS_ON );
       delay( 1000 );
   
       // draw a filled box
       dmd.drawFilledBox( 24+ix, 3+iy, 29+ix, 13+iy, GRAPHICS_ON );
       delay( 1000 );
     }
   }
   ////////////////////////////////////////////////////////////////////////////// 
  long start=millis();
  long timer=start;
  boolean ret=true;
  while(ret){
    dmd.marqueeScrollX( 1 );
    delay(100);
  }
}
// ///////////////////////////////////////////////////////////////////////////////////
//  Khong hoat dong
void testStripeChaser() {
  //   for(byte b = 0 ; b < 20 ; b++ )
  //  {
  //     dmd.drawTestPattern( (b&1)+PATTERN_STRIPE_0 );
  //     delay( 200 );      
  //  }
  //  delay( 200 );      
}

void testWritePixel(){
  dmd.fillScreen(true);
  delay(2000);
   dmd.fillScreen(false);
  delay(2000);   
  dmd.setPixel(1, 1, GRAPHICS_ON);
  delay(2000);
  dmd.fillScreen(true);
  dmd.setPixel(1, 1, GRAPHICS_OFF);
  delay(2000);
}
void testDrawLine() {
   dmd.clearScreen();
  dmd.drawLine(0,1, 31, 1, GRAPHICS_ON);
  dmd.drawLine(0,3, 31, 3, GRAPHICS_ON);
   dmd.drawLine(0,0, 15, 10, GRAPHICS_ON);
  delay(1000);
}