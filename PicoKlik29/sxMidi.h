// sxMidi.h

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
const static byte JackAE[]     =  { SysExHeader 0x05, 0x0E, 0x04, 0x00, 0x00, 0xF7 };  // IThru + Midi Jacks JackIn A-E connected to ?
const static byte PipeConf12[] =  { SysExHeader 0x05, 0x0E, 0x05, 0x00, 0x00, 0xF7 };  // IThru Enabled and Idle Time + Pipes 1 + 2
const static byte PipeConf34[] =  { SysExHeader 0x05, 0x0E, 0x06, 0x00, 0x00, 0xF7 };  // Pipes 3 + 4 and Pipe 1 Bypass Slot1-8
const static byte PipeConf56[] =  { SysExHeader 0x05, 0x0E, 0x07, 0x00, 0x00, 0xF7 };  // Pipes 2, 3, 4 Bypass Slot 1-8

const int SXSize = 32;
const int CCSize = 32;

const static byte SX1[] = { SysExHeader 0x06, 0x08, 0xF7 };                    // 06 08 Reboot in configuration mode   F0 77 77 78 06 08 F7
const static byte SX2[] = { SysExHeader 0x06, 0x04, 0xF7 };                    // 06 04 Set to Factory settings        F0 77 77 78 06 04 F7
const static byte SX3[] = { SysExHeader 0x06, 0x00, 0xF7 };                    // 06 00 Reboot after Hardware reset    F0 77 77 78 06 00 F7
const static byte SX4[] = { SysExHeader 0x06, 0x09, 0xF7 };                    // 06 09 Reboot in update mode          F0 77 77 78 06 09 F7
const static byte SX5[] = { SysExHeader 0x0E, 0x00, 0xF7 };                    // 0F 00 IThru routing Reset to default F0 77 77 78 0E 00 F7
const static byte SX6[] = { SysExHeader 0x0F, 0x00, 0xF7 };                    // 0F 00 Midi routing Reset to default  F0 77 77 78 0F 00 F7
const static byte SX7[] = { SysExHeader 0x06, 0x05, 0xF7 };                    // 06 05 Clear all (midi, Ithru routing rules and pipeline)
const static byte SX8[] = { SysExHeader 0x0E, 0x01, 0xF7 };                    // 0E 01 IThru Disable all              F0 77 77 78 0E 01 F7
const static byte SX9[] = { SysExHeader 0x06, 0x02, 0xF7 };                    // 06 02 Sysex acknowledgment on/off    F0 77 77 78 06 02 F7

const static byte SX10[] = { SysExHeader 0x05, 0x0E, 0x04, 0x00, 0x00, 0xF7 };  // 05 OE 04 Dump all Jacks iThru        F0 77 77 78 05 0E 04 00 00 F7
const static byte SX11[] = { SysExHeader 0x05, 0x0E, 0x05, 0x00, 0x00, 0xF7 };  // 05 OE 05 Dump iThru On and iThruIdle F0 77 77 78 05 0E 05 00 00 F7
const static byte SX12[] = { SysExHeader 0x05, 0x7F, 0x00, 0x00, 0x00, 0xF7 };  // 05 7F Configuration sysex dump All   F0 77 77 78 05 7F 00 00 00 F7
const static byte SX13[] = { SysExHeader 0x05, 0x0E, 0x04, 0x00, 0x00, 0xF7 };  // 05 OE 04 Dump all Jacks iThru        F0 77 77 78 05 0E 04 00 00 F7
const static byte SX14[] = { SysExHeader 0x0E, 0x02, 0x01, 0xF7 };              // 0E 02 Set USB idle 15   seconds      F0 77 77 78 0E 02 01 F7
const static byte SX15[] = { SysExHeader 0x0E, 0x02, 0x0A, 0xF7 };              // 0E 02 Set USB idle 150  seconds      F0 77 77 78 0E 02 0A F7
const static byte SX16[] = { SysExHeader 0x0E, 0x02, 0x64, 0xF7 };              // 0E 02 Set USB idle 1500 seconds      F0 77 77 78 0E 02 64 F7
const static byte SX17[] = { SysExHeader 0x0E, 0x06, 0x06, 0xF7 };              // 06 06 Save settings to flash memory  F0 77 77 78 06 06 F7
const static byte SX18[] = { SysExHeader 0x0C, 0x02, 0x00, 0x00, 0xF7 };        // 0C 02 Enable/disable Midi Time Clock F0 77 77 78 0C 02 MTC 0/1 F7

const static byte SX19[] = { SysExHeader 0x0E, 0x03, 0x01, 0xF7 };              // 0E 03 Set JackIn 1 IThru on/off      F0 77 77 78 0E 03 01 F7
const static byte SX20[] = { SysExHeader 0x0E, 0x03, 0x02, 0xF7 };              // 0E 03 Set JackIn 2 IThru on/off      F0 77 77 78 0E 03 02 F7
const static byte SX21[] = { SysExHeader 0x0E, 0x03, 0x03, 0xF7 };              // 0E 03 Set JackIn 3 IThru on/off      F0 77 77 78 0E 03 03 F7
const static byte SX22[] = { SysExHeader 0x0E, 0x03, 0x04, 0xF7 };              // 0E 03 Set JackIn 4 IThru on/off      F0 77 77 78 0E 03 04 F7
const static byte SX23[] = { SysExHeader 0x0E, 0x03, 0x05, 0xF7 };              // 0E 03 Set JackIn 5 IThru on/off      F0 77 77 78 0E 03 05 F7
const static byte SX24[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX25[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX26[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX27[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 

const static byte SX28[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX29[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX30[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX31[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX32[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX33[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX34[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX35[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 
const static byte SX36[] = { SysExHeader 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7 }; 

const byte * Str1to36[36] = { SX1, SX2, SX3, SX4, SX5, SX6, SX7, SX8, SX9,  SX10,SX11,SX12,SX13,SX14,SX15,SX16,SX17,SX18,
                              SX19,SX20,SX21,SX22,SX23,SX24,SX25,SX26,SX27, SX28,SX29,SX30,SX31,SX32,SX33,SX34,SX35,SX36 };

byte CC1[CCSize]  = { SysExHeader 0x0E, 0x03, 0x01, 0xF7 };              // 0E 03 Set JackIn 1 IThru on/off      F0 77 77 78 0E 03 01 F7
byte CC2[CCSize]  = { SysExHeader 0x0E, 0x03, 0x02, 0xF7 };              // 0E 03 Set JackIn 2 IThru on/off      F0 77 77 78 0E 03 02 F7
byte CC3[CCSize]  = { SysExHeader 0x0E, 0x03, 0x03, 0xF7 };              // 0E 03 Set JackIn 3 IThru on/off      F0 77 77 78 0E 03 03 F7
byte CC4[CCSize]  = { SysExHeader 0x0E, 0x03, 0x04, 0xF7 };              // 0E 03 Set JackIn 4 IThru on/off      F0 77 77 78 0E 03 04 F7
byte CC5[CCSize]  = { SysExHeader 0x0E, 0x03, 0x05, 0xF7 };              // 0E 03 Set JackIn 5 IThru on/off      F0 77 77 78 0E 03 05 F7
byte CC6[CCSize]  = { SysExHeader 0x0E, 0x02, 0x01, 0xF7 };              // 0E 02 Set USB idle 15   seconds      F0 77 77 78 0E 02 01 F7
byte CC7[CCSize]  = { SysExHeader 0x0E, 0x02, 0x0A, 0xF7 };              // 0E 02 Set USB idle 150  seconds      F0 77 77 78 0E 02 0A F7
byte CC8[CCSize]  = { SysExHeader 0x0E, 0x02, 0x64, 0xF7 };              // 0E 02 Set USB idle 1500 seconds      F0 77 77 78 0E 02 64 F7
byte CC9[CCSize]  = { SysExHeader 0x06, 0x02, 0xF7 };                    // 06 02 Sysex acknowledgment on/off    F0 77 77 78 06 02 F7

                   // CC10 = Reboot RP2040 Pico
                   // CC11 = Show the first four pipes in Slots 1-8
                   // CC12 = Toggle ShowSysX
                   // CC13 = Resync Pico and MidiKlik4x4  
                   // CC14 = Show the Bypass State of the first four pipes in Slots 1-

byte CC10[CCSize],CC11[CCSize],CC12[CCSize],CC13[CCSize],CC14[CCSize],CC15[CCSize],CC16[CCSize],CC17[CCSize],CC18[CCSize];
byte CC19[CCSize],CC20[CCSize],CC21[CCSize],CC22[CCSize],CC23[CCSize],CC24[CCSize],CC25[CCSize],CC26[CCSize],CC27[CCSize];
byte CC28[CCSize],CC29[CCSize],CC30[CCSize],CC31[CCSize],CC32[CCSize],CC33[CCSize],CC34[CCSize],CC35[CCSize],CC36[CCSize];

byte * Ctr1to36[36] = { CC1,CC2,CC3,CC4,CC5,CC6,CC7,CC8,CC9,          CC10,CC11,CC12,CC13,CC14,CC15,CC16,CC17,CC18,
                        CC19,CC20,CC21,CC22,CC23,CC24,CC25,CC26,CC27, CC28,CC29,CC30,CC31,CC32,CC33,CC34,CC35,CC36 }; 

                               
