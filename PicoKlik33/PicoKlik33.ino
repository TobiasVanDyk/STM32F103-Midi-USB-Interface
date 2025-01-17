 ///////////////////////////////////////////////////////////////////////////////////////////
// T van Dyk Jan 2025 GPL3
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
#include "pipesDef.h"

// Create the Serial MIDI port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

int LCDBackLight = 13; // Will be set as OUTPUT and LOW by TFT init

// Used to dim LCD Backlight if 60 seconds no touch
unsigned long NowMillis = 0;           // LCD Backlight Saver
unsigned long LastMillis = 0;          // LCD Backlight Saver
unsigned long TimePeriod = 25000;      // 25 seconds LCD Backlight Timer
unsigned long SyncPeriod = 9000;       // 9 seconds start Sync Timer
bool BackLightOn = true;               // LCD Backlight Saver
int Layout = 3;                        // Layout 1, 2, 5 or 3, 4
byte LayerAD = 0;                      // Switch [Layout A B C D] for [Layers 3 4] S or C i.e. A=[S1-S9] B=[S10-M18] C=[S19-S28] D=[S29-S36]
 
//////////////////////////////////////////////////////////////////////////////////////////////
//uint16_t calData[5] = { 243, 3398, 444, 3318, 1 };     
uint16_t calData[5];                              
uint8_t  calDataOK = 0;
#define CalFile "TouchCalData"  // Name of the Touch calibration file
bool DoCal = false;             // Set true if you want the calibration to run on each boot
////////////////////////////////////////////////////////////////////////////////////////////// 

uint16_t t_x = 0, t_y = 0;           // Touch coordinates

int KeyNum = 30;                     // Maximum number of keys per Layout     
byte KeyN[5] = {30, 30, 12, 12, 24}; // 30 for Layout 1,2 and 12 for Layouts 3,4 and 4x5+4=24 for Layout 5

// 25 + 5 Keys: 5x5 XPoint Switch Keys + 5 Option keys sizes and spacing 5 JackIn  -> 5 JackOut
// 20 + 4 keys: 4x5 XPoint Switch Keys + 4 Option keys sizes and spacing 4 CableIn -> 5 JackOut
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
#define key_X 38           // 30 is off-screen on left
#define key_Y 36           // 30 is the topmost with no cut-off

#define KEY_TEXTSIZE 1   // Font size multiplier

// Status line for messages
#define STATUS_X 160 // Centred on this
#define STATUS_Y 222

// Adding a delay between keypressing to give our OS time to respond
uint8_t keydelay = 250;          // keycode[1,..,6] delay milliseconds
uint8_t keydelay2 = 25;          // keyPress delay
uint16_t keydelay3 = 500;        // used in UAC confirm

// Create the screen object
TFT_eSPI tft = TFT_eSPI();

bool Routing = false;             // Routing used to enable/disable iThru press row A-E to toggle on/off
bool Slot = false;                // Slot toggle switches active  (Slots: inputs a-e)
byte SlotNum = 0;                 // SlotNum used with [Option] to change attached Slot 0,1-8
bool SlotOption = false;          // Select Slot 1-8 with [Op] active (Slots 1-8 for inputs a-e)

byte SlotPipeId[4][8] = { 0,0,0,0,0,0,0,0,   // 1st Pipe in each Slot 1-8 Pipe ID such as 00, 01 etc Empty = 0x7F
                          0,0,0,0,0,0,0,0,   // 2nd Pipe in each Slot 1-8 Pipe ID  
                          0,0,0,0,0,0,0,0,   // 3rd Pipe in each Slot 1-8 Pipe ID  
                          0,0,0,0,0,0,0,0 }; // 4th Pipe in each Slot 1-8 Pipe ID  
byte SlotPipeBp[4][8] = { 0,0,0,0,0,0,0,0,   // 1st Pipe in each Slot 1-8 Bypass = 1 Execute = 0
                          0,0,0,0,0,0,0,0,   // 2nd Pipe in each Slot 1-8  
                          0,0,0,0,0,0,0,0,   // 3rd Pipe in each Slot 1-8  
                          0,0,0,0,0,0,0,0 }; // 4th Pipe in each Slot 1-8                  
bool Pipes = false;                          // Pipes option on
byte PipeNum = 0;                            // PipeNum used with [Option] to change attached Pipe 1-8
bool PipeOption = false;                     // Select Pipe 1-8 with [Op] Bypass
bool SlotPipes = false;
byte SlotPipesNum = 0;

bool ae15[3][5][5] = {};           // 0=IThru=Layout1 1=Midi=Layout2 2=CableIn=Layout5 JacksRouting Ports 1-5 = ae15[0-4][0-4] i.e. exclude Port 0
byte ae08[2][5]    = {};           // 0=IThru 1=Midi Slot data for IThruIn 0-5 and MidiJacksIn 0-5 Slot=0 not attached which slot 1-8 is attached
byte ae00[2][5]    = {};           // Backup for ae08[] when slots toggled attached/detached 
byte iThruMask     = 0;            // 16bit mask read from MidiKlik4x4 STM32F103 ID4 as used here never > 0xFF only Jack 1-6 used 
bool iThru15[5]    = {0,0,0,0,0};  // iThru Enabled/Disabled for JackIn 1-5          
byte iThruIdle     = 0;            // iThru idle time
                  
const int ByteSize = 4096;      // 
const int MaxBytes = ByteSize;  // 
const int SxSize = 32;          // 
byte Sync1[SXSize] = {};
byte Sync2[SXSize] = {};
byte Sync3[SXSize] = {};
byte Sync4[SXSize] = {};

static const char* digits = "0123456789ABCDEF";              // dec 2 hex digits positional map
char Int2HexStr[64] = "                                  ";  // Filled with SysX

int  NumBytes = 0;
bool NewData = false;
bool CheckSerial = true;    // Switch off serial check unless *se* toggles CheckSerial - default is off

int SysExCount = 0;
bool DoSync = true;
int Sync = 0;
bool ShowSysX = false;     // Show SysEx

// Create 30 keys which is the maximum for any layer
TFT_eSPI_Button key[30];

/////////////////
void DoSync1()
/////////////////
{ uint8_t p, n, i, JackOut1, JackOut2, SlotIThru, SlotMidi, JackIn; 
  
  for (JackIn=1; JackIn<6; JackIn++)                                        // = arr[7] to arr[11] - 1st byte ignored here arr[6]  
      {JackOut1  = Sync1[JackIn+6];                                         // JackOut 1-5 bits in each arr[7] - [11]  -> 5 bytes 
       JackOut2  = Sync1[JackIn+12];                                        // JackOut 1-5 bits in each arr[13] - [17] -> 5 bytes
       SlotIThru = Sync1[JackIn+18];                                        // SlotIThru 0,1-8 in each arr[19] - [23]  -> 5 bytes
       SlotMidi  = Sync1[JackIn+24];                                        // SlotMidi 0,1-8 in each arr[25] - [29]   -> 5 bytes                         
       if (SlotIThru<9) ae00[0][JackIn-1] = ae08[0][JackIn-1] = SlotIThru;  // IThruJackIn Slots attached
       if (SlotMidi<9)  ae00[1][JackIn-1] = ae08[1][JackIn-1] = SlotMidi;   // MidiJacksIn Slots attached                           
       for (i=1; i<6; i++)                                                  // = each bit in arr[] = bit 1-5 = Jackout 1-5
           {if (bitRead(JackOut1, i)==1) ae15[0][JackIn-1][i-1] = true;     // IThru routing
            if (bitRead(JackOut2, i)==1) ae15[1][JackIn-1][i-1] = true;     // Midi Jack Routing 
           }
      }
}
/////////////////
void DoSync2()
/////////////////
{ uint8_t i, n, CableIn;
  iThruMask      = Sync2[7];                                 // iThru Mask each bit 1/0 JackIn 1-6 iThru on/off  
  iThruIdle      = Sync2[6];                                 // iThru Idle Time  
  
  for (i=1; i<6; i++)                                        // = each bit in arr[] = bit 1-5 = Jackout 1-6
       if (bitRead(iThruMask, i)==1) iThru15[i-1] = true;    // IThru On/Off for JackIn 1-5 
        
  for (i=0; i<8; i++)                                        // First 2 Pipes in exach Slot 1-8                                     
      {SlotPipeId[0][i] = Sync2[8+i];   
       SlotPipeId[1][i] = Sync2[16+i]; } 

  for (n=0; n<4; n++)                                               // = arr[24] to arr[27]  
      {CableIn = Sync2[24+n];                                       // CableIn 1-4 bits in each arr[23] - [27]  -> 4 byte      
      for (i=1; i<6; i++)                                           // = each bit in arr[] = bit 1-5 = Jackout 1-5
           if (bitRead(CableIn, i)==1) ae15[2][n][i-1] = true;      // CableIn routing 
      }
}
/////////////////
void DoSync3()
/////////////////
{ uint8_t i;
  for (i=0; i<8; i++)                                                                            
      {SlotPipeId[2][i] = Sync3[6+i];                 // bytes 6-21  Pipes 3+4 in exach Slot 1-8 
       SlotPipeId[3][i] = Sync3[14+i]; 
       SlotPipeBp[0][i] = Sync3[22+i]; }              // bytes 22-29 Pipe 1 Bypass 0/1 in Slots 1-8
}
/////////////////
void DoSync4()
/////////////////
{ uint8_t i;
  return;
  for (i=0; i<8; i++)                                                                            
      {SlotPipeBp[1][i] = Sync4[6+i];                 // Pipe 2 Bypass 0/1 in Slots 1-8
       SlotPipeBp[2][i] = Sync4[14+i];                // Pipe 3 Bypass 0/1 in Slots 1-8
       SlotPipeBp[3][i] = Sync4[22+i]; }              // Pipe 4 Bypass 0/1 in Slots 1-8
}
////////////////////////////////////////////////
void DisplaySysX(byte *SysXArr, byte k, byte a)
////////////////////////////////////////////////
{ byte b;
  byte m = 0;
  byte n = 0;
  while (n<a && b!=0xF7) { b = SysXArr[n+k];                                   
                Int2HexStr[n+m]   = digits[(b >> 4) & 0x0F];   
                Int2HexStr[n+m+1] = digits[b & 0x0F]; n++; m=m+3; 
              }    
  Int2HexStr[n+m] = 0x00;
  status((char *)Int2HexStr);
}
    
//////////////////////////////////////////////////////////////////////////////////////////////////
void DoSysEx(byte* arr, unsigned len)
//////////////////////////////////////////////////////////////////////////////////////////////////
// Returns F0 77 77 78 0E 04 6bytes 6bytes 6bytes 6bytes F7 
// 6+6 bytes Routing data for IThru 0-5 and MidiJacks 0-5
// 6+6 bytes Slot data for IThruIn 0-5 and MidiJacksIn 0-5 Slot=0 not attached 1-8 is attached
//////////////////////////////////////////////////////////////////////////////////////////////////
{ uint8_t p, n, i, JackOut1, JackOut2, SlotIThru, SlotMidi, JackIn; 
  /*
  if (DoSync && arr[4]==0x0E && arr[5]==0x04) { for (n=0; n<len; n++) Sync1[n] = arr[n]; return; }  
  if (DoSync && arr[4]==0x0E && arr[5]==0x05) { for (n=0; n<len; n++) Sync2[n] = arr[n]; return; }
  if (DoSync && arr[4]==0x0E && arr[5]==0x06) { for (n=0; n<len; n++) Sync3[n] = arr[n]; return; }
  */
  ///*
  if (DoSync && arr[4]==0x0E)
     { switch (arr[5]) { case 4: { for (n=0; n<len; n++) Sync1[n] = arr[n]; break; }  
                         case 5: { for (n=0; n<len; n++) Sync2[n] = arr[n]; break; }
                         case 6: { for (n=0; n<len; n++) Sync3[n] = arr[n]; break; }
                         case 7: { for (n=0; n<len; n++) Sync4[n] = arr[n]; break; }
                       }         }
       
  if (ShowSysX) DisplaySysX(arr, 4, 11);
}

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

  // Layouts 1 and 2 and 5 = 2 XPoint 5x5 and 1 XPoint 5x4 switch only initialised later
  Layout = 3;                   // Layout 1/2, 3/4

  for (int p=0; p<2; p++) for (int r=0; r<5; r++) for (int c=0; c<5; c++) ae15[p][r][c] = false; // iTHru+Midi Routing all off
  for (int p=0; p<2; p++) for (int n=0; n<5; n++) ae08[p][n] = ae00[p][n] = 0;                   // iTHru+Midi JacksIn Slots all detached
  
  if (Layout<3) ConfigButtons(1);              // Draw 5x5 Xpoint Switch + 5 Option Keys
  if (Layout>4) ConfigCable(1);                // Draw 4x5 Xpoint Switch + 4 Option Keys
  if (Layout==3||Layout==4) ConfigKeys(1);     // Draw 12 Keys  
  
  LastMillis = millis();        // initial start time for LCD Dim
}

/////////////////////////////
void loop() 
/////////////////////////////
{ int i;
  NowMillis = millis();        // get the current "time" (number of milliseconds since started)
   
  if (DoSync) { if ((NowMillis - LastMillis) >= SyncPeriod)                
                   {Sync++; 
                    if (Sync==1) { MIDI1.sendSysEx(sizeof(JackAE), JackAE, true);         SyncPeriod = 10000; }
                    if (Sync==2) { DoSync1();                                             SyncPeriod = 10500; }
                    if (Sync==3) { MIDI1.sendSysEx(sizeof(PipeConf12), PipeConf12, true); SyncPeriod = 11000; }                                                                       
                    if (Sync==4) { DoSync2();                                             SyncPeriod = 11500; }  
                    if (Sync==5) { MIDI1.sendSysEx(sizeof(PipeConf34), PipeConf34, true); SyncPeriod = 12000; } 
                    if (Sync==6) { DoSync3();                                             SyncPeriod = 12500; }  
                    if (Sync==7) { MIDI1.sendSysEx(sizeof(PipeConf56), PipeConf56, true); SyncPeriod = 13000; }                                                                        
                    if (Sync==8) { DoSync4(); DoSync = false; status("Sync Completed");                       }
                   } 
               }  // if (DoSync)
              
  if ((NowMillis - LastMillis) >= TimePeriod)  // dimming period has elapsed  
     { LastMillis = NowMillis;                    // Start Timer again       
       status("");                                // Clear the status line
       analogWrite(LCDBackLight, 238); 
       BackLightOn = false;
       if (Sync==8) { DoSync = false; Sync = 0; }
     } // Until keypress    

  KeyNum = KeyN[Layout-1];
  bool pressed = tft.getTouch(&t_x, &t_y, 400);               // Last param = threshold True if valid key pressed
  for (uint8_t b = 0; b < KeyNum; b++) { if (pressed && key[b].contains(t_x, t_y)) key[b].press(true);   
                                                                             else  key[b].press(false); }                    
   
  // Check if any key changed state void press(bool p); bool isPressed() bool justPressed(); bool justReleased();
  for (uint8_t b = 0; b < KeyNum; b++) { if (key[b].justReleased())   key[b].drawButton();           // draw normal again
                                         if (key[b].justPressed())  { key[b].drawButton(true);       // invert button colours
                                                                      status("");                    // Clear the old status
                                                                      buttonpress(b);           }    // Send the button number
                                       } // for b = 0
  MIDI1.read(); // delay(1);                                                                
  
}  // main Loop

//////////////////////////////////////////////////////////////////////////
void DoiThruOnOff(byte r)
//////////////////////////////////////////////////////////////////////////
// F0 77 77 78 0E 03 04 F7
// Toggle JackIn 04 iThru on/off if JackOut exist
//////////////////////////////////////////////////////////////////////////
// In iThru 5x5 at least one switch must be on (red)
// Note that iThru enable/disable is for an entire row A-E
// Press [Rt] it turns green and display "Press row A-E for iThru On/Off"
// Press in any row with the switch that must be iThru disabled
// The red switch turns purple
// Do the same to turn the purple switch back to red or use the
// normal switch action ble/red
//////////////////////////////////////////////////////////////////////////
{ byte ToggleiThru[8]  = { 0xF0,0x77,0x77,0x78, 0x0E,0x03, 0x00, 0xF7 };
//////////////////////////////////////////////////////////////////////////
  ToggleiThru[6] = r + 1;
  MIDI1.sendSysEx(8, ToggleiThru, true);
  status("iThru En/Dis");
  ConfigButtons(0); // No background clear required  
  delay(keydelay);
}

//////////////////////////////////////////////////////////////////////////////////////////
void DoPipeBP(int r, int c, byte p, byte pipeBP)
// F0 77 77 78 11 01 05 <slot:1-8> <pipe index:nn> <no bypass:0 | bypass:1> F7
// 
// How to use:
// As we do not sync current Bypass state use Column number 0 = off 1-4 = on
// First press [Pp] then keep pressing [Op] until PipeIndex number 1-8 shows - note that
// using 1 will act on pipeindex 0, 8 will be pipeindex 7.
// Then press the slot row that contains a slot with Pipes in that Pipeindex - press 
// Column 0 to Use the Pipe and Column 2-4 to Bypass the Pipe
//
// Note Pipe Index is Pipe position in slot from 0-7
// Pipe ID is Pipe Type such as 00 01 02 03 04 etc
// To switch the first pipe on or off on slot 05 send 
// F0 77 77 78 11 01 05 05 00 00/01 F7 not F0 77 77 78 11 01 05 05 01 00/01 F7
//////////////////////////////////////////////////////////////////////////////////////////
{ byte b, n, i;
  byte SetPipeBP[11]  = { 0xF0,0x77,0x77,0x78, 0x11,0x01,0x05, 0x01,0x01,0x01, 0xF7 };
  
  b = ae08[p][r];                                     // Slot 1-8 for row r 0-4 pressed
  if (b==0) { status("Press active slot"); return; }  // b=0 no slot attached to row r
  
  if (c>0) SlotPipeBp[0][b-1] = 1;     // NotSynced Use Column 0,1 = 1 2,3,4 = 0 
      else SlotPipeBp[0][b-1] = 0;     // Toggle ByPass On/Off - can also b = !b; b ^= 1;
  SetPipeBP[7] = b;                    // Get row pressed 1-5 then slot 1-8
  SetPipeBP[8] = pipeBP;               // Pipe Index Position 0-7 
  SetPipeBP[9] = SlotPipeBp[0][b-1];   // On/Off 0/1

  MIDI1.sendSysEx(11, SetPipeBP, true);
  
  if (ShowSysX) DisplaySysX(SetPipeBP, 4, 11); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DoPipeSlot(int r, int pipeIdx, byte p, byte pipeSet) // pipeSet = Choose from a preset set of Pipes = PipeID
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Add Pipe:
// F0 77 77 78 11 01 00 <slot:1-8> <pipe id: nn> <prm: nn nn nn nn > F7
// F0 77 77 78 11 01 00 08         01                  01 01 00 00 F7
// F0 77 77 78 11 01 00 08 01 01 01 00 00 F7
// Attach Pipe ID 01 = Notechange to Slot 8 
//
// 11 01 01 insert pipe at
//                       F0   77   77   78     11   01   01     <slot:1-8> <pipe idx:nn> <pipe id:nn> <prm:nn nn nn nn > F7
// byte SetPipe[15]  = { 0xF0,0x77,0x77,0x78,  0x11,0x01,0x01,  Slot,      pipeIdx,      pipeSet[0],  pipeSet[1-3],      0xF7 };
// How to use:
// Use Column number 1-5 for Pipe Index = position in the slot 0-4 
// -> if no pipes in slot must press the first column on the selected slot
// First switch-on [Sl] then [Pp] (both green) then keep pressing [Op] until PipeSet number 1-8 shows
// Then when selected number shows press on the visible slot 1-5 to choose te PipeIndex or slot position
// PipeSet: Byte 0 = PipeID Bytes 1-3 Pipe Parameters 1-4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ byte b, n, i;
  byte SetPipe[15]  = { 0xF0,0x77,0x77,0x78,  0x11,0x01,0x01,  0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xF7 };
  
  b = ae08[p][r];                                             // Slot 1-8 for row r 0-4 pressed
  if (b==0) { status("Press visible active slot"); return; }  // b=0 no slot attached to row r
  
  SetPipe[7] = b;                                             // Slot selected 1-8 
  SetPipe[8] = pipeIdx;                                       // Column pressed 0-4
  for (n=0; n<5; n++) SetPipe[n+9] = pipeIDSet[pipeSet][n];   // PipeID + 4 parameters 
 
  MIDI1.sendSysEx(15, SetPipe, true);
  ConfigButtons(0);

  if (ShowSysX) DisplaySysX(SetPipe, 7, 15); 
}
//////////////////////////////////////////////////////////////////////////
// F0 77 77 78 11 00 02 < in port type> < in port: 0-F> <slot: 00-08> F7
// For example send F0 77 77 78 11 00 02 01 02 01 F7
// Attach Slot 1 to JackIn B 
// For example send F0 77 77 78 11 00 02 01 02 00 F7
// Detach all Slots to JackIn B 
//////////////////////////////////////////////////////////////////////////
bool DoXSlots(int r, int c, byte p, bool NewSlot) // p=0,1
//////////////////////////////////////////////////////////////////////////
{ byte SetXSlot[11]  = { 0xF0,0x77,0x77,0x78, 0x11,0x00,0x02, 0x01,0x01,0x01, 0xF7 }; 
  uint8_t a, t, b, m, n;
  byte aeX08 = ae08[p][r];  
  byte PortType = (Layout==1)*3 + (Layout==2)*1;
  
  SetXSlot[8] = r + 1;                                           // JackIn 1-5
  SetXSlot[7] = PortType;                                        // PortIn = 1 = Jack or 3 = iThruIn

  if (NewSlot) SetXSlot[9] = ae08[p][r];                         // New Slot via [Op]
      else if (aeX08) { ae08[p][r] = SetXSlot[9] = 0; }          // Detach Slot from InPort
               else { SetXSlot[9] = ae08[p][r] = ae00[p][r]; }   // Restore Backup Slot  
  
  MIDI1.sendSysEx(11, SetXSlot, true);
    
  ConfigButtons(0); // No background clear required

  if (ShowSysX) DisplaySysX(SetXSlot, 4, 11);
  
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
  m=0; for (n=1; n<6; n++) if (ae15[p][r][n-1]) { SetXPoint[p][m + 8 + p] = n; m++; if (p==0) { iThru15[r] = true; bitSet(iThruMask, n-1); }  } 
  SetXPoint[p][m + 8 + p] = 0xF7;
   
  MIDI1.sendSysEx(XPointSize, SetXPoint[p], true);
  
  ConfigButtons(0); // No background clear required

  if (ShowSysX) DisplaySysX(SetXPoint[p], 4, 15);
  
  delay(keydelay);
  return aeX15;
}

bool DoXCable(int r, int c, byte XKey) 
/////////////////////////////////////////////////////////////////////////////////////////
//    0    1   2    3     4   5     6     7     8    9
// 0xF0,0x77,0x77,0x78, 0x0F,0x01, 0x00, 0x01, 0x01, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7 
/////////////////////////////////////////////////////////////////////////////////////////
{ uint8_t m, n, b;
     
  ae15[2][r][c] = !ae15[2][r][c];
  bool aeX15 = ae15[2][r][c]; 
  
  SetXPoint[1][6] = 0x00;          // Type Cable
  SetXPoint[1][7] = r;             // CableIn 1-4 but use 0-3 in MidiSysX r =0,1,2,3
  SetXPoint[1][8] = 0x01;          // Type = JackOut 1-5 use n=1-5
  m=0; for (n=1; n<6; n++) if (ae15[2][r][n-1]) { b = SetXPoint[1][m + 9] = n; m++; } 
  if (m==1) { SetXPoint[1][m + 9] = b; m++; } SetXPoint[1][m + 9] = 0xF7;
   
  MIDI1.sendSysEx(15, SetXPoint[1], true);
  
  ConfigCable(0); // No background clear required
  
  if (ShowSysX) DisplaySysX(SetXPoint[1], 4, 15);
  
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
{ int r, c, OnColour = Red;
  for (r=0; r<5; r++) { if ( p==0 && iThru15[r] == false ) OnColour = Magenta; else OnColour = Red; // Takes a resync if iThru switched on
                        for (c=0; c<5; c++) if (ae08[p][r]==0) { if (ae15[p][r][c]) keyColor12[r*6+c] = OnColour; else keyColor12[r*6+c] = Black; } 
                                                          else { if (ae15[p][r][c]) keyColor12[r*6+c] = OnColour; else keyColor12[r*6+c] = DGreen;                                                                  
                                                                 keyLabel12[r*6+c][1] = ae08[p][r] + 48;  }
                      }
}
////////////////////////////////////////////////////////////////////////////
// Rows a-d = Cables 1-4  Column 1-5 = Jackout 1-5 (skip Jack 0 no display)
////////////////////////////////////////////////////////////////////////////
void ad15Set()
{ int r, c, OnColour = Red;
  OnColour = Red; 
  for (r=0; r<4; r++) { for (c=0; c<5; c++) if (ae15[2][r][c]) keyColor5[r*6+c] = OnColour; else keyColor5[r*6+c] = Black;   }                      
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
////////////////////////////////////////////////////////////////////////////
// F0 77 77 78 11 01 00 <slot:1-8> <pipe id: nn> <prm: nn nn nn nn > F7
// F0 77 77 78 11 01 00 08         01                  01 01 00 00 F7
// F0 77 77 78 11 01 00 08 01 01 01 00 00 F7
// Attach Pipe ID 01 = Notechange to Slot 8 
////////////////////////////////////////////////////////////////////////////
void ShowPipeIdx()
{ byte n, m, p = 4;
  //                0123456789012345678901234567890123456789012345678901
  char SPStr[]   = "xSP  . .   . .   . .   . .   . .   . .   . .   . .  ";
  
  for (n=0; n<8; n++) for (m=0; m<4; m++) { if (SlotPipeId[m][n]<0x7F) { SPStr[p]=m+49; SPStr[p+2]=n+49; SPStr[p+4]=SlotPipeId[m][n]+48; if (p<34) p=p+6; } } 
  status((char *)SPStr);    
}
void ShowPipeIdxAll(byte id)
{ byte n, p = 4;
  //                012345678901234567890123456789012345
  char SPStr[]   = "SPn  .   .   .   .   .   .   .   .  ";
  
  SPStr[2]=id+49; 
  for (n=0; n<8; n++) { if (SlotPipeId[id][n]<0x7F) { SPStr[p]=n+49; SPStr[p+2]=SlotPipeId[id][n]+48; } p=p+4; } 
  status((char *)SPStr); 
  delay(3000);  
}
void ShowPipeBpAll(byte bp)
{ byte n, p = 4;
  //                012345678901234567890123456789012345
  char SPStr[]   = "SPn  .   .   .   .   .   .   .   .  ";
  
  SPStr[2]=bp+49; 
  for (n=0; n<8; n++) { if (SlotPipeBp[bp][n]==1) { SPStr[p]=n+49; SPStr[p+2]=SlotPipeBp[bp][n]+48; } p=p+4; } 
  status((char *)SPStr); 
  delay(3000);  
}
////////////////////////////////////////////////////////////////////////////
void ShowPipesBypass(bool All)  // This is Pipe Indexes not PipeID's
{ byte n;
  if (All) for (n=0; n<4; n++) ShowPipeBpAll(n); 
}
////////////////////////////////////////////////////////////////////////////
void ShowPipes(bool All)  // This is Pipe Indexes not PipeID's
{ byte n;
  if (All) for (n=0; n<4; n++) ShowPipeIdxAll(n); else ShowPipeIdx();
}
//////////////////////////////////////
void buttonpress(int xpoint)
//////////////////////////////////////
{ byte r, c, b;
  bool SXChoice        = true;
  bool CxxSpecial      = false;
  char SlotNumStr[]    = "Slot     ";
  char PipeNumStr[]    = "Pipe     ";
  char iThruNum[]      = "iThru    ";
  char iThruIdleNum[]  = "Idle     ";
  char PipeSetStr[]    = "PipeSet  ";
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
      
  if (Layout==4 && LayerAD==0 && xpoint<7 ) CxxSpecial = true;           // First 6 C01-C06 keys have special functions
  
  if (Layout<3 || Layout==5) {                    
  switch(xpoint){                                           // Layout=1 or Layout=2
    case 5: ////////////////////////////////////////////////// Option1 
      if (Layout==1) { Routing = !Routing; if (Routing) status("Press row A-E for iThru On/Off"); 
                       if (!Routing) { iThruNum[6] = digits[(iThruMask >> 4) & 0x0F]; iThruNum[7] = digits[iThruMask & 0x0F]; status((char *)iThruNum); } 
                       ConfigButtons(0); delay(keydelay); break; }
      if (Layout==2) { Routing = false; iThruIdleNum[6] = digits[(iThruIdle >> 4) & 0x0F]; iThruIdleNum[7] = digits[iThruIdle & 0x0F]; 
                       status((char *)iThruIdleNum); ConfigButtons(0); delay(keydelay); break; }
      if (Layout==5) { status("Cable Option 1"); ConfigButtons(5); delay(keydelay); break; }
      break;    
    case 11: //////////////////////////////////////////////// Option2 Dwn
      Slot = !Slot; if (!Slot) SlotOption = false;         // Slot switches (not)active
      if (Layout==1) { if (!ShowSlots(0)) status("No iThru Slots attached"); delay(keydelay); break; }
      if (Layout==2) { if (!ShowSlots(1)) status("No Midi Slots attached");  delay(keydelay); break; }
      if (Layout==5) { status("Cable Option 2"); ConfigButtons(5); delay(keydelay); break; }
      break; 
     case 17: //////////////////////////////////////////////// Nxt
       Routing = SlotOption = Slot = Pipes = false; Layout++; 
       if (Layout>5)  { Layout=1; ConfigButtons(1); break; }
       if (Layout>2) { ConfigKeys(1); break; }
       ConfigButtons(1); break;    
    case 23: //////////////////////////////////////////////// Option3      
      if (Layout!=5) { Pipes = !Pipes; if (Pipes) ShowPipes(0); ConfigButtons(0); delay(keydelay); break; }
      if (Layout==5) { status("Cable Option 3"); ConfigButtons(5); delay(keydelay); break; }     
    case 29: //////////////////////////////////////////////// Option4 
      if (Slot && Pipes) { SlotPipes = true; status("Press [Op] select preset Pipe 1-20"); SlotPipesNum++; if (SlotPipesNum>9) SlotPipesNum = 0; 
          PipeSetStr[8] = SlotPipesNum + 48; status((char *)PipeSetStr); delay(keydelay); break; }
      if (PipeOption) { PipeNum++; if (PipeNum>8) PipeNum=1; PipeNumStr[5] = PipeNum+48; status((char *)PipeNumStr); delay(keydelay); break; } 
      if (Pipes) { status("Press [Op] PipeIdx 1-8 Bypass"); PipeNum = 1; PipeOption = true; delay(keydelay); break; }    
      if (SlotOption) { SlotNum++; if (SlotNum>8) SlotNum=1; SlotNumStr[5] = SlotNum+48; status((char *)SlotNumStr); delay(keydelay); break; } 
      if (Slot) { status("Press [Op] for Slot 1-8"); SlotNum = 0; SlotOption = true; delay(keydelay); break; }
      break;          
  }   // switch 
  
  b = X2rcb[2][xpoint]; r = X2rcb[0][xpoint]; c = X2rcb[1][xpoint];
  
  if (b<100 && Layout==5) { DoXCable(r, c, b); return; } 
  
  if (b<100 && SlotPipes && SlotPipesNum>0) { DoPipeSlot(r, c, Layout-1, SlotPipesNum-1); SlotPipes = false; SlotPipesNum = 0; return; }  // Pipe insert in Slot
  if (b<100 && PipeOption && PipeNum>0) { DoPipeBP(r, c, Layout-1, PipeNum-1); PipeOption = false; PipeNum = 0; return; }                 // Pipe Bypass set to 0 or 1
  if (b<100 && SlotOption && SlotNum>0) { ae08[Layout-1][r] = ae00[Layout-1][r] = SlotNum; DoXSlots(r, 0, Layout-1, 1); SlotOption = false; SlotNum = 0; return; }
  if (b<100 && Routing && Layout==1) { Routing = false; iThru15[r] = !iThru15[r]; DoiThruOnOff(r); return; }

  if (b<100 && !Slot) { DoXPoint(r, c, Layout-1, b); return; }
  if (b<100 && Slot)  { DoXSlots(r, c, Layout-1, 0); return; }  
  
  }   // Layout=1 or Layout=2 or Layout=5
 
  else { //if (Layout>2)
  // SXChoice = true;   
  switch(xpoint){
    case 0: 
      if (CxxSpecial) { status("Rebooting Midiklik4x and Pico in 1s"); delay(500);        // C01 Reboot MidiKlik4x then PicoRP20240 
                                MIDI1.sendSysEx(SXSize, Str1to36[1], true);    delay(5000);
                                rp2040.reboot(); }                
      break;
    case 1: 
      if (CxxSpecial) { status("Reboot Pico in 1s"); delay(1000); rp2040.reboot(); }    // C02 Reboot PicoRP20240            
      break;
    case 2: 
      if (CxxSpecial) { SXChoice = false; ShowSysX = !ShowSysX;                            // C03 Show SysX
                        if (ShowSysX) status((char *)Int2HexStr); else status("Hide SysEx Received"); return; } 
      break;
    case 3:                                                                                // Up Layer
      if (LayerAD<3) LayerAD++; else LayerAD = 0; delay(keydelay); ConfigKeys(0);  
      break; 
    case 4: 
      if (CxxSpecial) { SXChoice = false; ShowPipes(1); return;}                           // C04 Show Pipes
      break; 
    case 5: 
      if (CxxSpecial) { ShowPipesBypass(1); return; }                                      // C05 Show PipesBypass
      break;  
    case 6: 
      if (CxxSpecial) { status("Reboot Pico for upload"); delay(1000); rp2040.rebootToBootloader(); }   // C06
      break;
    case 7: 
       Routing = SlotOption = Slot = Pipes = false; Layout++; 
       if (Layout==5) { ConfigCable(1); break; }
       ConfigKeys(1); 
       break;
    case 11:                                                                               // C07=C01 and C08=C02 C09=C03 not special function
      if (LayerAD>0) LayerAD--; else LayerAD = 3; delay(keydelay); ConfigKeys(0);          // Down Layer
      break; 
    } // switch
    
    if (!SXChoice) return;
    
    b = STrcb[2][xpoint]+(LayerAD*9); r = STrcb[0][xpoint]; c = STrcb[1][xpoint];  // row column and keys 1-9, 10-18, 19-27, 28-36

    if (b<100) DoSync = true;
    if (b<100 && Layout==3) { MIDI1.sendSysEx(SXSize, Str1to36[b],   true); delay(keydelay); return; } 
    if (b<100 && Layout==4) { MIDI1.sendSysEx(CCSize, Ctr1to30[b-6], true); delay(keydelay); return; }    // 1st 6 Cxx keys special function 
     
  }   // Layout>2 
     
}     // buttonpress

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigButtons(byte doClear)   // rowcount=0 all 5 rows rowcount=2 last row only  rowcount=1 no background redraw 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ int row, col, n, m, b, p;
  char TestStr[] = "               ";
  
  p = b = Layout - 1;                                      //  L=1/b=0 L=2/b=1                
  if (doClear>0)  tft.fillScreen(BackgroundColor[b]);      // All four layouts Black background
  if (doClear==1) status((char *)L125[LayOut125[Layout]]); 
    
  uint16_t OutlineColor = ButtonOutlineColor[b];
  
  for (n=0; n<30; n++) { keyColor12[n] = Colours12[n]; for (m=0; m<3; m++) keyLabel12[n][m] = Labels12[b][n][m]; }
  if (Pipes)   keyColor12[23] = Green5; // [Pp] key active
  if (Slot)    keyColor12[11] = Green5; // [Sl] key acive
  if (Routing) keyColor12[5]  = Green5; // [Rr] key active
  ae15Set(Layout-1);

  //m = 0; for (n=0; n<5; n++) { TestStr[m] = (ae08[1][n]) + 48; m=m+2; } status((char *)TestStr);
  //delay (3000);
  //m = 0; for (n=0; n<8; n++) { if (SlotPipeId[0][n]==0x7F) TestStr[m] = 'x'; else TestStr[m] = (SlotPipeId[0][n]) + 48; m=m+2; } status((char *)TestStr);
                    
  for (row = 0; row < 5; row++) {   // 5 rows a,b,c,d,e
    XPointLabelColor = White;
    if (ae08[p][row]>0) if (SlotPipeId[0][(ae08[p][row])-1]<0x7F) XPointLabelColor = White; else XPointLabelColor = Black;
    for (col = 0; col < 6; col++) {            // 6 columns 0,1,2,3,4,5
        b = col + row * 6;
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X), KEY_Y + row * (KEY_H + KEY_SPACING_Y), 
                                               KEY_W, KEY_H, OutlineColor, keyColor12[b], XPointLabelColor, keyLabel12[b], KEY_TEXTSIZE);
        key[b].drawButton();  }  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigCable(byte doClear)   // rowcount=0 all 5 rows rowcount=2 last row only  rowcount=1 no background redraw 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ int row, col, n, m, b;         
  
  b = 0;     
  if (doClear>0)  tft.fillScreen(BackgroundColor[b]);      // Black background
  if (doClear==1) status((char *)L125[2]);
    
  uint16_t OutlineColor = ButtonOutlineColor[b];
  
  for (n=0; n<24; n++) { keyColor5[n] = Colours5[n]; for (m=0; m<3; m++) keyLabel5[n][m] = Labels5[n][m]; }
  ad15Set();
                   
  for (row = 0; row < 4; row++) {              // 4 rows a,b,c,d = Cables 1-4
    XPointLabelColor = White;
    for (col = 0; col < 6; col++) {            // 6 columns 0,1,2,3,4,5 0-5 = JackOut 1-5 do not display Jackout 0
        b = col + row * 6;
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X), KEY_Y + row * (KEY_H + KEY_SPACING_Y), 
                                               KEY_W, KEY_H, OutlineColor, keyColor5[b], XPointLabelColor, keyLabel5[b], KEY_TEXTSIZE);
        key[b].drawButton();  }  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigKeys(bool doClear)    // Updates 12 keys in Layouts 3 4
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{ int n, m, b;
  uint8_t rows = 3;    // 3 rows of labels
  uint8_t row, col;    // 3 x 4 = 12 keys

  b = Layout - 1;                                      //  L=3/b=2 L=4/b=3  
  if (doClear) tft.fillScreen(BackgroundColor[b]); 
  
  uint16_t OutlineColor = ButtonOutlineColor[b];

  b = Layout - 3;                                      //  L=3/b=0 L=4/b=1 
  
  for (n=0; n<12; n++)  { keyColor34[n] = Colours34[b][n]; for (m=0; m<3; m++) keyLabel34[n][m] = Labels34[b][n+LayerAD*12][m]; } 
              
  for (row = 0; row < rows; row++)   {     // 3 rows 1,2 
       for (col = 0; col < 4; col++) {     // 4 columns 0,1,2,3
        b = col + row * 4;
        key[b].initButton(&tft, key_X + col * (key_W + key_SPACING_X), key_Y + row * (key_H + key_SPACING_Y), 
                                               key_W, key_H, OutlineColor, keyColor34[b], LabelColor[Layout-1], keyLabel34[b], KEY_TEXTSIZE);
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
  tft.setTextColor(Cyan, Black);
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setTextDatum(1);
  //tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
  tft.setFreeFont(&FreeSans12pt7b);         // KEY_TEXTSIZE 1 
  //tft.setTextSize(KEY_TEXTSIZE);
}  
