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
#include "coloursDef.h"   // Colours used
#include "keysDef.h"      // Keys used
#include "sxMidi.h"       // Midi System Exclusive and Midi ControlChange

// Create the Serial MIDI port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

int LCDBackLight = 13; // Will be set as OUTPUT and LOW by TFT init

// Used to dim LCD Backlight if 60 seconds no touch
unsigned long NowMillis = 0;           // LCD Backlight Saver
unsigned long LastMillis = 0;          // LCD Backlight Saver
unsigned long TimePeriod = 15000;      // 60 seconds LCD Backlight Timer
bool BackLightOn = true;               // LCD Backlight Saver
int Layout = 3;                        // Layout 1, 2 or 3, 4
byte LayerAD = 0;                      // Switch [Layout A B C D] for [Layers 3 4] S or T i.e. A=[S1-S9] B=[S10-M18] C=[S19-S28] D=[S29-S36]
 
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

bool Routing = true;       // Routing later used with [Option] to change Routing (Routing: inputs a-e then outputs 1-5)
bool Slot = false;         // Slot toggle switches active  (Slots: inputs a-e)
byte SlotNum = 0;          // SlotNum used wit [Option] to change attached Slot 0,1-8
bool SlotOption = false;   // Select Slot 1-8 with [Op] active (Slots 1-8 for inputs a-e)
 
bool ae15[2][5][5] = {};   // 0=IThru 1=Midi JacksRouting Ports 1-5 = ae15[0-4][0-4] i.e. exclude Port 0
byte ae08[2][5]    = {};   // 0=IThru 1=Midi Slot data for IThruIn 0-5 and MidiJacksIn 0-5 Slot=0 not attached which slot 1-8 is attached
byte ae00[2][5]    = {};   // Backup for ae08[] when slots toggled attached/detached                    
                     
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
// Returns F0 77 77 78 0E 04 00 06 00 38 00 00 00 00 02 00 00 00 6bytes 6bytes F7 with 
// 6+6 bytes Routing data for IThru 0-5 and MidiJacks 0-5
// 6+6 bytes Slot data for IThruIn 0-5 and MidiJacksIn 0-5 Slot=0 not attached 1-8 is attached
{ uint8_t p, n, i, JackOut1, JackOut2, SlotIThru, SlotMidi, JackIn; 
  
  if (GetIThru) { if (arr[0]==0xF0 && arr[4]==0x0E && arr[5]==0x04) 
                     {for (JackIn=1; JackIn<6; JackIn++)                                        // = arr[7] to arr[11]
                          {JackOut1  = arr[JackIn+6];                                           // JackOut bits in each arr[7] - [11]
                           JackOut2  = arr[JackIn+12];                                          // JackOut bits in each arr[13] - [17]
                           SlotIThru = arr[JackIn+18];                                          // SlotIThru 0,1-8 in each arr[18] - [23]
                           SlotMidi  = arr[JackIn+24];                                          // SlotMidi 0,1-8 in each arr[24] - [29]
                           if (SlotIThru<9) ae00[0][JackIn-1] = ae08[0][JackIn-1] = SlotIThru;  // IThruJackIn Slots attached
                           if (SlotMidi<9)  ae00[1][JackIn-1] = ae08[1][JackIn-1] = SlotMidi;   // MidiJacksIn Slots attached                           
                           for (i=1; i<6; i++)                                                  // = each bit in arr[] = bit 1-5 = Jackout 1-6
                               {if (bitRead(JackOut1, i)==1) ae15[0][JackIn-1][i-1] = true;     // IThru routing
                                if (bitRead(JackOut2, i)==1) ae15[1][JackIn-1][i-1] = true;     // Midi Jack Routing 
                               }
                          }                                       
                     }
                }                                                                               // if (GetIThru)           
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

  for (int p=0; p<2; p++) for (int r=0; r<5; r++) for (int c=0; c<5; c++) ae15[p][r][c] = false; // iTHru+Midi Routing all off
  for (int p=0; p<2; p++) for (int n=0; n<5; n++) ae08[p][n] = ae00[p][n] = 0;                   // iTHru+Midi JacksIn Slots all unattached
  
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
    if (GetIThruCount<2) { DoIThruConf(GetIThruCount); }  // Sync 5x5 JackIn->JackOut (and UART0 hardwired for SysEx control) and sync attached slots
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
// F0 77 77 78 11 00 02 < in port type> < in port: 0-F> <slot: 00-08> F7
// For example send F0 77 77 78 11 00 02 01 02 01 F7
// Attach Slot 1 to JackIn B 
// For example send F0 77 77 78 11 00 02 01 02 00 F7
// Detach all Slots to JackIn B 
///////////////////////////////////////////////////////
bool DoXSlots(int r, int c, byte p, bool NewSlot) // p=0,1
///////////////////////////////////////////////////////
// byte SetXSlot[11]  = { 0xF0,0x77,0x77,0x78, 0x11,0x00,0x02, 0x01,0x01,0x01, 0xF7 }; 
/////////////////////////////////////////////////////////////////////////////////////////
{ uint8_t a, t, b, m, n;
  byte aeX08 = ae08[p][r];  
  byte PortType = (Layout==1)*3 + (Layout==2)*1;
  
  SetXSlot[8] = r + 1;                                           // JackIn 1-5
  SetXSlot[7] = PortType;                                        // PortIn = 1 = Jack or 3 = iThruIn

  if (NewSlot) SetXSlot[9] = ae08[p][r];                         // New Slot via [Op]
      else if (aeX08) { ae08[p][r] = SetXSlot[9] = 0; }          // Detach Slot from InPort
               else { SetXSlot[9] = ae08[p][r] = ae00[p][r]; }   // Restore Backup Slot  
  
  MIDI1.sendSysEx(11, SetXSlot, true);
  
  /*
  n = 4; m = 0; a = 12;             // n=4; starts at 11               
  while (n<a) { b = SetXSlot[n];                                  
                Int2HexStr[n+m]   = digits[(b >> 4) & 0x0F];   
                Int2HexStr[n+m+1] = digits[b & 0x0F]; n++; m=m+3; }    
  status((char *)Int2HexStr); 
  */
  
  ConfigButtons(0); // No background clear required
  
  delay(keydelay);
  return (aeX08>0);
}

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
  //                     01234567890
  char XPointStatus[] = "00   Slot  ";  
  int XPointSize = 14 + p;  
   
  ae15[p][r][c] = !ae15[p][r][c];
  bool aeX15 = ae15[p][r][c];
  byte aeX08 = ae08[p][r];  
  
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
  
  // XPointStatus[0] = Labels12[p][XKey][0];  // Input = a-e, A-E
  // XPointStatus[1] = Labels12[p][XKey][1];  // Output = 1-5 
  // if (aeX15) XPointStatus[3] = 'X';        // If on
  // if (aeX08) XPointStatus[10] = aeX08+49;  // Slot if 1-8 
  // status((char *)XPointStatus); 

  ConfigButtons(0); // No background clear required
  
  delay(keydelay);
  return aeX15;
}

////////////////////////////////////////////////////////////////////////////
// Set Layout 1 5x5 A-E Input x 1-5 Output Switch Matrix
////////////////////////////////////////////////////////////////////////////
// F0 77 77 78 11 00 02 < in port type> < in port: 0-F> <slot: 00-08> F7
// For example send F0 77 77 78 11 00 02 01 02 01 F7
// Attach Slot 1 to JackIn B 
////////////////////////////////////////////////////////////////////////////
void ae15Set(byte p)
{ int r, c;
  for (r=0; r<5; r++) for (c=0; c<5; c++) if (ae08[p][r]==0) { if (ae15[p][r][c]) keyColor12[r*6+c] = Red; else keyColor12[r*6+c] = Blue; }
                                                        else { if (ae15[p][r][c]) keyColor12[r*6+c] = Red; else keyColor12[r*6+c] = DGreen; 
                                                               keyLabel12[r*6+c][1] = ae08[p][r] + 48;  }
}

////////////////////////////////////////////////////////////////////////////
bool ShowSlots(byte c)
////////////////////////////////////////////////////////////////////////////
// F0 77 77 78 11 00 02 < in port type> < in port: 0-F> <slot: 00-08> F7
// For example send F0 77 77 78 11 00 02 01 02 01 F7
// Attach Slot 1 to JackIn B 
////////////////////////////////////////////////////////////////////////////
{ byte p, n;
  //                0123456789012345678901
  //   SlotStr[] = "In/Slot a. b. c. d. e.";
  char SlotStr[] = "In/Slot               ";
  p = 8; for (n=0; n<5; n++) if (ae08[c][n]>0) { SlotStr[p]=n+0x41; SlotStr[p+1]=ae08[c][n]+48; p=p+3; status((char *)SlotStr); }
  ConfigButtons(0);
  if (p==8) return false; else return true;
}
//////////////////////////////////////
void buttonpress(int xpoint)
//////////////////////////////////////
{ byte r, c, b;
  char SlotNumStr[] = "Slot  ";
  const static byte X2rcb[3][30] = { 0,0,0,0,0, 100, 1,1,1,1,1, 101, 2, 2, 2, 2, 2,  102, 3, 3, 3, 3, 3,  103, 4, 4, 4, 4, 4,  104,
                                     0,1,2,3,4, 100, 0,1,2,3,4, 101, 0, 1, 2, 3, 4,  102, 0, 1, 2, 3, 4,  103, 0, 1, 2, 3, 4,  104,
                                     0,1,2,3,4, 100, 5,6,7,8,9, 101, 10,11,12,13,14, 102, 15,16,17,18,19, 103, 20,21,22,23,24, 104 };  
  const static byte STrcb[3][12] = { 0,0,0, 100, 1,1,1, 101, 2,2,2, 102,
                                     0,1,2, 100, 0,1,2, 101, 0,1,2, 102,  
                                     0,1,2, 100, 3,4,5, 101, 6,7,8, 102 };  


  ///////////////////////////////////////// Skip 1st any-key keypress which activates backlight
  LastMillis = millis();                 // Reset Backlight Timer if any key pressed                                        
  if (!BackLightOn)                      // Keypress only active if backlight on
     {//digitalWrite(LCDBackLight, LOW); // Backlight On
      analogWrite(LCDBackLight, 128);    // Backlight On 255=Off 0=Full-On
      BackLightOn = true;
      return; } 
  
  if (Layout<3) {                    
  switch(xpoint){                                           // Layout=1 or Layout=2
    case 5: ////////////////////////////////////////////////// Option1 
      Routing = true;                                       // Routing switches active
      if (Layout==1) { status("Routing JackIn iThru"); delay(keydelay); break; }
      if (Layout==2) { status("Routing JackIn Midi");  delay(keydelay); break; }
      break;    
    case 11: //////////////////////////////////////////////// Option2 Dwn
      Slot = !Slot;                                        // Slot switches (not)active
      if (Layout==1) { if (!ShowSlots(0)) status("No iThru Slots attached"); break; }
      if (Layout==2) { if (!ShowSlots(1)) status("No Midi Slots attached");  break; }
      break; 
     case 17: //////////////////////////////////////////////// Nxt
      Layout++; if (Layout>2) ConfigKeys(1); else ConfigButtons(1);
      break;     
    case 23: //////////////////////////////////////////////// Option3
      if (Layout==1) { status("Slot Pipes"); delay(keydelay); break; }    
      if (Layout==2) { status("Slot Pipes");  delay(keydelay); break; }        
      break;      
    case 29: //////////////////////////////////////////////// Option4      
      if (SlotOption) { SlotNum++; if (SlotNum>8) SlotNum=1; SlotNumStr[5] = SlotNum+48; status((char *)SlotNumStr); break; } 
      if (Slot) { status("Slot 1-8 Press [Op]"); SlotNum = 0; SlotOption = true; break; }
      if (Layout==1) { status("Option: "); delay(keydelay); break; }  
      if (Layout==2) { status("Option: "); delay(keydelay); break; }    
      break;          
  }   // switch 
  
  b = X2rcb[2][xpoint]; r = X2rcb[0][xpoint]; c = X2rcb[1][xpoint]; 
  if (b<100 && SlotOption && SlotNum>0) { ae08[Layout-1][r] = ae00[Layout-1][r] = SlotNum; DoXSlots(r, 0, Layout-1, 1); SlotOption = false; SlotNum = 0; return; }
  if (b<100 && !Slot) { DoXPoint(r, c, Layout-1, b); return; }  
  if (b<100 && Slot)  { DoXSlots(r, c, Layout-1, 0); return; }  
  
  }   // Layout=1 or Layout=2
 
  else { //if (Layout>2)    
  switch(xpoint){
    case 3: 
      if (LayerAD<3) LayerAD++; else LayerAD = 0; delay(keydelay); ConfigKeys(0); 
      break;
    case 7: 
      Layout++; if (Layout>4) { Layout=1; ConfigButtons(1); } else ConfigKeys(1);
      break;
    case 10: 
      if (Layout==4)  rp2040.reboot();                      
      break;
    case 11: 
      if (LayerAD>0) LayerAD--; else LayerAD = 3; delay(keydelay); ConfigKeys(0); 
      break; 
    } // switch
    
    b = STrcb[2][xpoint]+(LayerAD*9); r = STrcb[0][xpoint]; c = STrcb[1][xpoint];  // row column and keys 1-9, 10-18, 19-27, 28-36
    
    if (b<100 && Layout==3) { MIDI1.sendSysEx(SXSize, Str1to36[b], true); delay(keydelay); return; } 
    if (b<100 && Layout==4) { MIDI1.sendSysEx(CCSize, Ttr1to36[b], true); delay(keydelay); return; }   
     
  }   // Layout>2 
     
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
  if (Slot) keyColor12[11] = Green5; 
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
  
  for (n=0; n<12; n++)  { keyColor34[n] = Colours34[b][n]; for (m=0; m<3; m++) keyLabel34[n][m] = Labels34[b][n+LayerAD*12][m]; } 
              
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
