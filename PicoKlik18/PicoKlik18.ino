///////////////////////////////////////////////////////////////////////////////////////////
// T van Dyk Aug 2024 GPL3
//
// Makes use of (RP2040 Pico) and based on (modifications and additions):
// USBMidiKliK4x4 at https://github.com/TheKikGen/USBMidiKliK4x4/tree/master
// USBMidiKliK4x4 License as shown below
//
// Also based on TouchMacroPadPico
// https://github.com/TobiasVanDyk/Pico-MCU-from-Raspberry-Pi/tree/main/TouchMacroPadPico
// and the acknowledgements that it contains
///////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************
 Adafruit invests time and resources providing this open source code, please support 
 Adafruit and open-source hardware by purchasing products from Adafruit
 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
*************************************************************************************/
// ---------------------------------------------------------------------------------------------
// Note that this file is NOT part of the USBMIDIKLIK-4x4 distribution as listed below:
//
// USBMIDIKLIK 4X4 - USB Midi advanced firmware for STM32F1 platform.
// Copyright (C) 2019 by The KikGen labs.
// LICENCE CREATIVE COMMONS - Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)// 
// https://github.com/TheKikGen/USBMidiKliK4x4
// Copyright (c) 2019 TheKikGen Labs team.
// ---------------------------------------------------------------------------------------------

#include "Adafruit_TinyUSB.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <LittleFS.h>
#include <MIDI.h>

// Create the Serial MIDI port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

int LCDBackLight = 13; // Will be set as OUTPUT and LOW by TFT init

// Used to dim LCD Backlight if 60 seconds no touch
unsigned long NowMillis = 0;           // LCD Backlight Saver
unsigned long LastMillis = 0;          // LCD Backlight Saver
unsigned long TimePeriod = 15000;      // 60 seconds LCD Backlight Timer
bool BackLightOn = true;               // LCD Backlight Saver
int Layout = 3;                        // Layout 1, 2 or 3, 4

//////////////////////////////////////////////////////////////////////////////////////////////
//uint16_t calData[5] = { 243, 3398, 444, 3318, 1 };     
uint16_t calData[5];                              
uint8_t  calDataOK = 0;
#define CalFile "TouchCalData"  // Name of the Touch calibration file
bool DoCal = false;             // Set true if you want the calibration to run on each boot
////////////////////////////////////////////////////////////////////////////////////////////// 

uint16_t t_x = 0, t_y = 0;      // Touch coordinates

int KeyNum = 25; // 25 for Layout 1 and 12 for Layouts 2 3 4

// 25 Keys: 5x5 XPoint Switch Key sizes and spacing
#define KEY_W 42           // Key width
#define KEY_H 32           // Key height
#define KEY_SPACING_X 10   // X gap
#define KEY_SPACING_Y 10   // Y gap
// Keypad start position
#define KEY_X 28           // 26 is off-screen on left
#define KEY_Y 18           // topmost with no cut-off

// 12 Keys: sizes and spacing
#define key_W 70 // Key width  65 x 4 = 260 + 12 x 5 = 320  
#define key_H 60 // Key height 60 x 3 = 180 + 15 x 4 = 240
#define key_SPACING_X 8    // 12 // X gap
#define key_SPACING_Y 8    // 15 // Y gap
#define key_X 38                             // 30 is off-screen on left
#define key_Y 36                             // 30 is the topmost with no cut-off

#define KEY_TEXTSIZE 1   // Font size multiplier

// Status line for messages
#define STATUS_X 160 // Centred on this
#define STATUS_Y 225

// Adding a delay between keypressing to give our OS time to respond
uint8_t keydelay = 50;  // 100;  // keycode[1,..,6] delay milliseconds
uint8_t keydelay2 = 25;          // keyPress delay
uint16_t keydelay3 = 500;        // used in UAC confirm

// Create the screen object
TFT_eSPI tft = TFT_eSPI();

//////////////////////////////////////////////////////
// RGB565 Color Picker
// http://www.barth-dev.de/online/rgb565-color-picker/
//////////////////////////////////////////////////////
#define Black    0x0000      /*   0,   0,   0 */
#define Navy     0x000F      /*   0,   0, 128 */
#define DGreen   0x03E0      /*   0, 128,   0 */
#define DCyan    0x03EF      /*   0, 128, 128 */
#define Maroon   0x7800      /* 128,   0,   0 */
#define Purple   0x780F      /* 128,   0, 128 */
#define Olive    0x7BE0      /* 128, 128,   0 */
#define LGrey    0xD69A      /* 211, 211, 211 */
#define DGrey    0x7BEF      /* 128, 128, 128 */
#define Blue     0x001F      /*   0,   0, 255 */
#define Green    0x07E0      /*   0, 255,   0 */
#define Cyan     0x07FF      /*   0, 255, 255 */
#define Red      0xF800      /* 255,   0,   0 */
#define Magenta  0xF81F      /* 255,   0, 255 */
#define Yellow   0xFFE0      /* 255, 255,   0 */
#define White    0xFFFF      /* 255, 255, 255 */
#define Orange   0xFDA0      /* 255, 180,   0 */
#define Green1   0xB7E0      /* 180, 255,   0 */
#define Green2   0x26C8      
#define Green3   0x1750
#define Pink     0xFE19      /* 255, 192, 203 */ 
#define Brown    0x9A60      /* 150,  75,   0 */
#define Gold     0xFEA0      /* 255, 215,   0 */
#define Silver   0xC618      /* 192, 192, 192 */
#define Skyblue  0x867D      /* 135, 206, 235 */
#define Violet   0x915C      /* 180,  46, 226 */

// 25 button colours used in Layout 1 = IThru Routing and Layout 2 = Midi Jacks Routing
uint16_t Colours12[30] = {Blue, Blue, Blue,  Blue, Blue, Violet,    // L1 + L2
                          Blue, Blue, Blue,  Blue, Blue, Violet,  
                          Blue, Blue, Blue,  Blue, Blue, DGrey,
                          Blue, Blue, Blue,  Blue, Blue, Violet, 
                          Blue, Blue, Blue,  Blue, Blue, Violet };

// Two sets of 12 button colours used in Layouts 3 and 4
uint16_t Colours34[2][12] = {Violet, Violet, Violet, Violet,    // L3
                             Violet, Violet, Violet, Blue,   
                             Violet, Violet, Violet, Violet, 
                             Brown,  Brown,  Brown,  Brown,        // L4
                             Brown,  Brown,  Brown,  Maroon,     
                             Brown,  Brown,  Brown,  Brown, };
               
uint16_t keyColor12[30] = {};
uint16_t keyColor34[12] = {};

uint16_t BackgroundColor[4] = {Black, Black, Black, Black};   

uint16_t ButtonOutlineColor[4] = {White, White, White, LGrey};  

// Two sets of 5x5 25 Switches for Midi Xpoint switch + 5 Option keys
const static char Labels12[2][30][4] = 
{"a1", "a2", "a3", "a4", "a5", "mn",  
 "b1", "b2", "b3", "b4", "b5", "ch", 
 "c1", "c2", "c3", "c4", "c5", ">",
 "d1", "d2", "d3", "d4", "d5", "sx",
 "e1", "e2", "e3", "e4", "e5", "op", 
 "A1", "A2", "A3", "A4", "A5", "mn",  
 "B1", "B2", "B3", "B4", "B5", "ch", 
 "C1", "C2", "C3", "C4", "C5", ">",
 "D1", "D2", "D3", "D4", "D5", "sx",
 "E1", "E2", "E3", "E4", "E5", "op" }; 
 
char keyLabel12[30][4] = {};  // 25 5x5 XPoint switch + 4 Options keys and 1 Nav key
char keyLabel34[12][4] = {};  // 12 keys

// Two layout sets of 12 button labels <name>[<number-of-lables>][<number-of-chars-per-label]
// The number of chars per label include the termination char \0 => 3 character labels here
const static char Labels34[2][12][4] = 
{"S1", "S2", "S3", "Up", "S4", "S5", "S6", "Nxt", "S7", "S8", "S9", "Dwn",
 "T1", "T2", "T3", "Up", "T4", "T5", "T6", "Nxt", "T7", "T8", "T9", "Dwn" };
 
bool ae15[2][5][5] = {};   // 0=IThru 1=Midi JacksRouting Ports 1-5 = ae15[0-4][0-4] i.e. exclude Port 0

#define SysExHeader       0xF0,0x77,0x77,0x78,

// byte SetXPoint[2][11] = { 0xF0,0x77,0x77,0x78, 0x0E,0x03, 0x01, 0x01, 0x01, 0xF7, 0xF7,
//                           0xF0,0x77,0x77,0x78, 0x0F,0x01, 0x01, 0x01, 0x01, 0x01, 0xF7 };  
// F0 77 77 78 0E 03 <JackIn port > [<out port type> [<out ports list: nn...nn>] ] F7
// F0 77 77 78 0F 01 <in port type> <in port> <out port type>[out ports list: nn...nn] F7
byte SetXPoint[2][15] = { 0xF0,0x77,0x77,0x78, 0x0E,0x03, 0x01, 0x01, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
                          0xF0,0x77,0x77,0x78, 0x0F,0x01, 0x01, 0x01, 0x01, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7 }; 

static const byte X01[25][3] = 
{ 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x03, 0x01, 0x01, 0x04, 0x01, 0x01, 0x05, 
  0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x01, 0x03, 0x02, 0x01, 0x04, 0x02, 0x01, 0x05, 
  0x03, 0x01, 0x01, 0x03, 0x01, 0x02, 0x03, 0x01, 0x03, 0x03, 0x01, 0x04, 0x03, 0x01, 0x05, 
  0x04, 0x01, 0x01, 0x04, 0x01, 0x02, 0x04, 0x01, 0x03, 0x04, 0x01, 0x04, 0x04, 0x01, 0x05,
  0x05, 0x01, 0x01, 0x05, 0x01, 0x02, 0x05, 0x01, 0x03, 0x05, 0x01, 0x04, 0x05, 0x01, 0x05 }; 
     
// Get current iThru and Midi Jack Routing JackIn (A-E = 1-5) -> none or JackOut (1-5) configuration - Jack In0 Out0 connected to Pi Pico UART
// Returns F0 77 77 78 0E 04 00 06 00 38 00 00 00 00 02 00 00 00 F7 with 6+6 bytes data for IThru 0-5 and MidJacks 0-5
const static byte JackAE[] =     { SysExHeader 0x05, 0x0E, 0x04, 0x00, 0x00, 0xF7 };  // IThru + Midi Jacks JackIn A-E connected to ?

const static byte SX1[] = { SysExHeader 0x06, 0x08, 0xF7 };                    // 06 08 Reboot in configuration mode   F0 77 77 78 06 08 F7
const static byte SX2[] = { SysExHeader 0x06, 0x04, 0xF7 };                    // 06 04 Set to Factory settings        F0 77 77 78 06 04 F7
const static byte SX3[] = { SysExHeader 0x06, 0x00, 0xF7 };                    // 06 00 Reboot after Hardware reset    F0 77 77 78 06 00 F7
const static byte SX4[] = { SysExHeader 0x05, 0x7F, 0x00, 0x00, 0x00, 0xF7 };  // 05 7F Configuration sysex dump All   F0 77 77 78 05 7F 00 00 00 F7
const static byte SX5[] = { SysExHeader 0x05, 0x0E, 0x04, 0x00, 0x00, 0xF7 };  // 05 OE 04 Dump all Jacks iThru        F0 77 77 78 05 0E 04 00 00 F7
const static byte SX6[] = { SysExHeader 0x0E, 0x00, 0xF7 };                    // 0F 00 IThru routing Reset to default F0 77 77 78 0E 00 F7
const static byte SX7[] = { SysExHeader 0x0F, 0x00, 0xF7 };                    // 0F 00 Midi routing Reset to default  F0 77 77 78 0F 00 F7
const static byte SX8[] = { SysExHeader 0x06, 0x05, 0xF7 };                    // 06 05 Clear all (midi, Ithru routing rules and pipeline)
const static byte SX9[] = { SysExHeader 0x0E, 0x01, 0xF7 };                    // 0E 01 IThru Disable all              F0 77 77 78 0E 01 F7

const static byte TX1[] = { SysExHeader 0x0E, 0x02, 0x01, 0xF7 };              // 0E 02 Set USB idle 15   seconds      F0 77 77 78 0E 02 01 F7
const static byte TX2[] = { SysExHeader 0x0E, 0x02, 0x0A, 0xF7 };              // 0E 02 Set USB idle 150  seconds      F0 77 77 78 0E 02 0A F7
const static byte TX3[] = { SysExHeader 0x0E, 0x02, 0x64, 0xF7 };              // 0E 02 Set USB idle 1500 seconds      F0 77 77 78 0E 02 64 F7
const static byte TX4[] = { SysExHeader 0x06, 0x02, 0xF7 };                    // 06 02 Sysex acknowledgment on/off    F0 77 77 78 06 02 F7
const static byte TX5[] = { SysExHeader 0x0E, 0x03, 0x01, 0xF7 };              // 0E 03 Set JackIn 1 IThru on/off      F0 77 77 78 0E 03 01 F7
const static byte TX6[] = { SysExHeader 0x0E, 0x03, 0x03, 0xF7 };              // 0E 03 Set JackIn 3 IThru on/off      F0 77 77 78 0E 03 03 F7
const static byte TX7[] = { SysExHeader 0x0E, 0x03, 0x04, 0xF7 };              // 0E 03 Set JackIn 4 IThru on/off      F0 77 77 78 0E 03 04 F7
const static byte TX8[] = { SysExHeader 0x0E, 0x03, 0x05, 0xF7 };              // 0E 03 Set JackIn 5 IThru on/off      F0 77 77 78 0E 03 05 F7
//                TX9   =   Reboot RP2040 Pico

const int  ByteSize = 4096;      // 
const int  MaxBytes = ByteSize;  // 

static const char* digits = "0123456789ABCDEF";       // dec 2 hex digits positional map
char Int2HexStr[64] = "                                  ";

int  NumBytes = 0;
bool NewData = false;
bool CheckSerial = true;    // Switch off serial check unless *se* toggles CheckSerial - default is off

int SysExCount = 0;
bool GetIThru = true;
int GetIThruCount = 0;

// Create 30 keys 
TFT_eSPI_Button key[30];

/////////////////////////////////////
void DoSysEx(byte* arr, unsigned len)
/////////////////////////////////////
// Returns F0 77 77 78 0E 04 00 06 00 38 00 00 00 00 02 00 00 00 F7 with 6+6 bytes data for IThru 0-5 and MidiJacks 0-5
{ uint8_t i, JackOut1, JackOut2, JackIn; 
  
  if (GetIThru) { if (arr[0]==0xF0 && arr[4]==0x0E && arr[5]==0x04) 
                     {for (JackIn=1; JackIn<6; JackIn++)                                        // = arr[7] to arr[11]
                          {JackOut1 = arr[JackIn+6];                                            // JackOut bits in each arr[7] - [11]
                           JackOut2 = arr[JackIn+12];                                           // JackOut bits in each arr[13] - [17]
                           for (i=1; i<6; i++)                                                  // = each bit in arr[] = bit 1-5 = Jackout 1-6
                               {if (bitRead(JackOut1, i)==1) ae15[0][JackIn-1][i-1] = true;     // IThru routing
                                if (bitRead(JackOut2, i)==1) ae15[1][JackIn-1][i-1] = true;     // Midi Jack Routing 
                               }
                          }                  
                     }
                }                                                                               // aaawwhh indented brackets are so sweet           
    /*
    uint8_t n = 0; uint8_t m = 0; uint8_t a = 11;  uint8_t b;                     
    while (n<a) { b = arr[n+4];                                   // Contents in hex
                Int2HexStr[n+m]   = digits[(b >> 4) & 0x0F];   
                Int2HexStr[n+m+1] = digits[b & 0x0F]; n++; m=m+3; }    
    status((char *)Int2HexStr); 
    */          
}
/////////////////////////////////////////////////////////////////////////////////////////////
void DoIThruConf(byte Count) { if (Count==1) MIDI1.sendSysEx(sizeof(JackAE), JackAE, true); }
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
void setup() {
/////////////////////////////////////////////////////////
  
  pinMode(LED_BUILTIN, OUTPUT);   // Used for Capslock
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  delay(1000);                   // delay(12000); // If required for Midiklik boot
  //Serial1.begin(31250);
  //Serial1.setTimeout(100);  
  
  //if (!LittleFS.begin()) {LittleFS.format(); LittleFS.begin(); }   
  LittleFS.begin(); // LittleFs automatically format the filesystem if one is not detected
      
  // Initialise the TFT screen TFT_eSPI/TFT_eSPI.h
  tft.init();
  BackLightOn = true;                   // TFT init will turn it on
  analogWrite(LCDBackLight, 128);       // Backlight On 255=Off 0=Full-On 
  tft.setRotation(1);                   // Set the display orientation to 0, 1, 2 or 3
  touch_calibrate();                    // New Calibration Data obtained if DoCal = true
  tft.fillScreen(Black); 
  tft.setFreeFont(&FreeSans12pt7b);     // KEY_TEXTSIZE 
  tft.setTextSize(KEY_TEXTSIZE);
  tft.setTouch(calData);                
  
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOff();                     // Jack1InOut  -> MidiKlik5x5 
  MIDI1.setHandleSystemExclusive(DoSysEx); // to disable use MIDI1.disconnectCallbackFromType(midi::SystemExclusive);

  // Layouts 1 and 2 = XPoint 5x5 switches only initialised after first blank timeout set her at 15 seconds
  Layout = 3;                   // Layout 1/2, 3/4

  for (int p=0; p<2; p++) for (int r=0; r<5; r++) for (int c=0; c<5; c++) ae15[p][r][c] = false;
  
  if (Layout<3) ConfigButtons(1);              // Draw 5x5 Xpoint Switch
  if (Layout>2) ConfigKeys(1);                 // Draw 12 Keys  
  
  LastMillis = millis();        // initial start time  
}

/////////////////////////////
void loop() 
/////////////////////////////
{
  NowMillis = millis();                        // get the current "time" (number of milliseconds since started)
  if ((NowMillis - LastMillis) >= TimePeriod)  //test whether the dimming period has elapsed
  { LastMillis = NowMillis;                    // Start Timer again       
    status("");                                // Clear the status line
    GetIThruCount++;
    if (GetIThruCount<2) { DoIThruConf(GetIThruCount); }  // Sync 5x5 JackIn->JackOut (and UART0 hardwired for SysEx control) through 1 timeout
    analogWrite(LCDBackLight, 238); BackLightOn = false; 
  } // Until keypress    

  if (Layout<3) KeyNum=30; else KeyNum=12;
  bool pressed = tft.getTouch(&t_x, &t_y, 450);                              // True if valid key pressed
  for (uint8_t b = 0; b < KeyNum; b++) { if (pressed && key[b].contains(t_x, t_y)) key[b].press(true);   
                                                                             else  key[b].press(false); }                    
   
  // Check if any key changed state void press(bool p); bool isPressed() bool justPressed(); bool justReleased();
  for (uint8_t b = 0; b < KeyNum; b++) { if (key[b].justReleased())   key[b].drawButton();           // draw normal again
                                         if (key[b].justPressed())  { key[b].drawButton(true);       // invert button colours
                                                                      status("");                    // Clear the old status
                                                                      buttonpress(b);           } }  // Send the button number
  MIDI1.read(); // delay(1);                                                                
  
}  // main Loop

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// F0 77 77 78 0E 03 <JackIn port > [<out port type> [<out ports list: nn...nn>] ] F7
// F0 77 77 78 0F 01 <in port type> <in port> <out port type>[out ports list: nn...nn] F7
// byte SetXPoint[2][15] = { 0xF0,0x77,0x77,0x78, 0x0E,0x03, 0x01, 0x01, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
//                           0xF0,0x77,0x77,0x78, 0x0F,0x01, 0x01, 0x01, 0x01, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7 }; 
// F0 77 77 78 0F 01 01 01 01 01 F7 = A1 only one ON
// F0 77 77 78 0F 01 01 01 01 F7    = A1 off all others off
///////////////////////////////////////////////////////
bool DoXPoint(int r, int c, byte p, byte XKey) // p=0,1
///////////////////////////////////////////////////////
{ uint8_t a, t, b, m, n;
  char XPointStatus[] = "00  ";  
  int XPointSize = 14 + p;  
   
  ae15[p][r][c] = !ae15[p][r][c];
  bool aeX15 = ae15[p][r][c]; 
  
  SetXPoint[p][6 + p] = r + 1;     // JackIn 1-5
  SetXPoint[p][7 + p] = 0x01;      // OutType = Jack
  m=0; for (n=1; n<6; n++) if (ae15[p][r][n-1]) { SetXPoint[p][m + 8 + p] = n; m++; } 
  SetXPoint[p][m + 8 + p] = 0xF7;
   
  MIDI1.sendSysEx(XPointSize, SetXPoint[p], true);
  
  /*
  n = 4; m = 0; a = 12;             // n=4; starts at OE or OF               
  while (n<a) { b = SetXPoint[p][n];                                  
                Int2HexStr[n+m]   = digits[(b >> 4) & 0x0F];   
                Int2HexStr[n+m+1] = digits[b & 0x0F]; n++; m=m+3; }    
  status((char *)Int2HexStr); 
  */
  
  // XPointStatus[0] = Labels12[p][XKey][0];
  // XPointStatus[1] = Labels12[p][XKey][1];
  // if (aeX15) XPointStatus[3] = 'X';
  // status((char *)XPointStatus); 

  ConfigButtons(0); // No background clear required
  
  delay(keydelay);
  return aeX15;
}

//////////////////////////////////////////////////////////
// Set Layout 1 5x5 A-E Input x 1-5 Output Switch Matrix
//////////////////////////////////////////////////////////
void ae15Set(byte p)
{ int r, c;
  for (r=0; r<5; r++) for (c=0; c<5; c++) { if (ae15[p][r][c]) keyColor12[r*6+c] = Red; else keyColor12[r*6+c] = Blue; }
}

//////////////////////////////////////
void buttonpress(int xpoint)
//////////////////////////////////////
{ byte r, c, b;
  const static byte X2rcb[3][30] = { 0,0,0,0,0, 100, 1,1,1,1,1, 101, 2, 2, 2, 2, 2,  102, 3, 3, 3, 3, 3,  103, 4, 4, 4, 4, 4,  104,
                                     0,1,2,3,4, 100, 0,1,2,3,4, 101, 0, 1, 2, 3, 4,  102, 0, 1, 2, 3, 4,  103, 0, 1, 2, 3, 4,  104,
                                     0,1,2,3,4, 100, 5,6,7,8,9, 101, 10,11,12,13,14, 102, 15,16,17,18,19, 103, 20,21,22,23,24, 104 };  

  ///////////////////////////////////////// Skip 1st any-key keypress which activates backlight
  LastMillis = millis();                 // Reset Backlight Timer if any key pressed                                        
  if (!BackLightOn)                      // Keypress only active if backlight on
     {//digitalWrite(LCDBackLight, LOW); // Backlight On
      analogWrite(LCDBackLight, 128);    // Backlight On 255=Off 0=Full-On
      BackLightOn = true;
      return; } 
  
  if (Layout<3) { b = X2rcb[2][xpoint]; r = X2rcb[0][xpoint]; c = X2rcb[1][xpoint]; 
                  if (b<100) { DoXPoint(r, c, Layout-1, b); return; } 
                   
  switch(xpoint){                                           // Layout=1 or Layout=2
    case 5: ////////////////////////////////////////////////// Option1   
      if (Layout==1) { status("Filter IThru Channel"); delay(keydelay); break; }
      if (Layout==2) { status("Filter Midi Channel");  delay(keydelay); break; }
      break;    
    case 11: //////////////////////////////////////////////// Option2 Dwn
      if (Layout==1) { status("Filter IThru Notes"); delay(keydelay); break; }
      if (Layout==2) { status("Filter Midi Notes");  delay(keydelay); break; } 
      break; 
     case 17: //////////////////////////////////////////////// Nxt
      Layout++; if (Layout>2) ConfigKeys(1); else ConfigButtons(1);
      break;     
    case 23: //////////////////////////////////////////////// Option3
      if (Layout==1) { status("Filter IThru SysEx"); delay(keydelay); break; }    
      if (Layout==2) { status("Filter Midi SysEx");  delay(keydelay); break; }        
      break;      
    case 29: //////////////////////////////////////////////// Option4
      if (Layout==1) { status("Option"); delay(keydelay); break; }  
      if (Layout==2) { status("Option"); delay(keydelay); break; }    
      break;          
  }   // switch  
  }   // Layout=1 or Layout=2
 
  else //if (Layout>2)    
  switch(xpoint){
    case 0:
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX1), SX1, true); status("S1"); delay(keydelay); break; }
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX1), TX1, true); status("T1"); delay(keydelay); break; }         
      break;
    case 1: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX2), SX2, true); status("S2"); delay(keydelay); break; } 
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX2), TX2, true); status("T2"); delay(keydelay); break; }       
      break;
    case 2: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX3), SX3, true); status("S3"); delay(keydelay); break; } 
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX3), TX3, true); status("T3"); delay(keydelay); break; }         
      break;
    case 3: 
      status("Up"); delay(keydelay); break;
    case 4: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX4), SX4, true); status("S4"); delay(keydelay); break; }
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX4), TX7, true); status("T4"); delay(keydelay); break; } 
      break;
    case 5: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX5), SX5, true); status("S5"); delay(keydelay); break; }
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX5), TX5, true); status("T5"); delay(keydelay); break; } 
      break;
    case 6: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX6), SX6, true); status("S6"); delay(keydelay); break; }
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX6), TX6, true); status("T6"); delay(keydelay); break; }       
      break;
    case 7: 
      Layout++; if (Layout>4) { Layout=1; ConfigButtons(1); } else ConfigKeys(1);
      break;
    case 8: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX7), SX7, true); status("S7"); delay(keydelay); break; }  
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX7), TX7, true); status("T7"); delay(keydelay); break; }     
      break;
    case 9: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX8), SX8, true); status("S8"); delay(keydelay); break; } 
      if (Layout==4) { MIDI1.sendSysEx(sizeof(TX8), TX8, true); status("T8"); delay(keydelay); break; }   
      break;
    case 10: 
      if (Layout==3) { MIDI1.sendSysEx(sizeof(SX9), SX9, true); status("S9"); delay(keydelay); break; } 
      if (Layout==4)  rp2040.reboot();                      
      break;
    case 11: 
      status("Dwn"); delay(keydelay); break; 
  }   // Layout=2 xpoint switch
     
}     // buttonpress

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigButtons(bool doClear)   // rowcount=0 all 5 rows rowcount=2 last row only  rowcount=1 no background redraw 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ int n, m, b;

  b = Layout - 1;                                       //  L=1/b=0 L=2/b=1                
  if (doClear) tft.fillScreen(BackgroundColor[b]);      // All four layouts Black background
  byte rowcount = 0;
  
  uint16_t OutlineColor = ButtonOutlineColor[b];
  
  for (n=0; n<30; n++) { keyColor12[n] = Colours12[n]; for (m=0; m<3; m++) keyLabel12[n][m] = Labels12[b][n][m]; }
  ae15Set(Layout-1);
                    
  for (uint8_t row = 0 + rowcount; row < 5; row++) {   // 5 rows a,b,c,d,e
    for (uint8_t col = 0; col < 6; col++) {            // 6 columns 0,1,2,3,4,5
        b = col + row * 6;
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X), KEY_Y + row * (KEY_H + KEY_SPACING_Y), 
                                               KEY_W, KEY_H, OutlineColor, keyColor12[b], White, keyLabel12[b], KEY_TEXTSIZE);
        key[b].drawButton();  }  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigKeys(bool doClear)    // Updates 12 keys in Layouts 3 4
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ int n, m, b;
  uint8_t rows = 3;    // 3 rows of labels
  uint8_t row, col;    // 3 x 4 = 12 keys

  b = Layout - 3;                                      //  L=3/b=0 L=4/b=1  
  if (doClear) tft.fillScreen(BackgroundColor[b+1]); 
  
  uint16_t OutlineColor = ButtonOutlineColor[b+1];
  
  for (n=0; n<12; n++)  { keyColor34[n] = Colours34[b][n]; for (m=0; m<3; m++) keyLabel34[n][m] = Labels34[b][n][m]; } 
              
  for (row = 0; row < rows; row++)   {     // 3 rows 1,2 
       for (col = 0; col < 4; col++) {     // 4 columns 0,1,2,3
        b = col + row * 4;
        key[b].initButton(&tft, key_X + col * (key_W + key_SPACING_X), key_Y + row * (key_H + key_SPACING_Y), 
                                               key_W, key_H, OutlineColor, keyColor34[b], White, keyLabel34[b], KEY_TEXTSIZE);
        key[b].drawButton();  }  }
}

////////////////////////////
void touch_calibrate()
///////////////////////////
{ LittleFS.begin();

  // DigitalWrite(LCDBackLight, HIGH); // Switch on here incase TFT.init did not - press sequence is TopLeft,BottomLeft,TopRight,BottomRight
    
  if (LittleFS.exists(CalFile))        
     {if ( DoCal)                
         { LittleFS.remove(CalFile);}             // Delete if we want to re-calibrate
      else {File f = LittleFS.open(CalFile, "r"); // check if calibration file exists and size is correct
            if (f) {if (f.readBytes((char *)calData, 14) == 14) calDataOK = 1;  f.close(); }  }  }

  if (calDataOK && !DoCal)         // calibration data valid
     {tft.setTouch(calData);}    
     else { tft.fillScreen(Black); // data not valid so recalibrate
            tft.setCursor(20, 0);
            tft.setTextFont(2);
            tft.setTextSize(1);
            tft.setTextColor(White, Black);
            tft.println("Touch corners as indicated");
            tft.setTextFont(1);
            tft.println();
            if (DoCal) 
               { tft.setTextColor(Red, Black);
                 tft.println("Set DoCal to false to stop this running again!"); }
            tft.calibrateTouch(calData, White, Black, 15);
            tft.setTextColor(Green, Black);
            tft.println("Calibration complete!");
            File f = LittleFS.open(CalFile, "w");  // store data
            if (f) { f.write((const unsigned char *)calData, 14); f.close();  }  }

}

/////////////////////////////////////////////////
// Status bar
/////////////////////////////////////////////////
void status(const char *msg) {
  tft.setTextPadding(320);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(LGrey, Black);
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setTextDatum(1);
  //tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
  tft.setFreeFont(&FreeSans12pt7b);         // KEY_TEXTSIZE 1 
  //tft.setTextSize(KEY_TEXTSIZE);
}  
